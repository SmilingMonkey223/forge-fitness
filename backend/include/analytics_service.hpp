#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace forge {

struct MuscleGroupVolume {
    std::string muscle_group;
    int total_sets;
    double percentage;

    nlohmann::json to_json() const {
        return {
            {"muscle_group", muscle_group},
            {"total_sets", total_sets},
            {"percentage", percentage}
        };
    }
};

struct PersonalRecord {
    std::string id;
    std::string exercise_name;
    double weight_kg;
    int reps;
    std::string pr_type;
    double value;
    std::string date;

    nlohmann::json to_json() const {
        return {
            {"id", id},
            {"exercise_name", exercise_name},
            {"weight_kg", weight_kg},
            {"reps", reps},
            {"pr_type", pr_type},
            {"value", value},
            {"date", date}
        };
    }
};

struct ConsistencyPoint {
    std::string date;
    bool workout_completed;
    double volume_kg;

    nlohmann::json to_json() const {
        return {
            {"date", date},
            {"workout_completed", workout_completed},
            {"volume_kg", volume_kg}
        };
    }
};

struct StreakData {
    int current;
    int best;

    nlohmann::json to_json() const {
        return {
            {"current", current},
            {"best", best}
        };
    }
};

struct Streaks {
    StreakData workout;
    StreakData logging;
    StreakData weight;

    nlohmann::json to_json() const {
        return {
            {"workout", workout.to_json()},
            {"logging", logging.to_json()},
            {"weight", weight.to_json()}
        };
    }
};

struct CheckInRecommendation {
    std::string summary;
    double adaptive_tdee_kcal;
    double formula_tdee_kcal;
    double recommended_calorie_target;
    double protein_target_g;
    double carbs_target_g;
    double fat_target_g;
    double calorie_adjustment;

    nlohmann::json to_json() const {
        return {
            {"summary", summary},
            {"adaptive_tdee_kcal", adaptive_tdee_kcal},
            {"formula_tdee_kcal", formula_tdee_kcal},
            {"recommended_calorie_target", recommended_calorie_target},
            {"protein_target_g", protein_target_g},
            {"carbs_target_g", carbs_target_g},
            {"fat_target_g", fat_target_g},
            {"calorie_adjustment", calorie_adjustment}
        };
    }
};

class AnalyticsService {
public:
    explicit AnalyticsService(Database& db) : db_(db) {}

    // Muscle group volume distribution over the last N days
    std::vector<MuscleGroupVolume> get_muscle_distribution(
        const std::string& user_id,
        int days = 30
    );

    // Personal record history
    std::vector<PersonalRecord> get_pr_history(
        const std::string& user_id,
        int limit = 20
    );

    // Training consistency: for each day, was there a workout, total volume
    std::vector<ConsistencyPoint> get_training_consistency(
        const std::string& user_id,
        int days = 30
    );

    // Current and best streaks for workouts, nutrition logging, weight logging
    Streaks get_streaks(const std::string& user_id);

    // Weekly nutrition check-in analysis with adaptive TDEE
    CheckInRecommendation generate_nutrition_checkin(const std::string& user_id);

private:
    Database& db_;

    // Helper: calculate streak (current + best) from a list of dates
    StreakData calculate_streak(const std::vector<std::string>& dates);
};

} // namespace forge
