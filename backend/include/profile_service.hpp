#pragma once

#include "models.hpp"
#include "database.hpp"
#include "uuid.hpp"
#include <string>
#include <optional>
#include <cmath>
#include <chrono>
#include <stdexcept>
#include <unordered_map>

namespace forge {

class ProfileService {
public:
    struct OnboardingRequest {
        std::string user_id;
        std::string date_of_birth;  // YYYY-MM-DD
        std::string sex;            // male, female, other
        double height_cm;
        double weight_kg;
        std::string activity_level; // sedentary, lightly_active, moderately_active, very_active, extremely_active
        std::string fitness_goal;   // lose_fat, maintain, build_muscle
        std::string unit_preference = "metric";
    };

    struct UpdateProfileRequest {
        std::string user_id;
        std::optional<std::string> date_of_birth;
        std::optional<std::string> sex;
        std::optional<double> height_cm;
        std::optional<double> weight_kg;
        std::optional<std::string> activity_level;
        std::optional<std::string> fitness_goal;
        std::optional<std::string> unit_preference;
    };

    struct MacroTargets {
        double tdee_calories;
        double target_calories;
        double target_protein_g;
        double target_carbs_g;
        double target_fat_g;
    };

    // Calculate age from date of birth string (YYYY-MM-DD)
    static int calculate_age(const std::string& date_of_birth);

    // Calculate BMR using Mifflin-St Jeor equation
    static double calculate_bmr(const std::string& sex, double weight_kg, double height_cm, int age);

    // Get activity multiplier
    static double get_activity_multiplier(const std::string& activity_level);

    // Calculate TDEE = BMR * activity multiplier
    static double calculate_tdee(const std::string& sex, double weight_kg, double height_cm,
                                  int age, const std::string& activity_level);

    // Calculate macro targets based on fitness goal
    static MacroTargets calculate_macros(const std::string& sex, double weight_kg, double height_cm,
                                          int age, const std::string& activity_level,
                                          const std::string& fitness_goal);

    // Get profile by user_id - returns nullopt if not found
    static std::optional<UserProfile> get_profile(const std::string& user_id);

    // Complete onboarding - creates profile with calculated TDEE and macros
    static UserProfile complete_onboarding(const OnboardingRequest& req);

    // Update existing profile
    static UserProfile update_profile(const UpdateProfileRequest& req);

private:
    // Validation helpers
    static void validate_sex(const std::string& sex);
    static void validate_activity_level(const std::string& activity_level);
    static void validate_fitness_goal(const std::string& fitness_goal);
    static void validate_date_of_birth(const std::string& dob);
    static void validate_height(double height_cm);
    static void validate_weight(double weight_kg);
};

} // namespace forge
