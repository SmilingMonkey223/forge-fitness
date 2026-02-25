#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace forge {

struct NutritionToday {
    double calories_consumed = 0;
    double calories_target = 2400;
    double protein_consumed = 0;
    double protein_target = 176;
    double carbs_consumed = 0;
    double carbs_target = 280;
    double fat_consumed = 0;
    double fat_target = 80;
};

struct WorkoutToday {
    bool completed = false;
    std::string name;
    int duration_seconds = 0;
    double total_volume_kg = 0;
    int exercises_count = 0;
    int sets_count = 0;
    int prs_count = 0;

    nlohmann::json to_json() const {
        return {
            {"completed", completed},
            {"name", name},
            {"duration_seconds", duration_seconds},
            {"total_volume_kg", total_volume_kg},
            {"exercises_count", exercises_count},
            {"sets_count", sets_count},
            {"prs_count", prs_count}
        };
    }
};

struct WeekOverview {
    std::vector<bool> workout_days;
    std::vector<nlohmann::json> daily_calories;
    std::vector<nlohmann::json> daily_protein;
    double calorie_target = 2400;
    double protein_target = 176;
    int current_streak = 0;
};

struct DashboardResponse {
    std::string date;
    NutritionToday nutrition;
    std::optional<WorkoutToday> workout;
    WeekOverview week;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"today", {
                {"date", date},
                {"nutrition", {
                    {"calories", {{"consumed", nutrition.calories_consumed}, {"target", nutrition.calories_target}}},
                    {"protein_g", {{"consumed", nutrition.protein_consumed}, {"target", nutrition.protein_target}}},
                    {"carbs_g", {{"consumed", nutrition.carbs_consumed}, {"target", nutrition.carbs_target}}},
                    {"fat_g", {{"consumed", nutrition.fat_consumed}, {"target", nutrition.fat_target}}}
                }},
                {"workout", workout ? workout->to_json() : nlohmann::json(nullptr)}
            }},
            {"week", {
                {"workout_days", week.workout_days},
                {"daily_calories", week.daily_calories},
                {"daily_protein", week.daily_protein},
                {"calorie_target", week.calorie_target},
                {"protein_target", week.protein_target},
                {"current_streak", week.current_streak}
            }}
        };
        return j;
    }
};

class DashboardService {
public:
    explicit DashboardService(Database& db) : db_(db) {}

    DashboardResponse get_dashboard(const std::string& user_id);

private:
    Database& db_;

    NutritionToday get_nutrition_today(pqxx::work& txn, const std::string& user_id, const std::string& date);
    std::optional<WorkoutToday> get_workout_today(pqxx::work& txn, const std::string& user_id, const std::string& date);
    WeekOverview get_week_overview(pqxx::work& txn, const std::string& user_id, const std::string& today);
    int calculate_streak(pqxx::work& txn, const std::string& user_id, const std::string& today);
};

} // namespace forge
