#pragma once

#include "models.hpp"
#include "database.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace forge {

class DashboardService {
public:
    // Nutrition totals for a single day
    struct NutritionToday {
        double calories = 0;
        double protein_g = 0;
        double carbs_g = 0;
        double fat_g = 0;
        double target_calories = 2400;
        double target_protein_g = 176;
        double target_carbs_g = 280;
        double target_fat_g = 80;

        json to_json() const {
            return {
                {"calories", {{"consumed", calories}, {"target", target_calories}}},
                {"protein_g", {{"consumed", protein_g}, {"target", target_protein_g}}},
                {"carbs_g", {{"consumed", carbs_g}, {"target", target_carbs_g}}},
                {"fat_g", {{"consumed", fat_g}, {"target", target_fat_g}}}
            };
        }
    };

    // Today's workout summary
    struct WorkoutToday {
        bool completed = false;
        std::string name;
        int duration_seconds = 0;
        double total_volume_kg = 0;
        int exercises_count = 0;
        int sets_count = 0;
        int prs_count = 0;

        json to_json() const {
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

    // Weekly overview data
    struct WeekOverview {
        std::vector<bool> workout_days;           // 7 booleans, Mon-Sun
        std::vector<double> daily_calories;        // 7 doubles, Mon-Sun (-1 = no data)
        std::vector<double> daily_protein;         // 7 doubles, Mon-Sun (-1 = no data)
        double calorie_target = 2400;
        double protein_target = 176;
        int current_streak = 0;

        json to_json() const {
            json cal_array = json::array();
            json prot_array = json::array();
            for (size_t i = 0; i < daily_calories.size(); ++i) {
                if (daily_calories[i] < 0) {
                    cal_array.push_back(nullptr);
                } else {
                    cal_array.push_back(daily_calories[i]);
                }
            }
            for (size_t i = 0; i < daily_protein.size(); ++i) {
                if (daily_protein[i] < 0) {
                    prot_array.push_back(nullptr);
                } else {
                    prot_array.push_back(daily_protein[i]);
                }
            }

            return {
                {"workout_days", workout_days},
                {"daily_calories", cal_array},
                {"daily_protein", prot_array},
                {"calorie_target", calorie_target},
                {"protein_target", protein_target},
                {"current_streak", current_streak}
            };
        }
    };

    // Full dashboard response
    struct DashboardResponse {
        std::string date;
        NutritionToday nutrition;
        std::optional<WorkoutToday> workout;
        WeekOverview week;
        bool has_profile = false;

        json to_json() const {
            json workout_json = nullptr;
            if (workout.has_value()) {
                workout_json = workout->to_json();
            }

            return {
                {"today", {
                    {"date", date},
                    {"nutrition", nutrition.to_json()},
                    {"workout", workout_json}
                }},
                {"week", week.to_json()},
                {"has_profile", has_profile}
            };
        }
    };

    // Main entry point: fetch all dashboard data for a user
    static DashboardResponse get_dashboard(const std::string& user_id);

private:
    // Load macro targets from user_profiles, or use defaults
    static void load_targets(pqxx::work& txn,
                             const std::string& user_id,
                             NutritionToday& nutrition,
                             WeekOverview& week,
                             bool& has_profile);

    // Aggregate today's nutrition from nutrition_log
    static void load_nutrition_today(pqxx::work& txn,
                                     const std::string& user_id,
                                     const std::string& today_date,
                                     NutritionToday& nutrition);

    // Load today's most recent completed workout summary
    static std::optional<WorkoutToday> load_workout_today(pqxx::work& txn,
                                                          const std::string& user_id,
                                                          const std::string& today_date);

    // Build the week overview (workout days, nutrition trends, streak)
    static WeekOverview load_week_overview(pqxx::work& txn,
                                           const std::string& user_id,
                                           const std::string& today_date,
                                           double calorie_target,
                                           double protein_target);
};

} // namespace forge
