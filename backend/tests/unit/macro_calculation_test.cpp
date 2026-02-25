#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <algorithm>

// Test macro target calculation logic.
// This mirrors ProfileService::calculate_macros from profile_service.cpp.
// Actual ratios from the code:
//   lose_fat:     protein = 2.0 g/kg, fat = 0.9 g/kg, calories = TDEE - 500
//   maintain:     protein = 1.8 g/kg, fat = 1.0 g/kg, calories = TDEE
//   build_muscle: protein = 2.2 g/kg, fat = 1.0 g/kg, calories = TDEE + 300
//   carbs: fill remaining calories (4 cal/g protein, 4 cal/g carbs, 9 cal/g fat)

namespace {

struct MacroTargets {
    double tdee_calories;
    double target_calories;
    double target_protein_g;
    double target_carbs_g;
    double target_fat_g;
};

// Mirrors ProfileService::calculate_macros
MacroTargets calculate_macros(double tdee, double weight_kg,
                              const std::string& fitness_goal) {
    MacroTargets targets{};
    targets.tdee_calories = std::round(tdee);

    double protein_per_kg = 0.0;
    double fat_per_kg = 0.0;

    if (fitness_goal == "lose_fat") {
        targets.target_calories = std::round(tdee - 500.0);
        protein_per_kg = 2.0;
        fat_per_kg = 0.9;
    } else if (fitness_goal == "maintain") {
        targets.target_calories = std::round(tdee);
        protein_per_kg = 1.8;
        fat_per_kg = 1.0;
    } else if (fitness_goal == "build_muscle") {
        targets.target_calories = std::round(tdee + 300.0);
        protein_per_kg = 2.2;
        fat_per_kg = 1.0;
    }

    targets.target_protein_g = std::round(protein_per_kg * weight_kg);
    targets.target_fat_g = std::round(fat_per_kg * weight_kg);

    double protein_calories = targets.target_protein_g * 4.0;
    double fat_calories = targets.target_fat_g * 9.0;
    double remaining_calories = targets.target_calories - protein_calories - fat_calories;
    targets.target_carbs_g = std::round(std::max(0.0, remaining_calories / 4.0));

    return targets;
}

// Verify that macros reconstruct to approximately the calorie target
void verify_calorie_consistency(const MacroTargets& m, double tolerance = 2.0) {
    double total = m.target_protein_g * 4.0 + m.target_carbs_g * 4.0 + m.target_fat_g * 9.0;
    // Due to rounding, allow some tolerance
    if (m.target_carbs_g > 0.0) {
        EXPECT_NEAR(total, m.target_calories, tolerance)
            << "Macros should approximate calorie target. "
            << "protein=" << m.target_protein_g << "g, carbs=" << m.target_carbs_g
            << "g, fat=" << m.target_fat_g << "g -> " << total << " kcal";
    }
}

} // anonymous namespace

// ── Build muscle ────────────────────────────────────────────────────

TEST(MacroCalculationUnit, BuildMuscle80kg) {
    // 80kg, build_muscle, TDEE=2759 -> target=3059
    auto m = calculate_macros(2759.0, 80.0, "build_muscle");

    EXPECT_DOUBLE_EQ(m.target_calories, std::round(2759.0 + 300.0));
    // protein = round(80 * 2.2) = 176g
    EXPECT_DOUBLE_EQ(m.target_protein_g, 176.0);
    // fat = round(80 * 1.0) = 80g
    EXPECT_DOUBLE_EQ(m.target_fat_g, 80.0);
    // carbs = round((3059 - 176*4 - 80*9) / 4) = round((3059 - 704 - 720) / 4) = round(408.75) = 409
    EXPECT_DOUBLE_EQ(m.target_carbs_g, 409.0);

    verify_calorie_consistency(m);
}

// ── Lose fat ────────────────────────────────────────────────────────

TEST(MacroCalculationUnit, LoseFat80kg) {
    // 80kg, lose_fat, TDEE=2759 -> target=2259
    auto m = calculate_macros(2759.0, 80.0, "lose_fat");

    EXPECT_DOUBLE_EQ(m.target_calories, std::round(2759.0 - 500.0));
    // protein = round(80 * 2.0) = 160g
    EXPECT_DOUBLE_EQ(m.target_protein_g, 160.0);
    // fat = round(80 * 0.9) = 72g
    EXPECT_DOUBLE_EQ(m.target_fat_g, 72.0);
    // carbs = round((2259 - 160*4 - 72*9) / 4) = round((2259 - 640 - 648) / 4) = round(242.75) = 243
    EXPECT_DOUBLE_EQ(m.target_carbs_g, 243.0);

    verify_calorie_consistency(m);
}

