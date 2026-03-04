#include "../../include/analytics_service.hpp"
#include "../../include/profile_service.hpp"
#include <cmath>
#include <set>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace forge {

// Helper: get today's date as YYYY-MM-DD string
static std::string get_today_str() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_now = std::gmtime(&time_t_now);
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%Y-%m-%d");
    return oss.str();
}

// Helper: subtract N days from a date string and return YYYY-MM-DD
static std::string subtract_days(const std::string& date_str, int days) {
    std::tm tm = {};
    std::istringstream iss(date_str);
    iss >> std::get_time(&tm, "%Y-%m-%d");
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    tp -= std::chrono::hours(24 * days);
    auto time_t_result = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm_result = std::gmtime(&time_t_result);
    std::ostringstream oss;
    oss << std::put_time(tm_result, "%Y-%m-%d");
    return oss.str();
}

// Helper: generate all dates from start to end (inclusive) as YYYY-MM-DD
static std::vector<std::string> generate_date_range(const std::string& start, const std::string& end) {
    std::vector<std::string> dates;
    std::tm tm_start = {};
    std::istringstream iss_s(start);
    iss_s >> std::get_time(&tm_start, "%Y-%m-%d");
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_start));

    std::tm tm_end = {};
    std::istringstream iss_e(end);
    iss_e >> std::get_time(&tm_end, "%Y-%m-%d");
    auto tp_end = std::chrono::system_clock::from_time_t(std::mktime(&tm_end));

    while (tp <= tp_end) {
        auto t = std::chrono::system_clock::to_time_t(tp);
        std::tm* tm_cur = std::gmtime(&t);
        std::ostringstream oss;
        oss << std::put_time(tm_cur, "%Y-%m-%d");
        dates.push_back(oss.str());
        tp += std::chrono::hours(24);
    }

    return dates;
}

std::vector<MuscleGroupVolume> AnalyticsService::get_muscle_distribution(
    const std::string& user_id,
    int days)
{
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT e.muscle_group, COUNT(es.id) AS total_sets "
        "FROM exercise_sets es "
        "JOIN workouts w ON es.workout_id = w.id "
        "JOIN exercises e ON es.exercise_id = e.id "
        "WHERE w.user_id = $1 "
        "AND w.status = 'completed' "
        "AND w.started_at >= CURRENT_TIMESTAMP - $2 * INTERVAL '1 day' "
        "GROUP BY e.muscle_group "
        "ORDER BY total_sets DESC",
        user_id, days
    );

    txn.commit();

    std::vector<MuscleGroupVolume> volumes;
    int grand_total = 0;

    for (const auto& row : result) {
        MuscleGroupVolume v;
        v.muscle_group = row["muscle_group"].c_str();
        v.total_sets = row["total_sets"].as<int>();
        grand_total += v.total_sets;
        volumes.push_back(std::move(v));
    }

    // Calculate percentages
    for (auto& v : volumes) {
        if (grand_total > 0) {
            v.percentage = std::round(static_cast<double>(v.total_sets) / grand_total * 1000.0) / 10.0;
        } else {
            v.percentage = 0;
        }
    }

    return volumes;
}

std::vector<PersonalRecord> AnalyticsService::get_pr_history(
    const std::string& user_id,
    int limit)
{
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT es.id, e.name AS exercise_name, "
        "es.weight_kg, es.reps, "
        "TO_CHAR(w.started_at, 'YYYY-MM-DD') AS date "
        "FROM exercise_sets es "
        "JOIN workouts w ON es.workout_id = w.id "
        "JOIN exercises e ON es.exercise_id = e.id "
        "WHERE w.user_id = $1 "
        "AND es.is_pr = true "
        "AND w.status = 'completed' "
        "ORDER BY w.started_at DESC "
        "LIMIT $2",
        user_id, limit
    );

    txn.commit();

    std::vector<PersonalRecord> prs;
    prs.reserve(result.size());

    for (const auto& row : result) {
        PersonalRecord pr;
        pr.id = row["id"].c_str();
        pr.exercise_name = row["exercise_name"].c_str();
        pr.weight_kg = row["weight_kg"].is_null() ? 0.0 : row["weight_kg"].as<double>();
        pr.reps = row["reps"].is_null() ? 0 : row["reps"].as<int>();
        pr.pr_type = "estimated_1rm";
        pr.date = row["date"].c_str();

        // Epley formula: 1RM = weight * (1 + reps/30)
        if (pr.weight_kg > 0 && pr.reps > 0) {
            pr.value = std::round(pr.weight_kg * (1.0 + static_cast<double>(pr.reps) / 30.0) * 10.0) / 10.0;
        } else {
            pr.value = pr.weight_kg;
        }

        prs.push_back(std::move(pr));
    }

    return prs;
}

