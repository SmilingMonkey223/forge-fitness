#pragma once

#include "database.hpp"
#include <string>
#include <vector>
#include <optional>
#include <chrono>

namespace forge {

using Date = std::chrono::sys_days;

// Single weight entry
struct WeightEntry {
    std::string id;
    std::string user_id;
    Date date;
    float weight_kg;
    std::string source;  // manual, apple_health, google_fit
    std::chrono::system_clock::time_point created_at;
};

// Weight with trend calculation
struct WeightData {
    Date date;
    float scale_weight_kg;
    std::optional<float> trend_weight_kg;
};

// Trend analysis result
struct WeightTrend {
    float current_trend_weight_kg;
    float weekly_rate_kg;          // kg/week
    float monthly_rate_kg;          // kg/week (30-day smoothed)

    // Goal progress
    float goal_weight_kg;
    float starting_weight_kg;
    float progress_percent;
    std::optional<int> days_to_goal;  // Estimate based on current rate

    bool is_reliable;  // true if >= 3 weigh-ins
    int total_weigh_ins;
};

/**
 * Weight tracking and trend analysis
 * Implements EWMA (Exponentially Weighted Moving Average) for noise reduction
 */
class WeightTracker {
public:
    explicit WeightTracker(std::shared_ptr<Database> db);
    ~WeightTracker() = default;

    // CRUD operations
    std::optional<WeightEntry> log_weight(
        const std::string& user_id,
        const Date& date,
        float weight_kg,
        const std::string& source = "manual"
    );

    std::optional<WeightEntry> get_weight(const std::string& user_id, const Date& date);

    std::vector<WeightData> get_weight_range(
        const std::string& user_id,
        const Date& start,
        const Date& end
    );

    bool delete_weight(const std::string& user_id, const Date& date);

    // Trend calculation
    std::optional<WeightTrend> get_trend(const std::string& user_id);

    // Compute trend weights using EWMA
    std::vector<WeightData> compute_trend_weights(
        const std::vector<WeightEntry>& entries,
        float alpha = 0.1f  // Smoothing factor
    );

private:
    std::shared_ptr<Database> db_;

    // Linear interpolation for missing days
    std::vector<WeightData> interpolate_missing_days(const std::vector<WeightEntry>& entries);

    // Calculate rate of change
    std::optional<float> calculate_rate(
        const std::vector<WeightData>& trend_data,
        int days_lookback
    );
};

} // namespace forge