// ── Maintain ────────────────────────────────────────────────────────

TEST(MacroCalculationUnit, Maintain80kg) {
    // 80kg, maintain, TDEE=2759 -> target=2759
    auto m = calculate_macros(2759.0, 80.0, "maintain");

    EXPECT_DOUBLE_EQ(m.target_calories, std::round(2759.0));
    // protein = round(80 * 1.8) = 144g
    EXPECT_DOUBLE_EQ(m.target_protein_g, 144.0);
    // fat = round(80 * 1.0) = 80g
    EXPECT_DOUBLE_EQ(m.target_fat_g, 80.0);
    // carbs = round((2759 - 144*4 - 80*9) / 4) = round((2759 - 576 - 720) / 4) = round(365.75) = 366
    EXPECT_DOUBLE_EQ(m.target_carbs_g, 366.0);

    verify_calorie_consistency(m);
}

// ── Different weights ───────────────────────────────────────────────

TEST(MacroCalculationUnit, LightPerson50kg) {
    auto m = calculate_macros(1800.0, 50.0, "maintain");

    // protein = round(50 * 1.8) = 90g
    EXPECT_DOUBLE_EQ(m.target_protein_g, 90.0);
    // fat = round(50 * 1.0) = 50g
    EXPECT_DOUBLE_EQ(m.target_fat_g, 50.0);
    verify_calorie_consistency(m);
}

TEST(MacroCalculationUnit, HeavyPerson120kg) {
    auto m = calculate_macros(3500.0, 120.0, "build_muscle");

    // protein = round(120 * 2.2) = 264g
    EXPECT_DOUBLE_EQ(m.target_protein_g, 264.0);
    // fat = round(120 * 1.0) = 120g
    EXPECT_DOUBLE_EQ(m.target_fat_g, 120.0);
    verify_calorie_consistency(m);
}

// ── Edge case: very low TDEE (carbs should not go negative) ─────────

TEST(MacroCalculationUnit, VeryLowTDEECarbsFloorAtZero) {
    // If calorie target is very low, protein + fat might exceed it
    auto m = calculate_macros(1300.0, 80.0, "lose_fat");

    // target = 1300 - 500 = 800
    // protein = round(80*2.0) = 160g -> 640 cal
    // fat = round(80*0.9) = 72g -> 648 cal
    // remaining = 800 - 640 - 648 = -488 -> carbs clamped to 0
    EXPECT_DOUBLE_EQ(m.target_protein_g, 160.0);
    EXPECT_DOUBLE_EQ(m.target_fat_g, 72.0);
    EXPECT_DOUBLE_EQ(m.target_carbs_g, 0.0);
}

TEST(MacroCalculationUnit, ExactlyEnoughForProteinAndFat) {
    // Target where protein+fat calories exactly match target
    // protein = round(80*2.2)=176g -> 704 cal, fat = round(80*1.0)=80g -> 720 cal
    // Need target_cal = 704 + 720 = 1424 -> TDEE = 1424 - 300 = 1124
    auto m = calculate_macros(1124.0, 80.0, "build_muscle");

    EXPECT_DOUBLE_EQ(m.target_protein_g, 176.0);
    EXPECT_DOUBLE_EQ(m.target_fat_g, 80.0);
    EXPECT_DOUBLE_EQ(m.target_carbs_g, 0.0);
}

// ── Calorie consistency for various scenarios ───────────────────────

TEST(MacroCalculationUnit, CalorieConsistencyAcrossGoals) {
    double tdees[] = {1500.0, 2000.0, 2500.0, 3000.0, 3500.0};
    std::string goals[] = {"lose_fat", "maintain", "build_muscle"};
    double weights[] = {60.0, 80.0, 100.0};

    for (double tdee : tdees) {
        for (const auto& goal : goals) {
            for (double weight : weights) {
                auto m = calculate_macros(tdee, weight, goal);

                // Carbs should never be negative
                EXPECT_GE(m.target_carbs_g, 0.0);

                // All values should be rounded (no fractional grams)
                EXPECT_DOUBLE_EQ(m.target_protein_g, std::round(m.target_protein_g));
                EXPECT_DOUBLE_EQ(m.target_fat_g, std::round(m.target_fat_g));
                EXPECT_DOUBLE_EQ(m.target_carbs_g, std::round(m.target_carbs_g));
            }
        }
    }
}

// ── TDEE stored correctly ────────────────────────────────────────────

TEST(MacroCalculationUnit, TDEERoundedAndStored) {
    auto m = calculate_macros(2759.123, 80.0, "maintain");
    EXPECT_DOUBLE_EQ(m.tdee_calories, 2759.0); // std::round(2759.123)
}
