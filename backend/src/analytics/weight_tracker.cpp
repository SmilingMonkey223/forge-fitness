#include "weight_tracker.hpp"
#include <algorithm>
#include <cmath>

namespace forge {

WeightTracker::WeightTracker(std::shared_ptr<Database> db) : db_(db) {}

std::optional<WeightEntry> WeightTracker::log_weight(
    const std::string& user_id,
    const Date& date,
    float weight_kg,
    const std::string& source
) {
    // Validate weight range
    if (weight_kg < 30.0f || weight_kg > 300.0f) {
        return std::nullopt;
    }

    auto conn = db_->get_connection();
    pqxx::work txn(*conn);

    try {
        // Convert date to string
        auto date_str = std::format("{:%Y-%m-%d}", date);

        // Insert or update
        auto result = txn.exec_params(
            "INSERT INTO weight_entries (user_id, date, weight_kg, source) "
            "VALUES ($1, $2, $3, $4) "
            "ON CONFLICT (user_id, date) DO UPDATE "
            "SET weight_kg = EXCLUDED.weight_kg, source = EXCLUDED.source "
            "RETURNING id, created_at",
            user_id, date_str, weight_kg, source
        );

        txn.commit();

        if (result.empty()) {
            return std::nullopt;
        }

        WeightEntry entry{};
        entry.id = result[0]["id"].as<std::string>();
        entry.user_id = user_id;
        entry.date = date;
        entry.weight_kg = weight_kg;
        entry.source = source;
        // Parse created_at timestamp
        // entry.created_at = ...

        return entry;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::optional<WeightEntry> WeightTracker::get_weight(
    const std::string& user_id,
    const Date& date
) {
    auto conn = db_->get_connection();
    pqxx::work txn(*conn);

    auto date_str = std::format("{:%Y-%m-%d}", date);

    auto result = txn.exec_params(
        "SELECT id, weight_kg, source, created_at "
        "FROM weight_entries "
        "WHERE user_id = $1 AND date = $2",
        user_id, date_str
    );

    if (result.empty()) {
        return std::nullopt;
    }

    WeightEntry entry{};
    entry.id = result[0]["id"].as<std::string>();
    entry.user_id = user_id;
    entry.date = date;
    entry.weight_kg = result[0]["weight_kg"].as<float>();
    entry.source = result[0]["source"].as<std::string>();

    return entry;
}

std::vector<WeightData> WeightTracker::get_weight_range(
    const std::string& user_id,
    const Date& start,
    const Date& end
) {
    auto conn = db_->get_connection();
    pqxx::work txn(*conn);

    auto start_str = std::format("{:%Y-%m-%d}", start);
    auto end_str = std::format("{:%Y-%m-%d}", end);

    auto result = txn.exec_params(
        "SELECT date, weight_kg "
        "FROM weight_entries "
        "WHERE user_id = $1 AND date >= $2 AND date <= $3 "
        "ORDER BY date ASC",
        user_id, start_str, end_str
    );

    std::vector<WeightEntry> entries;
    for (const auto& row : result) {
        WeightEntry entry{};
        entry.user_id = user_id;
        // Parse date from string
        // entry.date = ...
        entry.weight_kg = row["weight_kg"].as<float>();
        entries.push_back(entry);
    }

    // Compute trend weights
    return compute_trend_weights(entries);
}

bool WeightTracker::delete_weight(const std::string& user_id, const Date& date) {
    auto conn = db_->get_connection();
    pqxx::work txn(*conn);

    auto date_str = std::format("{:%Y-%m-%d}", date);

    auto result = txn.exec_params(
        "DELETE FROM weight_entries WHERE user_id = $1 AND date = $2",
        user_id, date_str
    );

    txn.commit();
    return result.affected_rows() > 0;
}

std::optional<WeightTrend> WeightTracker::get_trend(const std::string& user_id) {
    // Get last 90 days of data
    auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    auto ninety_days_ago = today - std::chrono::days(90);

    auto trend_data = get_weight_range(user_id, ninety_days_ago, today);

    if (trend_data.size() < 3) {
        // Not enough data for reliable trend
        WeightTrend result{};
        result.is_reliable = false;
        result.total_weigh_ins = trend_data.size();
        return result;
    }

    // Get most recent trend weight
    float current_trend = trend_data.back().trend_weight_kg.value_or(
        trend_data.back().scale_weight_kg
    );

    // Calculate weekly rate
    auto weekly_rate = calculate_rate(trend_data, 7);
    auto monthly_rate = calculate_rate(trend_data, 30);

    // TODO: Get goal weight and starting weight from user profile
    float goal_weight_kg = 75.0f;  // Placeholder
    float starting_weight_kg = 85.0f;  // Placeholder

    WeightTrend result{};
    result.current_trend_weight_kg = current_trend;
    result.weekly_rate_kg = weekly_rate.value_or(0.0f);
    result.monthly_rate_kg = monthly_rate.value_or(0.0f);
    result.goal_weight_kg = goal_weight_kg;
    result.starting_weight_kg = starting_weight_kg;

    // Calculate progress
    float total_change_needed = starting_weight_kg - goal_weight_kg;
    float change_so_far = starting_weight_kg - current_trend;
    result.progress_percent = (change_so_far / total_change_needed) * 100.0f;

    // Estimate days to goal
    if (std::abs(result.weekly_rate_kg) > 0.01f) {
        float remaining_weight = current_trend - goal_weight_kg;
        float weeks_to_goal = remaining_weight / result.weekly_rate_kg;
        result.days_to_goal = static_cast<int>(weeks_to_goal * 7);
    }

    result.is_reliable = true;
    result.total_weigh_ins = trend_data.size();

    return result;
}

std::vector<WeightData> WeightTracker::compute_trend_weights(
    const std::vector<WeightEntry>& entries,
    float alpha
) {
    if (entries.empty()) {
        return {};
    }

    // First, interpolate missing days
    auto interpolated = interpolate_missing_days(entries);

    // Apply EWMA
    std::vector<WeightData> result;
    float trend = interpolated[0].scale_weight_kg;  // Initial trend = first weight

    for (const auto& data : interpolated) {
        // EWMA formula: trend[t] = α × weight[t] + (1 - α) × trend[t-1]
        trend = alpha * data.scale_weight_kg + (1 - alpha) * trend;

        WeightData point{};
        point.date = data.date;
        point.scale_weight_kg = data.scale_weight_kg;
        point.trend_weight_kg = trend;

        result.push_back(point);
    }

    return result;
}

std::vector<WeightData> WeightTracker::interpolate_missing_days(
    const std::vector<WeightEntry>& entries
) {
    if (entries.empty()) {
        return {};
    }

    std::vector<WeightData> result;

    for (size_t i = 0; i < entries.size(); ++i) {
        WeightData point{};
        point.date = entries[i].date;
        point.scale_weight_kg = entries[i].weight_kg;
        result.push_back(point);

        // If there's a gap, interpolate
        if (i + 1 < entries.size()) {
            auto days_gap = (entries[i + 1].date - entries[i].date).count();
            if (days_gap > 1) {
                float weight_start = entries[i].weight_kg;
                float weight_end = entries[i + 1].weight_kg;
                float weight_per_day = (weight_end - weight_start) / days_gap;

                for (int d = 1; d < days_gap; ++d) {
                    WeightData interpolated{};
                    interpolated.date = entries[i].date + std::chrono::days(d);
                    interpolated.scale_weight_kg = weight_start + weight_per_day * d;
                    result.push_back(interpolated);
                }
            }
        }
    }

    return result;
}

std::optional<float> WeightTracker::calculate_rate(
    const std::vector<WeightData>& trend_data,
    int days_lookback
) {
    if (trend_data.size() < 2) {
        return std::nullopt;
    }

    // Find data point from days_lookback ago
    auto target_date = trend_data.back().date - std::chrono::days(days_lookback);

    // Find closest data point to target date
    auto it = std::lower_bound(
        trend_data.begin(),
        trend_data.end(),
        target_date,
        [](const WeightData& a, const Date& b) { return a.date < b; }
    );

    if (it == trend_data.end() || it == trend_data.begin()) {
        return std::nullopt;
    }

    float old_trend = it->trend_weight_kg.value_or(it->scale_weight_kg);
    float new_trend = trend_data.back().trend_weight_kg.value_or(
        trend_data.back().scale_weight_kg
    );

    auto actual_days = (trend_data.back().date - it->date).count();
    if (actual_days == 0) {
        return std::nullopt;
    }

    // Return rate per week
    float rate_per_day = (new_trend - old_trend) / actual_days;
    return rate_per_day * 7.0f;  // Convert to per week
}

} // namespace forge
