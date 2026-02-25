#include "../../include/dashboard_service.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace forge {

// Helper: get today's date as "YYYY-MM-DD" string in local time
static std::string get_today_date() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&time_t_now, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d");
    return oss.str();
}

// Helper: get the Monday of the week containing `date_str` (ISO format)
static std::string get_week_start(const std::string& date_str) {
    std::tm tm_buf{};
    std::istringstream iss(date_str);
    iss >> std::get_time(&tm_buf, "%Y-%m-%d");
    // mktime normalizes the struct, filling in tm_wday
    mktime(&tm_buf);
    // tm_wday: 0=Sunday, 1=Monday, ..., 6=Saturday
    // We want Monday = 0
    int days_since_monday = (tm_buf.tm_wday + 6) % 7;
    tm_buf.tm_mday -= days_since_monday;
    mktime(&tm_buf); // re-normalize
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d");
    return oss.str();
}

void DashboardService::load_targets(pqxx::work& txn,
                                     const std::string& user_id,
                                     NutritionToday& nutrition,
                                     WeekOverview& week,
                                     bool& has_profile) {
    auto result = txn.exec_params(
        "SELECT target_calories, target_protein_g, target_carbs_g, target_fat_g "
        "FROM user_profiles WHERE user_id = $1",
        user_id
    );

    if (!result.empty()) {
        has_profile = true;
        auto row = result[0];

        if (!row["target_calories"].is_null()) {
            nutrition.target_calories = row["target_calories"].as<double>();
        }
        if (!row["target_protein_g"].is_null()) {
            nutrition.target_protein_g = row["target_protein_g"].as<double>();
        }
        if (!row["target_carbs_g"].is_null()) {
            nutrition.target_carbs_g = row["target_carbs_g"].as<double>();
        }
        if (!row["target_fat_g"].is_null()) {
            nutrition.target_fat_g = row["target_fat_g"].as<double>();
        }

        week.calorie_target = nutrition.target_calories;
        week.protein_target = nutrition.target_protein_g;
    } else {
        has_profile = false;
        // Keep defaults from struct initialization
    }
}

void DashboardService::load_nutrition_today(pqxx::work& txn,
                                             const std::string& user_id,
                                             const std::string& today_date,
                                             NutritionToday& nutrition) {
    auto result = txn.exec_params(
        "SELECT "
        "  COALESCE(SUM(calories * quantity), 0) AS total_calories, "
        "  COALESCE(SUM(protein_g * quantity), 0) AS total_protein, "
        "  COALESCE(SUM(carbs_g * quantity), 0) AS total_carbs, "
        "  COALESCE(SUM(fat_g * quantity), 0) AS total_fat "
        "FROM nutrition_log "
        "WHERE user_id = $1 "
        "  AND DATE(logged_at) = $2::date "
        "  AND deleted_at IS NULL",
        user_id, today_date
    );

    if (!result.empty()) {
        auto row = result[0];
        nutrition.calories = row["total_calories"].as<double>();
        nutrition.protein_g = row["total_protein"].as<double>();
        nutrition.carbs_g = row["total_carbs"].as<double>();
        nutrition.fat_g = row["total_fat"].as<double>();
    }
}

std::optional<DashboardService::WorkoutToday>
DashboardService::load_workout_today(pqxx::work& txn,
                                      const std::string& user_id,
                                      const std::string& today_date) {
    // Find the most recent completed workout for today
    auto workout_result = txn.exec_params(
        "SELECT w.id, w.name, w.duration_seconds, "
        "  COUNT(DISTINCT es.exercise_id) AS exercises_count, "
        "  COUNT(es.id) AS sets_count, "
        "  COALESCE(SUM(CASE WHEN es.weight_kg IS NOT NULL AND es.reps IS NOT NULL "
        "    THEN es.weight_kg * es.reps ELSE 0 END), 0) AS total_volume_kg, "
        "  COALESCE(SUM(CASE WHEN es.is_pr THEN 1 ELSE 0 END), 0) AS prs_count "
        "FROM workouts w "
        "LEFT JOIN exercise_sets es ON es.workout_id = w.id "
        "WHERE w.user_id = $1 "
        "  AND DATE(w.started_at) = $2::date "
        "  AND w.status = 'completed' "
        "  AND w.deleted_at IS NULL "
        "GROUP BY w.id, w.name, w.duration_seconds "
        "ORDER BY w.started_at DESC "
        "LIMIT 1",
        user_id, today_date
    );

    if (workout_result.empty()) {
        return std::nullopt;
    }

    auto row = workout_result[0];

    // If there's a row but exercises_count is 0 and the id is null,
    // it means the LEFT JOIN produced no match -- but we did GROUP BY w.id,
    // so an empty result means no workout at all.
    WorkoutToday wt;
    wt.completed = true;
    wt.name = row["name"].is_null() ? "Workout" : row["name"].c_str();
    wt.duration_seconds = row["duration_seconds"].is_null() ? 0 : row["duration_seconds"].as<int>();
    wt.exercises_count = row["exercises_count"].as<int>();
    wt.sets_count = row["sets_count"].as<int>();
    wt.total_volume_kg = row["total_volume_kg"].as<double>();
    wt.prs_count = row["prs_count"].as<int>();

    return wt;
}

