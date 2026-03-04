#include "../../include/dashboard_service.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace forge {

static std::string today_date_str() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&t, &tm_buf);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
    return std::string(buf);
}

static int iso_weekday(const std::string& date_str) {
    std::tm tm{};
    std::istringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    std::mktime(&tm);
    return (tm.tm_wday + 6) % 7; // 0=Mon..6=Sun
}

static std::string date_offset(const std::string& base, int days) {
    std::tm tm{};
    std::istringstream ss(base);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    tp += std::chrono::hours(24 * days);
    auto t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_buf{};
    localtime_r(&t, &tm_buf);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
    return std::string(buf);
}

DashboardResponse DashboardService::get_dashboard(const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    std::string today = today_date_str();

    DashboardResponse resp;
    resp.date = today;
    resp.nutrition = get_nutrition_today(txn, user_id, today);
    resp.workout = get_workout_today(txn, user_id, today);
    resp.week = get_week_overview(txn, user_id, today);

    txn.commit();
    return resp;
}

NutritionToday DashboardService::get_nutrition_today(
    pqxx::work& txn, const std::string& user_id, const std::string& date
) {
    NutritionToday nt;

    auto r = txn.exec_params(
        "SELECT COALESCE(SUM(calories * quantity), 0) AS cal, "
        "       COALESCE(SUM(protein_g * quantity), 0) AS pro, "
        "       COALESCE(SUM(carbs_g * quantity), 0) AS carb, "
        "       COALESCE(SUM(fat_g * quantity), 0) AS fat "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date",
        user_id, date
    );

    if (!r.empty()) {
        nt.calories_consumed = r[0]["cal"].as<double>(0);
        nt.protein_consumed = r[0]["pro"].as<double>(0);
        nt.carbs_consumed = r[0]["carb"].as<double>(0);
        nt.fat_consumed = r[0]["fat"].as<double>(0);
    }

    auto pr = txn.exec_params(
        "SELECT target_calories, target_protein_g, target_carbs_g, target_fat_g "
        "FROM user_profiles WHERE user_id = $1",
        user_id
    );

    if (!pr.empty()) {
        if (!pr[0]["target_calories"].is_null())
            nt.calories_target = pr[0]["target_calories"].as<double>();
        if (!pr[0]["target_protein_g"].is_null())
            nt.protein_target = pr[0]["target_protein_g"].as<double>();
        if (!pr[0]["target_carbs_g"].is_null())
            nt.carbs_target = pr[0]["target_carbs_g"].as<double>();
        if (!pr[0]["target_fat_g"].is_null())
            nt.fat_target = pr[0]["target_fat_g"].as<double>();
    }

    return nt;
}

std::optional<WorkoutToday> DashboardService::get_workout_today(
    pqxx::work& txn, const std::string& user_id, const std::string& date
) {
    auto r = txn.exec_params(
        "SELECT w.id, w.name, w.status, "
        "       EXTRACT(EPOCH FROM (w.completed_at - w.started_at))::int AS duration_seconds, "
        "       COUNT(DISTINCT es.exercise_id) AS exercises_count, "
        "       COUNT(es.id) AS sets_count, "
        "       COALESCE(SUM(CASE WHEN es.weight_kg IS NOT NULL AND es.reps IS NOT NULL "
        "           THEN es.weight_kg * es.reps ELSE 0 END), 0) AS total_volume_kg, "
        "       COALESCE(SUM(CASE WHEN es.is_pr THEN 1 ELSE 0 END), 0) AS prs_count "
        "FROM workouts w "
        "LEFT JOIN exercise_sets es ON es.workout_id = w.id "
        "WHERE w.user_id = $1 AND DATE(w.started_at) = $2::date "
        "  AND w.deleted_at IS NULL "
        "GROUP BY w.id "
        "ORDER BY w.started_at DESC LIMIT 1",
        user_id, date
    );

    if (r.empty()) return std::nullopt;

    WorkoutToday wt;
    wt.completed = (r[0]["status"].as<std::string>() == "completed");
    wt.name = r[0]["name"].is_null() ? "Workout" : r[0]["name"].as<std::string>();
    wt.duration_seconds = r[0]["duration_seconds"].is_null() ? 0 : r[0]["duration_seconds"].as<int>();
    wt.total_volume_kg = r[0]["total_volume_kg"].as<double>(0);
    wt.exercises_count = r[0]["exercises_count"].as<int>(0);
    wt.sets_count = r[0]["sets_count"].as<int>(0);
    wt.prs_count = r[0]["prs_count"].as<int>(0);

    return wt;
}

