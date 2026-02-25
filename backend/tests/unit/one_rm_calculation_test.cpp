#include <gtest/gtest.h>
#include <cmath>

// Test the Epley formula for estimated 1RM:
//   1RM = weight × (1 + reps / 30)
//
// This mirrors WorkoutService::calculate_estimated_1rm from workout_service.cpp.
// Edge cases from the actual code:
//   - reps <= 0 OR weight <= 0 → returns 0.0
//   - reps == 1 → returns weight_kg (actual 1RM, no estimation needed)

namespace {

// Epley formula matching WorkoutService::calculate_estimated_1rm
double calculate_estimated_1rm(double weight_kg, int reps) {
    if (reps <= 0 || weight_kg <= 0.0) return 0.0;
    if (reps == 1) return weight_kg;
    return weight_kg * (1.0 + static_cast<double>(reps) / 30.0);
}

// Check if current set is a PR compared to previous best 1RM
bool is_pr(double weight_kg, int reps, double previous_best_1rm) {
    return calculate_estimated_1rm(weight_kg, reps) > previous_best_1rm;
}

} // anonymous namespace

// ── Standard calculations ───────────────────────────────────────────

TEST(OneRMCalculationUnit, HundredKgOneRep) {
    // 100kg × 1 rep → returns weight directly = 100.0
    double result = calculate_estimated_1rm(100.0, 1);
    EXPECT_DOUBLE_EQ(result, 100.0);
}

TEST(OneRMCalculationUnit, HundredKgFiveReps) {
    // 100kg × 5 reps → 1RM = 100 × (1 + 5/30) = 116.67
    double result = calculate_estimated_1rm(100.0, 5);
    EXPECT_NEAR(result, 116.67, 0.01);
}

TEST(OneRMCalculationUnit, HundredKgTenReps) {
    // 100kg × 10 reps → 1RM = 100 × (1 + 10/30) = 133.33
    double result = calculate_estimated_1rm(100.0, 10);
    EXPECT_NEAR(result, 133.33, 0.01);
}

TEST(OneRMCalculationUnit, SixtyKgTwelveReps) {
    // 60kg × 12 reps → 1RM = 60 × (1 + 12/30) = 84.0
    double result = calculate_estimated_1rm(60.0, 12);
    EXPECT_NEAR(result, 84.0, 0.01);
}

// ── Edge cases (matching WorkoutService behavior) ───────────────────

TEST(OneRMCalculationUnit, ZeroRepsReturnsZero) {
    double result = calculate_estimated_1rm(100.0, 0);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST(OneRMCalculationUnit, NegativeRepsReturnsZero) {
    double result = calculate_estimated_1rm(100.0, -5);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST(OneRMCalculationUnit, ZeroWeightReturnsZero) {
    double result = calculate_estimated_1rm(0.0, 10);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST(OneRMCalculationUnit, NegativeWeightReturnsZero) {
    double result = calculate_estimated_1rm(-50.0, 5);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST(OneRMCalculationUnit, OneRepReturnsExactWeight) {
    EXPECT_DOUBLE_EQ(calculate_estimated_1rm(100.0, 1), 100.0);
    EXPECT_DOUBLE_EQ(calculate_estimated_1rm(200.0, 1), 200.0);
    EXPECT_DOUBLE_EQ(calculate_estimated_1rm(60.5, 1), 60.5);
}

TEST(OneRMCalculationUnit, HighRepsEstimate) {
    // 50kg × 20 reps → 1RM = 50 × (1 + 20/30) = 83.33
    double result = calculate_estimated_1rm(50.0, 20);
    EXPECT_NEAR(result, 83.33, 0.01);
}

TEST(OneRMCalculationUnit, VeryLightWeight) {
    // 20kg × 15 reps → 1RM = 20 × 1.5 = 30.0
    double result = calculate_estimated_1rm(20.0, 15);
    EXPECT_DOUBLE_EQ(result, 30.0);
}

TEST(OneRMCalculationUnit, VeryHeavyWeight) {
    // 250kg × 3 reps → 1RM = 250 × 1.1 = 275.0
    double result = calculate_estimated_1rm(250.0, 3);
    EXPECT_DOUBLE_EQ(result, 275.0);
}

// ── Higher reps give higher 1RM estimate for same weight ────────────

TEST(OneRMCalculationUnit, HigherRepsHigherEstimate) {
    double rm5 = calculate_estimated_1rm(100.0, 5);
    double rm10 = calculate_estimated_1rm(100.0, 10);
    double rm15 = calculate_estimated_1rm(100.0, 15);

    EXPECT_LT(rm5, rm10);
    EXPECT_LT(rm10, rm15);
}

// ── PR detection ────────────────────────────────────────────────────

TEST(OneRMCalculationUnit, PRDetectedWhenNewBest) {
    double previous_best = calculate_estimated_1rm(100.0, 5);
    EXPECT_TRUE(is_pr(105.0, 5, previous_best));
}

TEST(OneRMCalculationUnit, NoPRWhenLowerThanBest) {
    double previous_best = calculate_estimated_1rm(100.0, 5);
    EXPECT_FALSE(is_pr(90.0, 5, previous_best));
}

TEST(OneRMCalculationUnit, PRViaMoreReps) {
    double previous_best = calculate_estimated_1rm(100.0, 5);
    EXPECT_TRUE(is_pr(100.0, 8, previous_best));
}

TEST(OneRMCalculationUnit, PRExactlyEqualIsNotPR) {
    double previous_best = calculate_estimated_1rm(100.0, 5);
    EXPECT_FALSE(is_pr(100.0, 5, previous_best));
}

TEST(OneRMCalculationUnit, PRFirstEverSet) {
    EXPECT_TRUE(is_pr(40.0, 10, 0.0));
}

// ── Monotonicity: weight increases → 1RM increases ─────────────────

TEST(OneRMCalculationUnit, MonotonicWithWeight) {
    for (int w = 20; w <= 200; w += 10) {
        double rm_current = calculate_estimated_1rm(static_cast<double>(w), 5);
        double rm_next = calculate_estimated_1rm(static_cast<double>(w + 10), 5);
        EXPECT_LT(rm_current, rm_next);
    }
}

// ── Monotonicity: reps increase → 1RM increases ────────────────────

TEST(OneRMCalculationUnit, MonotonicWithReps) {
    for (int r = 2; r <= 20; ++r) {
        double rm_current = calculate_estimated_1rm(100.0, r);
        double rm_next = calculate_estimated_1rm(100.0, r + 1);
        EXPECT_LT(rm_current, rm_next);
    }
}