std::vector<ConsistencyPoint> AnalyticsService::get_training_consistency(
    const std::string& user_id,
    int days)
{
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Get workout dates and volumes for the range
    auto result = txn.exec_params(
        "SELECT DATE(w.started_at) AS workout_date, "
        "COALESCE(SUM(es.weight_kg * es.reps), 0) AS volume_kg "
        "FROM workouts w "
        "LEFT JOIN exercise_sets es ON es.workout_id = w.id "
        "WHERE w.user_id = $1 "
        "AND w.status = 'completed' "
        "AND w.started_at >= CURRENT_TIMESTAMP - $2 * INTERVAL '1 day' "
        "GROUP BY DATE(w.started_at) "
        "ORDER BY workout_date",
        user_id, days
    );

    txn.commit();

    // Build a set of workout dates for fast lookup
    std::set<std::string> workout_dates;
    std::map<std::string, double> date_volume;
    for (const auto& row : result) {
        std::string date = row["workout_date"].c_str();
        workout_dates.insert(date);
        date_volume[date] = row["volume_kg"].as<double>();
    }

    // Generate all dates in range
    std::string today = get_today_str();
    std::string start = subtract_days(today, days);
    auto all_dates = generate_date_range(start, today);

    std::vector<ConsistencyPoint> points;
    points.reserve(all_dates.size());

    for (const auto& date : all_dates) {
        ConsistencyPoint pt;
        pt.date = date;
        pt.workout_completed = workout_dates.count(date) > 0;
        pt.volume_kg = date_volume.count(date) > 0 ? date_volume[date] : 0.0;
        points.push_back(std::move(pt));
    }

    return points;
}

StreakData AnalyticsService::calculate_streak(const std::vector<std::string>& dates) {
    StreakData result{0, 0};
    if (dates.empty()) return result;

    // dates should be sorted ASC (earliest first)
    std::string today = get_today_str();

    // Build a set for O(1) lookup
    std::set<std::string> date_set(dates.begin(), dates.end());

    // Calculate current streak: walk backwards from today
    int current = 0;
    std::string check_date = today;
    while (date_set.count(check_date)) {
        current++;
        check_date = subtract_days(today, current);
    }
    result.current = current;

    // Calculate best streak: scan all dates in the 90-day range
    auto all_dates = generate_date_range(subtract_days(today, 90), today);
    int best = 0;
    int running = 0;
    for (const auto& d : all_dates) {
        if (date_set.count(d)) {
            running++;
            if (running > best) best = running;
        } else {
            running = 0;
        }
    }
    result.best = best;

    return result;
}

Streaks AnalyticsService::get_streaks(const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Workout dates (completed workouts, last 90 days)
    auto workout_result = txn.exec_params(
        "SELECT DISTINCT DATE(started_at) AS d "
        "FROM workouts "
        "WHERE user_id = $1 "
        "AND status = 'completed' "
        "AND started_at >= CURRENT_TIMESTAMP - INTERVAL '90 days' "
        "ORDER BY d ASC",
        user_id
    );

    // Nutrition logging dates (last 90 days)
    auto nutrition_result = txn.exec_params(
        "SELECT DISTINCT DATE(logged_at) AS d "
        "FROM nutrition_log "
        "WHERE user_id = $1 "
        "AND deleted_at IS NULL "
        "AND logged_at >= CURRENT_TIMESTAMP - INTERVAL '90 days' "
        "ORDER BY d ASC",
        user_id
    );

    // Weight logging dates (last 90 days)
    auto weight_result = txn.exec_params(
        "SELECT DISTINCT DATE(logged_at) AS d "
        "FROM weight_logs "
        "WHERE user_id = $1 "
        "AND logged_at >= CURRENT_TIMESTAMP - INTERVAL '90 days' "
        "ORDER BY d ASC",
        user_id
    );

    txn.commit();

    // Extract date strings
    std::vector<std::string> workout_dates, nutrition_dates, weight_dates;
    for (const auto& row : workout_result) {
        workout_dates.push_back(row["d"].c_str());
    }
    for (const auto& row : nutrition_result) {
        nutrition_dates.push_back(row["d"].c_str());
    }
    for (const auto& row : weight_result) {
        weight_dates.push_back(row["d"].c_str());
    }

    Streaks streaks;
    streaks.workout = calculate_streak(workout_dates);
    streaks.logging = calculate_streak(nutrition_dates);
    streaks.weight = calculate_streak(weight_dates);

    return streaks;
}

