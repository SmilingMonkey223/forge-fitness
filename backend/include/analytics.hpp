#pragma once

#include "database.hpp"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <chrono>

namespace forge {

// Exercise progression data point
struct ExerciseProgressionPoint {
    std::chrono::system_clock::time_point date;
    float max_weight_kg;
    float total_volume_kg;
    float estimated_1rm_kg;
};

// Muscle group volume data
struct MuscleGroupVolume {
    std::string muscle_group;
    int total_sets;
    float total_volume_kg;
    float percentage;
};

// Personal record
struct PersonalRecord {
    std::string id;
    std::string exercise_name;
    std::string exercise_id;
    std::string pr_type;  // "weight", "volume", "1rm"
    float value;
    int reps;
    float weight_kg;
    std::chrono::system_clock::time_point date;
};

// Training consistency data point
struct ConsistencyPoint {
    std::chrono::sys_days date;
    bool workout_completed;
    float volume_kg;
};

// Macro intake data point
struct MacroIntakePoint {
    std::chrono::sys_days date;
    float calories;
    float protein_g;
    float carbs_g;
    float fat_g;
    float target_calories;
};

// Analytics summary
struct AnalyticsSummary {
    // Training metrics
    int current_workout_streak;
    int workouts_this_week;
    int workouts_this_month;
    int total_prs;

    // Nutrition metrics
    float avg_calories_7day;
    float avg_protein_7day;
    int days_hit_calorie_target_30day;
    int days_hit_protein_target_30day;
    float calorie_adherence_percent;
    float protein_adherence_percent;
};

/**
 * Analytics service for workout and nutrition insights
 */
class Analytics {
public:
    explicit Analytics(std::shared_ptr<Database> db);
    ~Analytics() = default;

    // Workout Analytics

    // Get exercise progression (weight, volume, 1RM over time)
    std::vector<ExerciseProgressionPoint> get_exercise_progression(
        const std::string& user_id,
        const std::string& exercise_id,
        int days_lookback = 90
    );

    // Get muscle group distribution for a time period
    std::vector<MuscleGroupVolume> get_muscle_distribution(
        const std::string& user_id,
        int weeks_lookback = 4
    );

    // Get weekly volume per muscle group
    std::vector<MuscleGroupVolume> get_weekly_volume_per_muscle(
        const std::string& user_id,
        const std::string& week_iso  // e.g., "2026-W08"
    );

    // Get PR history
    std::vector<PersonalRecord> get_pr_history(
        const std::string& user_id,
        std::optional<std::string> exercise_id = std::nullopt,
        std::optional<std::string> pr_type = std::nullopt
    );

    // Get training consistency (calendar heatmap data)
    std::vector<ConsistencyPoint> get_training_consistency(
        const std::string& user_id,
        int year
    );

    // Nutrition Analytics

    // Get macro intake over time
    std::vector<MacroIntakePoint> get_macro_intake(
        const std::string& user_id,
        int days_lookback = 30
    );

    // Get calorie adherence rate
    float get_calorie_adherence_rate(
        const std::string& user_id,
        int days_lookback = 30,
        float tolerance_percent = 10.0f
    );

    // Get protein hit rate
    float get_protein_hit_rate(
        const std::string& user_id,
        int days_lookback = 30
    );

    // Combined Analytics

    // Get comprehensive analytics summary
    std::optional<AnalyticsSummary> get_summary(const std::string& user_id);

private:
    std::shared_ptr<Database> db_;

    // Calculate estimated 1RM using Epley formula
    float calculate_1rm(float weight_kg, int reps);

    // Detect if a set is a new PR
    bool is_new_pr(
        const std::string& user_id,
        const std::string& exercise_id,
        float weight_kg,
        int reps,
        const std::chrono::system_clock::time_point& date
    );
};

} // namespace forge
