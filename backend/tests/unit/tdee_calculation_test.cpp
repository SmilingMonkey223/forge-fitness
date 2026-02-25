#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <map>

// Test the Mifflin-St Jeor TDEE calculation formula.
// This mirrors the logic in ProfileService::calculate_bmr,
// ProfileService::get_activity_multiplier, and ProfileService::calculate_tdee.

namespace {

// Activity multipliers (must match ProfileService::get_activity_multiplier)
double get_activity_multiplier(const std::string& level) {
    if (level == "sedentary") return 1.2;
    if (level == "lightly_active") return 1.375;
    if (level == "moderately_active") return 1.55;
    if (level == "very_active") return 1.725;
    if (level == "extremely_active") return 1.9;
    return 0.0; // Invalid
}

// Mifflin-St Jeor formula (must match ProfileService::calculate_bmr)
// Female: BMR = 10*weight + 6.25*height - 5*age - 161
// Male:   BMR = 10*weight + 6.25*height - 5*age - 161 + 166 = ... + 5
// Other:  BMR = 10*weight + 6.25*height - 5*age - 161 + 83  (midpoint)
double calculate_bmr(const std::string& sex, double weight_kg,
                     double height_cm, int age) {
    double bmr = 10.0 * weight_kg + 6.25 * height_cm - 5.0 * age - 161.0;
    if (sex == "male") bmr += 166.0;
    if (sex == "other") bmr += 83.0;
    return bmr;
}

double calculate_tdee(const std::string& sex, double weight_kg,
                      double height_cm, int age,
                      const std::string& activity_level) {
    return calculate_bmr(sex, weight_kg, height_cm, age)
         * get_activity_multiplier(activity_level);
}

} // anonymous namespace

// ── Male calculations ───────────────────────────────────────────────

TEST(TDEECalculationUnit, MaleSedentary) {
    // Male, 80kg, 180cm, age 30, sedentary
    // BMR = 10*80 + 6.25*180 - 5*30 - 161 + 166 = 800+1125-150-161+166 = 1780
    // TDEE = 1780 * 1.2 = 2136
    double result = calculate_tdee("male", 80.0, 180.0, 30, "sedentary");
    EXPECT_NEAR(result, 2136.0, 1.0);
}

TEST(TDEECalculationUnit, MaleModeratelyActive) {
    // BMR = 1780, TDEE = 1780 * 1.55 = 2759
    double result = calculate_tdee("male", 80.0, 180.0, 30, "moderately_active");
    EXPECT_NEAR(result, 2759.0, 1.0);
}

TEST(TDEECalculationUnit, MaleLightlyActive) {
    // BMR = 1780, TDEE = 1780 * 1.375 = 2447.5
    double result = calculate_tdee("male", 80.0, 180.0, 30, "lightly_active");
    EXPECT_NEAR(result, 2447.5, 1.0);
}

TEST(TDEECalculationUnit, MaleVeryActive) {
    // BMR = 1780, TDEE = 1780 * 1.725 = 3070.5
    double result = calculate_tdee("male", 80.0, 180.0, 30, "very_active");
    EXPECT_NEAR(result, 3070.5, 1.0);
}

TEST(TDEECalculationUnit, MaleExtremelyActive) {
    // BMR = 1780, TDEE = 1780 * 1.9 = 3382
    double result = calculate_tdee("male", 80.0, 180.0, 30, "extremely_active");
    EXPECT_NEAR(result, 3382.0, 1.0);
}

// ── Female calculations ─────────────────────────────────────────────

TEST(TDEECalculationUnit, FemaleLightlyActive) {
    // Female, 60kg, 165cm, age 25, lightly_active
    // BMR = 10*60 + 6.25*165 - 5*25 - 161 = 600+1031.25-125-161 = 1345.25
    // TDEE = 1345.25 * 1.375 = 1849.71875
    double expected_bmr = 10.0*60.0 + 6.25*165.0 - 5.0*25.0 - 161.0;
    double expected_tdee = expected_bmr * 1.375;
    double result = calculate_tdee("female", 60.0, 165.0, 25, "lightly_active");
    EXPECT_NEAR(result, expected_tdee, 0.5);
    EXPECT_NEAR(expected_bmr, 1345.25, 0.01);
}

