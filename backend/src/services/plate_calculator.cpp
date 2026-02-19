#include "plate_calculator.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace forge {

nlohmann::json PlateCombination::to_json() const {
    nlohmann::json j;
    j["weight_per_side"] = weight_per_side;
    j["is_valid"] = is_valid;

    if (!is_valid) {
        j["error"] = error_message;
        return j;
    }

    nlohmann::json plates_json = nlohmann::json::array();
    for (const auto& [weight, count] : plates) {
        plates_json.push_back({
            {"weight", weight},
            {"count_per_side", count}
        });
    }
    j["plates"] = plates_json;

    return j;
}

PlateConfiguration PlateCalculator::standard_lbs() {
    return {
        45.0, // Standard Olympic barbell
        {45, 35, 25, 10, 5, 2.5, 1.25} // Imperial plates
    };
}

PlateConfiguration PlateCalculator::standard_kg() {
    return {
        20.0, // Standard Olympic barbell (20kg = 44lbs)
        {20, 15, 10, 5, 2.5, 1.25, 0.5} // Metric plates
    };
}

PlateConfiguration PlateCalculator::womens_lbs() {
    return {
        35.0, // Women's Olympic bar
        {45, 35, 25, 10, 5, 2.5, 1.25}
    };
}

PlateConfiguration PlateCalculator::womens_kg() {
    return {
        15.0, // Women's Olympic bar (15kg)
        {20, 15, 10, 5, 2.5, 1.25, 0.5}
    };
}

bool PlateCalculator::is_achievable(
    double target_weight,
    const PlateConfiguration& config
) {
    // Weight must be at least the bar weight
    if (target_weight < config.bar_weight) {
        return false;
    }

    double weight_needed = (target_weight - config.bar_weight) / 2.0;

    // Find smallest plate
    double min_plate = *std::min_element(
        config.available_plates.begin(),
        config.available_plates.end()
    );

    // Weight must be achievable with smallest plate increment
    double remainder = std::fmod(weight_needed, min_plate);
    return std::abs(remainder) < 0.01 || std::abs(remainder - min_plate) < 0.01;
}

std::vector<std::pair<double, int>> PlateCalculator::find_plate_combination(
    double weight_needed,
    const std::vector<double>& available_plates
) {
    std::vector<std::pair<double, int>> combination;

    // Sort plates in descending order
    std::vector<double> sorted_plates = available_plates;
    std::sort(sorted_plates.rbegin(), sorted_plates.rend());

    double remaining = weight_needed;

    for (double plate : sorted_plates) {
        int count = static_cast<int>(remaining / plate);
        if (count > 0) {
            combination.push_back({plate, count});
            remaining -= count * plate;
        }

        // Break if we've accounted for all weight (within tolerance)
        if (std::abs(remaining) < 0.01) {
            break;
        }
    }

    return combination;
}

PlateCombination PlateCalculator::calculate(
    double target_weight,
    const PlateConfiguration& config
) {
    PlateCombination result;

    // Check if target is less than bar weight
    if (target_weight < config.bar_weight) {
        result.is_valid = false;
        result.error_message = "Target weight is less than bar weight";
        return result;
    }

    // Calculate weight needed on each side
    double weight_per_side = (target_weight - config.bar_weight) / 2.0;
    result.weight_per_side = weight_per_side;

    // Check if achievable
    if (!is_achievable(target_weight, config)) {
        result.is_valid = false;
        std::ostringstream oss;
        oss << "Weight not achievable with available plates. ";
        oss << "Closest would be: ";

        // Find smallest plate
        double min_plate = *std::min_element(
            config.available_plates.begin(),
            config.available_plates.end()
        );

        // Round to nearest achievable weight
        int increments = static_cast<int>(std::round(weight_per_side / min_plate));
        double closest = config.bar_weight + (increments * min_plate * 2);

        oss << closest << " lbs/kg";
        result.error_message = oss.str();
        return result;
    }

    // Calculate plate combination
    result.plates = find_plate_combination(weight_per_side, config.available_plates);
    result.is_valid = true;

    return result;
}

} // namespace forge
