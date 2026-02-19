#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace forge {

struct User {
    std::string id;
    std::string email;
    std::string username;
    std::string display_name;
    std::string password_hash;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::optional<std::chrono::system_clock::time_point> deleted_at;

    json to_json() const {
        return {
            {"id", id},
            {"email", email},
            {"username", username},
            {"display_name", display_name}
        };
    }
};

struct UserProfile {
    std::string id;
    std::string user_id;
    std::string date_of_birth;
    std::string sex;
    double height_cm;
    double weight_kg;
    std::string activity_level;
    std::string fitness_goal;
    std::string unit_preference;
    std::optional<double> tdee_calories;
    std::optional<double> target_calories;
    std::optional<double> target_protein_g;
    std::optional<double> target_carbs_g;
    std::optional<double> target_fat_g;

    json to_json() const {
        json j = {
            {"id", id},
            {"user_id", user_id},
            {"date_of_birth", date_of_birth},
            {"sex", sex},
            {"height_cm", height_cm},
            {"weight_kg", weight_kg},
            {"activity_level", activity_level},
            {"fitness_goal", fitness_goal},
            {"unit_preference", unit_preference}
        };

        if (tdee_calories) j["tdee_calories"] = *tdee_calories;
        if (target_calories) j["target_calories"] = *target_calories;
        if (target_protein_g) j["target_protein_g"] = *target_protein_g;
        if (target_carbs_g) j["target_carbs_g"] = *target_carbs_g;
        if (target_fat_g) j["target_fat_g"] = *target_fat_g;

        return j;
    }
};

struct Exercise {
    std::string id;
    std::string name;
    std::string muscle_group;
    std::string equipment;
    bool is_custom;
    std::optional<std::string> created_by;

    json to_json() const {
        json j = {
            {"id", id},
            {"name", name},
            {"muscle_group", muscle_group},
            {"equipment", equipment},
            {"is_custom", is_custom}
        };

        if (created_by) j["created_by"] = *created_by;

        return j;
    }
};

struct ExerciseSet {
    std::string id;
    std::string workout_id;
    std::string exercise_id;
    int set_order;
    int exercise_order;
    std::string set_type;
    std::optional<int> reps;
    std::optional<double> weight_kg;
    std::optional<int> duration_seconds;
    std::optional<double> rpe;
    std::optional<int> rest_seconds;
    bool is_pr;
    std::optional<std::string> notes;

    json to_json() const {
        json j = {
            {"id", id},
            {"workout_id", workout_id},
            {"exercise_id", exercise_id},
            {"set_order", set_order},
            {"exercise_order", exercise_order},
            {"set_type", set_type},
            {"is_pr", is_pr}
        };

        if (reps) j["reps"] = *reps;
        if (weight_kg) j["weight_kg"] = *weight_kg;
        if (duration_seconds) j["duration_seconds"] = *duration_seconds;
        if (rpe) j["rpe"] = *rpe;
        if (rest_seconds) j["rest_seconds"] = *rest_seconds;
        if (notes) j["notes"] = *notes;

        return j;
    }
};

struct Workout {
    std::string id;
    std::string user_id;
    std::optional<std::string> name;
    std::chrono::system_clock::time_point started_at;
    std::optional<std::chrono::system_clock::time_point> completed_at;
    std::optional<int> duration_seconds;
    std::optional<std::string> notes;
    std::string status;
    std::vector<ExerciseSet> sets;

    json to_json() const;
};

struct NutritionLog {
    std::string id;
    std::string user_id;
    std::chrono::system_clock::time_point logged_at;
    std::optional<std::string> meal_type;
    std::string food_name;
    std::optional<std::string> brand;
    double serving_size;
    std::string serving_unit;
    double quantity;
    double calories;
    double protein_g;
    double carbs_g;
    double fat_g;
    std::optional<double> fiber_g;
    std::optional<double> sugar_g;
    std::optional<double> sodium_mg;
    bool is_custom;
    std::string source;

    json to_json() const;
};

} // namespace forge