TEST(TDEECalculationUnit, FemaleSedentary) {
    // BMR = 1345.25, TDEE = 1345.25 * 1.2 = 1614.3
    double result = calculate_tdee("female", 60.0, 165.0, 25, "sedentary");
    EXPECT_NEAR(result, 1614.3, 1.0);
}

// ── "Other" sex uses midpoint ───────────────────────────────────────

TEST(TDEECalculationUnit, OtherSexMidpointBMR) {
    // Other, 80kg, 180cm, age 30
    // BMR = 10*80 + 6.25*180 - 5*30 - 161 + 83 = 800+1125-150-161+83 = 1697
    double bmr = calculate_bmr("other", 80.0, 180.0, 30);
    EXPECT_NEAR(bmr, 1697.0, 0.01);

    // Should be midpoint between male (1780) and female (1614)
    double male_bmr = calculate_bmr("male", 80.0, 180.0, 30);
    double female_bmr = calculate_bmr("female", 80.0, 180.0, 30);
    EXPECT_NEAR(bmr, (male_bmr + female_bmr) / 2.0, 0.01);
}

// ── Edge cases ──────────────────────────────────────────────────────

TEST(TDEECalculationUnit, VeryHeavyPerson) {
    // 150kg, 200cm, age 20, extremely_active
    // BMR = 10*150 + 6.25*200 - 5*20 - 161 + 166 = 1500+1250-100-161+166 = 2655
    // TDEE = 2655 * 1.9 = 5044.5
    double result = calculate_tdee("male", 150.0, 200.0, 20, "extremely_active");
    EXPECT_NEAR(result, 5044.5, 1.0);
}

TEST(TDEECalculationUnit, MinimalValuesMale) {
    // 40kg, 140cm, age 65, sedentary
    // BMR = 10*40 + 6.25*140 - 5*65 - 161 + 166 = 400+875-325-161+166 = 955
    // TDEE = 955 * 1.2 = 1146
    double result = calculate_tdee("male", 40.0, 140.0, 65, "sedentary");
    EXPECT_NEAR(result, 1146.0, 1.0);
}

TEST(TDEECalculationUnit, MinimalFemale) {
    // 40kg, 140cm, age 65, sedentary, female
    // BMR = 10*40 + 6.25*140 - 5*65 - 161 = 400+875-325-161 = 789
    // TDEE = 789 * 1.2 = 946.8
    double result = calculate_tdee("female", 40.0, 140.0, 65, "sedentary");
    EXPECT_NEAR(result, 946.8, 1.0);
}

// ── Activity multiplier verification ────────────────────────────────

TEST(TDEECalculationUnit, ActivityMultiplierSedentary) {
    EXPECT_DOUBLE_EQ(get_activity_multiplier("sedentary"), 1.2);
}

TEST(TDEECalculationUnit, ActivityMultiplierLightlyActive) {
    EXPECT_DOUBLE_EQ(get_activity_multiplier("lightly_active"), 1.375);
}

TEST(TDEECalculationUnit, ActivityMultiplierModeratelyActive) {
    EXPECT_DOUBLE_EQ(get_activity_multiplier("moderately_active"), 1.55);
}

TEST(TDEECalculationUnit, ActivityMultiplierVeryActive) {
    EXPECT_DOUBLE_EQ(get_activity_multiplier("very_active"), 1.725);
}

TEST(TDEECalculationUnit, ActivityMultiplierExtremelyActive) {
    EXPECT_DOUBLE_EQ(get_activity_multiplier("extremely_active"), 1.9);
}

TEST(TDEECalculationUnit, ActivityMultiplierInvalidReturnsZero) {
    EXPECT_DOUBLE_EQ(get_activity_multiplier("unknown_level"), 0.0);
}
