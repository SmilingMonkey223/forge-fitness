#pragma once

#include <vector>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

namespace forge {

struct PlateConfiguration {
    double bar_weight; // Standard 45lbs (20kg) or 35lbs women's bar
    std::vector<double> available_plates; // e.g., {45, 35, 25, 10, 5, 2.5, 1.25}
};

struct PlateCombination {
    double weight_per_side;
    std::vector<std::pair<double, int>> plates; // plate weight, count per side
    bool is_valid;
    std::string error_message;

    nlohmann::json to_json() const;
};

class PlateCalculator {
public:
    // Calculate plates needed to hit target weight
    static PlateCombination calculate(
        double target_weight,
        const PlateConfiguration& config
    );

    // Get standard plate configurations
    static PlateConfiguration standard_lbs(); // 45lb bar, imperial plates
    static PlateConfiguration standard_kg();  // 20kg bar, metric plates
    static PlateConfiguration womens_lbs();   // 35lb bar
    static PlateConfiguration womens_kg();    // 15kg bar

    // Validate if target weight is achievable
    static bool is_achievable(
        double target_weight,
        const PlateConfiguration& config
    );

private:
    static std::vector<std::pair<double, int>> find_plate_combination(
        double weight_needed,
        const std::vector<double>& available_plates
    );
};

} // namespace forge
