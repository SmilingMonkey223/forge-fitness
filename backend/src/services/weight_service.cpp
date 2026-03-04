#include "../../include/weight_service.hpp"
#include "../../include/uuid.hpp"
#include <cmath>
#include <stdexcept>

namespace forge {

WeightEntry WeightService::log_weight(
    const std::string& user_id,
    double weight_kg,
    const std::optional<std::string>& notes)
{
    if (weight_kg <= 0 || weight_kg >= 500) {
        throw std::invalid_argument("INVALID_WEIGHT");
    }

    std::string id = UUID::generate();

    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Insert weight log entry
    pqxx::result result;
    if (notes.has_value()) {
        result = txn.exec_params(
            "INSERT INTO weight_logs (id, user_id, weight_kg, logged_at, notes) "
            "VALUES ($1, $2, $3, CURRENT_TIMESTAMP, $4) "
            "RETURNING id, TO_CHAR(logged_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS logged_at_str",
            id, user_id, weight_kg, notes.value()
        );
    } else {
        result = txn.exec_params(
            "INSERT INTO weight_logs (id, user_id, weight_kg, logged_at) "
            "VALUES ($1, $2, $3, CURRENT_TIMESTAMP) "
            "RETURNING id, TO_CHAR(logged_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS logged_at_str",
            id, user_id, weight_kg
        );
    }

    // Also update user_profiles.weight_kg to keep it current
    txn.exec_params(
        "UPDATE user_profiles SET weight_kg = $1 WHERE user_id = $2",
        weight_kg, user_id
    );

    txn.commit();

    WeightEntry entry;
    entry.id = id;
    entry.user_id = user_id;
    entry.weight_kg = weight_kg;
    entry.logged_at = result[0]["logged_at_str"].c_str();
    entry.notes = notes;

    return entry;
}

std::vector<WeightEntry> WeightService::get_weight_history(
    const std::string& user_id,
    int limit)
{
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT id, user_id, weight_kg, "
        "TO_CHAR(logged_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS logged_at_str, "
        "notes "
        "FROM weight_logs "
        "WHERE user_id = $1 "
        "ORDER BY logged_at DESC "
        "LIMIT $2",
        user_id, limit
    );

    txn.commit();

    std::vector<WeightEntry> entries;
    entries.reserve(result.size());

    for (const auto& row : result) {
        WeightEntry entry;
        entry.id = row["id"].c_str();
        entry.user_id = row["user_id"].c_str();
        entry.weight_kg = row["weight_kg"].as<double>();
        entry.logged_at = row["logged_at_str"].c_str();
        if (!row["notes"].is_null() && std::string(row["notes"].c_str()) != "") {
            entry.notes = row["notes"].c_str();
        }
        entries.push_back(std::move(entry));
    }

    return entries;
}

WeightTrend WeightService::get_weight_trend(const std::string& user_id)
{
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Get all entries from the last 30 days for trend calculation
    auto result = txn.exec_params(
        "SELECT weight_kg, "
        "TO_CHAR(logged_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS logged_at_str "
        "FROM weight_logs "
        "WHERE user_id = $1 "
        "AND logged_at >= CURRENT_TIMESTAMP - INTERVAL '30 days' "
        "ORDER BY logged_at ASC",
        user_id
    );

    txn.commit();

    WeightTrend trend;
    trend.data_points = static_cast<int>(result.size());

    if (result.empty()) {
        trend.current_weight = 0;
        trend.start_weight = 0;
        trend.change_kg = 0;
        trend.change_percent = 0;
        trend.avg_weekly_change = 0;
        trend.trend_direction = "stable";
        return trend;
    }

    trend.start_weight = result[0]["weight_kg"].as<double>();
    trend.current_weight = result[result.size() - 1]["weight_kg"].as<double>();
    trend.change_kg = std::round((trend.current_weight - trend.start_weight) * 100.0) / 100.0;

    if (trend.start_weight > 0) {
        trend.change_percent = std::round((trend.change_kg / trend.start_weight) * 10000.0) / 100.0;
    } else {
        trend.change_percent = 0;
    }

    // Average weekly change over the 30-day period
    // Total change divided by ~4.3 weeks
    double weeks = 30.0 / 7.0;
    trend.avg_weekly_change = std::round((trend.change_kg / weeks) * 100.0) / 100.0;

    // Determine trend direction
    if (trend.avg_weekly_change < -0.1) {
        trend.trend_direction = "losing";
    } else if (trend.avg_weekly_change > 0.1) {
        trend.trend_direction = "gaining";
    } else {
        trend.trend_direction = "stable";
    }

    return trend;
}

bool WeightService::delete_weight(
    const std::string& weight_id,
    const std::string& user_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "DELETE FROM weight_logs WHERE id = $1 AND user_id = $2",
        weight_id, user_id
    );

    txn.commit();
    return result.affected_rows() > 0;
}

} // namespace forge