CheckInRecommendation AnalyticsService::generate_nutrition_checkin(
    const std::string& user_id)
{
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Get user's profile for formula TDEE and targets
    auto profile_result = txn.exec_params(
        "SELECT weight_kg, tdee_calories, target_calories, "
        "target_protein_g, target_carbs_g, target_fat_g, "
        "fitness_goal "
        "FROM user_profiles WHERE user_id = $1",
        user_id
    );

    if (profile_result.empty()) {
        txn.commit();
        throw std::invalid_argument("PROFILE_NOT_FOUND");
    }

    auto profile_row = profile_result[0];
    double formula_tdee = profile_row["tdee_calories"].is_null()
        ? 2400.0 : profile_row["tdee_calories"].as<double>();
    double current_target = profile_row["target_calories"].is_null()
        ? 2400.0 : profile_row["target_calories"].as<double>();
    double target_protein = profile_row["target_protein_g"].is_null()
        ? 150.0 : profile_row["target_protein_g"].as<double>();
    double target_carbs = profile_row["target_carbs_g"].is_null()
        ? 250.0 : profile_row["target_carbs_g"].as<double>();
    double target_fat = profile_row["target_fat_g"].is_null()
        ? 70.0 : profile_row["target_fat_g"].as<double>();
    std::string fitness_goal = profile_row["fitness_goal"].c_str();

    // Average daily calorie intake over last 7 days
    auto intake_result = txn.exec_params(
        "SELECT COALESCE(AVG(daily_total), 0) AS avg_daily_intake "
        "FROM ("
        "  SELECT DATE(logged_at) AS d, SUM(calories * quantity) AS daily_total "
        "  FROM nutrition_log "
        "  WHERE user_id = $1 "
        "  AND deleted_at IS NULL "
        "  AND logged_at >= CURRENT_TIMESTAMP - INTERVAL '7 days' "
        "  GROUP BY DATE(logged_at)"
        ") sub",
        user_id
    );

    double avg_daily_intake = intake_result[0]["avg_daily_intake"].as<double>();

    // Weight change over the last 7 days
    auto weight_result = txn.exec_params(
        "SELECT weight_kg, logged_at "
        "FROM weight_logs "
        "WHERE user_id = $1 "
        "AND logged_at >= CURRENT_TIMESTAMP - INTERVAL '8 days' "
        "ORDER BY logged_at ASC",
        user_id
    );

    txn.commit();

    double weight_change_weekly = 0;
    if (weight_result.size() >= 2) {
        double first_weight = weight_result[0]["weight_kg"].as<double>();
        double last_weight = weight_result[weight_result.size() - 1]["weight_kg"].as<double>();
        weight_change_weekly = last_weight - first_weight;
    }

    // Adaptive TDEE calculation:
    // 1 kg fat ~ 7700 kcal, so daily surplus/deficit = weight_change * 7700 / 7 = weight_change * 1100
    // adaptive_tdee = avg_daily_intake - (weight_change_weekly * 1100)
    double adaptive_tdee = avg_daily_intake - (weight_change_weekly * 1100.0);
    adaptive_tdee = std::round(adaptive_tdee);

    double calorie_adjustment = std::round(adaptive_tdee - formula_tdee);

    // Recommended target is the user's existing target (already goal-adjusted)
    double recommended_target = current_target;

    // Generate summary text
    std::string summary;
    if (std::abs(calorie_adjustment) < 100) {
        summary = "Your actual energy expenditure closely matches your formula estimate. Current targets look good.";
    } else if (calorie_adjustment > 0) {
        summary = "Your adaptive TDEE is higher than estimated. You may be burning more than expected. "
                  "Consider increasing intake if progress stalls.";
    } else {
        summary = "Your adaptive TDEE is lower than estimated. You may be burning less than expected. "
                  "Consider reducing intake slightly if not seeing expected results.";
    }

    CheckInRecommendation rec;
    rec.summary = summary;
    rec.adaptive_tdee_kcal = adaptive_tdee;
    rec.formula_tdee_kcal = formula_tdee;
    rec.recommended_calorie_target = recommended_target;
    rec.protein_target_g = target_protein;
    rec.carbs_target_g = target_carbs;
    rec.fat_target_g = target_fat;
    rec.calorie_adjustment = calorie_adjustment;

    return rec;
}

} // namespace forge
