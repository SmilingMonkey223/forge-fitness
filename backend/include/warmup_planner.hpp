#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace forge {

struct WarmupSet {
    double weight;
    int reps;
    double percentage_of_working_weight; // e.g., 0.50 = 50%
    std::string description;

    nlohmann::json to_json() const;
};

class WarmupPlanner {
public:
    // Generate warm-up sets based on working weight
    static std::vector<WarmupSet> generate_warmup(
        double working_weight,
        int working_reps,
        double bar_weight = 45.0
    );

    // Different warmup protocols
    static std::vector<WarmupSet> powerlifting_warmup(
        double working_weight,
        double bar_weight = 45.0
    );

    static std::vector<WarmupSet> bodybuilding_warmup(
        double working_weight,
        double bar_weight = 45.0
    );

    static std::vector<WarmupSet> beginner_warmup(
        double working_weight,
        double bar_weight = 45.0
    );

private:
    static double round_to_nearest(double value, double increment = 5.0);
};

} // namespace forge
