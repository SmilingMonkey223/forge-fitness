#pragma once

#include "database.hpp"
#include <string>
#include <optional>
#include <memory>

namespace forge {

// TDEE calculation result
struct TDEEResult {
    float formula_tdee_kcal;    // From Mifflin-St Jeor equation
    std::optional<float> adaptive_tdee_kcal;  // From actual data (nullable if not enough data)

    // Data quality indicators
    bool has_sufficient_data;
    int days_tracked;
    int weigh_ins_count;
    int nutrition_days_count;

    // Calculation details (for debugging/transparency)
    float average_daily_intake_kcal;
    float weight_change_kg;
    float weight_change_per_day_kg;
};

// Weekly check-in recommendation
struct CheckInRecommendation {
    // Current metrics
    float current_weight_kg;
    float target_rate_kg_per_week;
    float actual_rate_kg_per_week;

    // TDEE
    float adaptive_tdee_kcal;
    float formula_tdee_kcal;

    // Target adjustments
    float current_calorie_target;
    float recommended_calorie_target;
    float calorie_adjustment;

    // Macro recommendations
    float protein_target_g;
    float carbs_target_g;
    float fat_target_g;

    // Progress summary
    std::string summary_message;
};

/**
 * Adaptive TDEE calculator
 * Calculates energy expenditure from weight change + intake (MacroFactor-style)
 */
class TDEECalculator {
public:
    explicit TDEECalculator(std::shared_ptr<Database> db);
    ~TDEECalculator() = default;

    // Calculate formula-based TDEE (Mifflin-St Jeor)
    float calculate_formula_tdee(
        float weight_kg,
        float height_cm,
        int age_years,
        const std::string& sex,  // "male" or "female"
        const std::string& activity_level
    );

    // Calculate adaptive TDEE from actual data
    std::optional<TDEEResult> calculate_adaptive_tdee(
        const std::string& user_id,
        int lookback_days = 14
    );

    // Generate weekly check-in recommendation
    std::optional<CheckInRecommendation> generate_checkin(
        const std::string& user_id
    );

    // Apply check-in recommendations
    bool apply_checkin_targets(
        const std::string& user_id,
        const std::string& checkin_id,
        bool accepted,
        std::optional<float> manual_calorie_override = std::nullopt
    );

private:
    std::shared_ptr<Database> db_;

    // Energy density of tissue change (kcal/kg)
    float get_energy_density(float weight_change_per_day);

    // Activity multipliers for Mifflin-St Jeor
    float get_activity_multiplier(const std::string& activity_level);

    // Calculate macro targets from calorie target
    struct MacroTargets {
        float protein_g;
        float carbs_g;
        float fat_g;
    };
    MacroTargets calculate_macros(float calorie_target, float weight_kg, const std::string& goal);
};

} // namespace forge