WeekOverview DashboardService::get_week_overview(
    pqxx::work& txn, const std::string& user_id, const std::string& today
) {
    WeekOverview wo;

    int weekday = iso_weekday(today);
    std::string monday = date_offset(today, -weekday);
    std::string sunday = date_offset(monday, 6);

    wo.workout_days.resize(7, false);
    wo.daily_calories.resize(7, nullptr);
    wo.daily_protein.resize(7, nullptr);

    auto wr = txn.exec_params(
        "SELECT DISTINCT DATE(started_at) AS d "
        "FROM workouts "
        "WHERE user_id = $1 AND DATE(started_at) BETWEEN $2::date AND $3::date "
        "  AND status = 'completed' AND deleted_at IS NULL",
        user_id, monday, sunday
    );

    for (const auto& row : wr) {
        std::string d = row["d"].as<std::string>().substr(0, 10);
        for (int i = 0; i < 7; ++i) {
            if (date_offset(monday, i) == d) {
                wo.workout_days[i] = true;
                break;
            }
        }
    }

    auto nr = txn.exec_params(
        "SELECT DATE(logged_at) AS d, "
        "       SUM(calories * quantity) AS cal, "
        "       SUM(protein_g * quantity) AS pro "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) BETWEEN $2::date AND $3::date "
        "GROUP BY DATE(logged_at)",
        user_id, monday, sunday
    );

    for (const auto& row : nr) {
        std::string d = row["d"].as<std::string>().substr(0, 10);
        for (int i = 0; i < 7; ++i) {
            if (date_offset(monday, i) == d) {
                wo.daily_calories[i] = row["cal"].as<double>(0);
                wo.daily_protein[i] = row["pro"].as<double>(0);
                break;
            }
        }
    }

    auto pr = txn.exec_params(
        "SELECT target_calories, target_protein_g FROM user_profiles WHERE user_id = $1",
        user_id
    );
    if (!pr.empty()) {
        if (!pr[0]["target_calories"].is_null())
            wo.calorie_target = pr[0]["target_calories"].as<double>();
        if (!pr[0]["target_protein_g"].is_null())
            wo.protein_target = pr[0]["target_protein_g"].as<double>();
    }

    wo.current_streak = calculate_streak(txn, user_id, today);

    return wo;
}

int DashboardService::calculate_streak(
    pqxx::work& txn, const std::string& user_id, const std::string& today
) {
    auto r = txn.exec_params(
        "SELECT DISTINCT DATE(started_at) AS d "
        "FROM workouts "
        "WHERE user_id = $1 AND status = 'completed' AND deleted_at IS NULL "
        "  AND DATE(started_at) <= $2::date "
        "ORDER BY d DESC "
        "LIMIT 60",
        user_id, today
    );

    if (r.empty()) return 0;

    int streak = 0;
    std::string expected = today;

    for (const auto& row : r) {
        std::string d = row["d"].as<std::string>().substr(0, 10);
        if (d == expected) {
            streak++;
            expected = date_offset(expected, -1);
        } else if (streak == 0 && d == date_offset(today, -1)) {
            streak = 1;
            expected = date_offset(d, -1);
        } else {
            break;
        }
    }

    return streak;
}

} // namespace forge