DashboardService::WeekOverview
DashboardService::load_week_overview(pqxx::work& txn,
                                      const std::string& user_id,
                                      const std::string& today_date,
                                      double calorie_target,
                                      double protein_target) {
    WeekOverview week;
    week.calorie_target = calorie_target;
    week.protein_target = protein_target;
    week.workout_days.resize(7, false);
    week.daily_calories.resize(7, -1.0);
    week.daily_protein.resize(7, -1.0);

    std::string week_start = get_week_start(today_date);

    // Query workout days for this week
    auto workout_days_result = txn.exec_params(
        "SELECT DISTINCT DATE(started_at) AS workout_date "
        "FROM workouts "
        "WHERE user_id = $1 "
        "  AND DATE(started_at) >= $2::date "
        "  AND DATE(started_at) < ($2::date + INTERVAL '7 days') "
        "  AND status = 'completed' "
        "  AND deleted_at IS NULL",
        user_id, week_start
    );

    // Parse week_start into a tm for day offset calculations
    std::tm week_start_tm{};
    {
        std::istringstream iss(week_start);
        iss >> std::get_time(&week_start_tm, "%Y-%m-%d");
        mktime(&week_start_tm);
    }

    for (const auto& row : workout_days_result) {
        std::string date_str = row["workout_date"].c_str();
        std::tm date_tm{};
        std::istringstream iss(date_str);
        iss >> std::get_time(&date_tm, "%Y-%m-%d");
        mktime(&date_tm);

        // Calculate day index (0 = Monday)
        auto start_time = std::mktime(&week_start_tm);
        auto cur_time = std::mktime(&date_tm);
        int day_idx = static_cast<int>(std::difftime(cur_time, start_time) / 86400.0);
        if (day_idx >= 0 && day_idx < 7) {
            week.workout_days[day_idx] = true;
        }
    }

    // Query daily nutrition for this week
    auto nutrition_week_result = txn.exec_params(
        "SELECT DATE(logged_at) AS log_date, "
        "  COALESCE(SUM(calories * quantity), 0) AS total_calories, "
        "  COALESCE(SUM(protein_g * quantity), 0) AS total_protein "
        "FROM nutrition_log "
        "WHERE user_id = $1 "
        "  AND DATE(logged_at) >= $2::date "
        "  AND DATE(logged_at) < ($2::date + INTERVAL '7 days') "
        "  AND deleted_at IS NULL "
        "GROUP BY DATE(logged_at)",
        user_id, week_start
    );

    for (const auto& row : nutrition_week_result) {
        std::string date_str = row["log_date"].c_str();
        std::tm date_tm{};
        std::istringstream iss(date_str);
        iss >> std::get_time(&date_tm, "%Y-%m-%d");
        mktime(&date_tm);

        auto start_time = std::mktime(&week_start_tm);
        auto cur_time = std::mktime(&date_tm);
        int day_idx = static_cast<int>(std::difftime(cur_time, start_time) / 86400.0);
        if (day_idx >= 0 && day_idx < 7) {
            week.daily_calories[day_idx] = row["total_calories"].as<double>();
            week.daily_protein[day_idx] = row["total_protein"].as<double>();
        }
    }

    // Calculate workout streak (consecutive days with completed workouts ending today or yesterday)
    auto streak_result = txn.exec_params(
        "SELECT DISTINCT DATE(started_at) AS workout_date "
        "FROM workouts "
        "WHERE user_id = $1 "
        "  AND DATE(started_at) <= $2::date "
        "  AND DATE(started_at) >= ($2::date - INTERVAL '30 days') "
        "  AND status = 'completed' "
        "  AND deleted_at IS NULL "
        "ORDER BY workout_date DESC",
        user_id, today_date
    );

    if (!streak_result.empty()) {
        // Parse today's date for comparison
        std::tm today_tm{};
        {
            std::istringstream iss(today_date);
            iss >> std::get_time(&today_tm, "%Y-%m-%d");
            mktime(&today_tm);
        }

        int streak = 0;
        // expected_date starts at today; if today has no workout, try yesterday
        auto expected_time = std::mktime(&today_tm);

        for (const auto& row : streak_result) {
            std::string date_str = row["workout_date"].c_str();
            std::tm date_tm{};
            std::istringstream iss(date_str);
            iss >> std::get_time(&date_tm, "%Y-%m-%d");
            auto row_time = std::mktime(&date_tm);

            if (streak == 0) {
                // First row: must be today or yesterday to start a streak
                double diff_days = std::difftime(expected_time, row_time) / 86400.0;
                if (diff_days < -0.5 || diff_days > 1.5) {
                    break; // No recent workout, streak is 0
                }
                streak = 1;
                // Next expected date is the day before this one
                expected_time = row_time - 86400;
            } else {
                // Subsequent rows: must be exactly one day before expected
                double diff = std::difftime(expected_time, row_time) / 86400.0;
                if (diff < -0.5 || diff > 0.5) {
                    break; // Gap found, streak ends
                }
                streak++;
                expected_time = row_time - 86400;
            }
        }

        week.current_streak = streak;
    }

    return week;
}

DashboardService::DashboardResponse
DashboardService::get_dashboard(const std::string& user_id) {
    DashboardResponse response;
    response.date = get_today_date();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    // 1. Load user targets from profile
    load_targets(txn, user_id, response.nutrition, response.week, response.has_profile);

    // 2. Load today's nutrition totals
    load_nutrition_today(txn, user_id, response.date, response.nutrition);

    // 3. Load today's workout summary
    response.workout = load_workout_today(txn, user_id, response.date);

    // 4. Load week overview (workout days, nutrition trends, streak)
    response.week = load_week_overview(txn, user_id, response.date,
                                        response.nutrition.target_calories,
                                        response.nutrition.target_protein_g);

    txn.commit();

    return response;
}

} // namespace forge
