#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace forge {

struct WeightEntry {
    std::string id;
    std::string user_id;
    double weight_kg;
    std::string logged_at;
    std::optional<std::string> notes;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"weight_kg", weight_kg},
            {"logged_at", logged_at}
        };
        if (notes) j["notes"] = *notes;
        return j;
    }
};

struct WeightTrend {
    double current_weight;
    double start_weight;
    double change_kg;
    double change_percent;
    double avg_weekly_change;
    std::string trend_direction; // "losing" | "gaining" | "stable"
    int data_points;

    nlohmann::json to_json() const {
        return {
            {"current_weight", current_weight},
            {"start_weight", start_weight},
            {"change_kg", change_kg},
            {"change_percent", change_percent},
            {"avg_weekly_change", avg_weekly_change},
            {"trend_direction", trend_direction},
            {"data_points", data_points}
        };
    }
};

class WeightService {
public:
    explicit WeightService(Database& db) : db_(db) {}

    // Log a weight entry; also updates user_profiles.weight_kg
    WeightEntry log_weight(
        const std::string& user_id,
        double weight_kg,
        const std::optional<std::string>& notes = std::nullopt
    );

    // Get weight history ordered by logged_at DESC
    std::vector<WeightEntry> get_weight_history(
        const std::string& user_id,
        int limit = 90
    );

    // Calculate trend from weight history
    WeightTrend get_weight_trend(const std::string& user_id);

private:
    Database& db_;
};

} // namespace forge
