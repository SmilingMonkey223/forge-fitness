#include "warmup_planner.hpp"
#include <cmath>

namespace forge {

nlohmann::json WarmupSet::to_json() const {
    return {
        {"weight", weight},
        {"reps", reps},
        {"percentage", percentage_of_working_weight * 100},
        {"description", description}
    };
}

double WarmupPlanner::round_to_nearest(double value, double increment) {
    return std::round(value / increment) * increment;
}

std::vector<WarmupSet> WarmupPlanner::generate_warmup(
    double working_weight,
    int working_reps,
    double bar_weight
) {
    // Use different protocols based on rep range
    if (working_reps <= 5) {
        // Heavy strength work - more warm-up sets
        return powerlifting_warmup(working_weight, bar_weight);
    } else if (working_reps <= 12) {
        // Hypertrophy range - moderate warm-up
        return bodybuilding_warmup(working_weight, bar_weight);
    } else {
        // High rep work - minimal warm-up
        return beginner_warmup(working_weight, bar_weight);
    }
}

std::vector<WarmupSet> WarmupPlanner::powerlifting_warmup(
    double working_weight,
    double bar_weight
) {
    std::vector<WarmupSet> warmup;

    // If working weight is light (< 135 lbs), use simpler protocol
    if (working_weight < 135.0) {
        return beginner_warmup(working_weight, bar_weight);
    }

    // Set 1: Empty bar (if working weight is heavy enough)
    if (working_weight > bar_weight * 2) {
        warmup.push_back({
            bar_weight,
            10,
            bar_weight / working_weight,
            "Empty bar - activate muscles"
        });
    }

    // Set 2: 50% of working weight
    double weight_50 = round_to_nearest(working_weight * 0.50);
    warmup.push_back({
        weight_50,
        5,
        0.50,
        "50% - groove movement pattern"
    });

    // Set 3: 70% of working weight
    double weight_70 = round_to_nearest(working_weight * 0.70);
    warmup.push_back({
        weight_70,
        3,
        0.70,
        "70% - build up tension"
    });

    // Set 4: 85% of working weight
    double weight_85 = round_to_nearest(working_weight * 0.85);
    warmup.push_back({
        weight_85,
        2,
        0.85,
        "85% - final prep"
    });

    // Set 5 (optional): 95% of working weight for very heavy lifts
    if (working_weight > 300.0) {
        double weight_95 = round_to_nearest(working_weight * 0.95);
        warmup.push_back({
            weight_95,
            1,
            0.95,
            "95% - prime nervous system"
        });
    }

    return warmup;
}

std::vector<WarmupSet> WarmupPlanner::bodybuilding_warmup(
    double working_weight,
    double bar_weight
) {
    std::vector<WarmupSet> warmup;

    // Set 1: 40% of working weight
    double weight_40 = round_to_nearest(working_weight * 0.40);
    // Don't go below bar weight
    if (weight_40 < bar_weight) weight_40 = bar_weight;

    warmup.push_back({
        weight_40,
        12,
        weight_40 / working_weight,
        "40% - pump blood into muscles"
    });

    // Set 2: 60% of working weight
    double weight_60 = round_to_nearest(working_weight * 0.60);
    warmup.push_back({
        weight_60,
        8,
        0.60,
        "60% - groove movement"
    });

    // Set 3: 80% of working weight (only if working weight is substantial)
    if (working_weight > 100.0) {
        double weight_80 = round_to_nearest(working_weight * 0.80);
        warmup.push_back({
            weight_80,
            4,
            0.80,
            "80% - final warm-up"
        });
    }

    return warmup;
}

std::vector<WarmupSet> WarmupPlanner::beginner_warmup(
    double working_weight,
    double bar_weight
) {
    std::vector<WarmupSet> warmup;

    // Simple 2-set warmup for beginners or light weights

    // Set 1: 50% of working weight or bar (whichever is higher)
    double weight_50 = round_to_nearest(working_weight * 0.50);
    if (weight_50 < bar_weight) weight_50 = bar_weight;

    warmup.push_back({
        weight_50,
        8,
        weight_50 / working_weight,
        "50% - prepare muscles"
    });

    // Set 2: 75% of working weight (only if working weight > bar weight)
    if (working_weight > bar_weight * 1.5) {
        double weight_75 = round_to_nearest(working_weight * 0.75);
        warmup.push_back({
            weight_75,
            5,
            0.75,
            "75% - final prep"
        });
    }

    return warmup;
}

} // namespace forge
