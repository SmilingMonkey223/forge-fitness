#include "workout_service.hpp"
#include "uuid.hpp"
#include <sstream>
#include <cmath>
#include <iomanip>
#include <stdexcept>

namespace forge {

// ── JSON Serialization ────────────────────────────────────────────────

// Helper to format time_point as ISO 8601 string
static std::string format_iso8601(const std::chrono::system_clock::time_point& tp) {
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_val;
    gmtime_r(&time_t_val, &tm_val);
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

nlohmann::json Workout::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["user_id"] = user_id;
    if (name) j["name"] = *name;
    j["started_at"] = format_iso8601(started_at);
    if (completed_at) {
        j["completed_at"] = format_iso8601(*completed_at);
    } else {
        j["completed_at"] = nullptr;
    }
    if (duration_seconds) j["duration_seconds"] = *duration_seconds;
    if (notes) j["notes"] = *notes;
    j["status"] = status;

    nlohmann::json sets_json = nlohmann::json::array();
    for (const auto& s : sets) {
        sets_json.push_back(s.to_json());
    }
    j["sets"] = sets_json;

    // Calculate summary stats
    double total_volume = 0.0;
    int pr_count = 0;
    for (const auto& s : sets) {
        if (s.set_type == "working" && s.weight_kg && s.reps) {
            total_volume += (*s.weight_kg) * (*s.reps);
        }
        if (s.is_pr) ++pr_count;
    }
    j["total_volume_kg"] = total_volume;
    j["pr_count"] = pr_count;

    return j;
}

nlohmann::json AddSetResponse::to_json() const {
    nlohmann::json j = set.to_json();
    j["is_pr"] = is_pr;
    if (estimated_1rm) j["estimated_1rm"] = *estimated_1rm;
    if (previous_best_1rm) j["previous_best_1rm"] = *previous_best_1rm;
    return j;
}

nlohmann::json ExerciseHistoryEntry::to_json() const {
    nlohmann::json j;
    j["workout_id"] = workout_id;
    j["workout_date"] = workout_date;
    if (workout_name) j["workout_name"] = *workout_name;

    nlohmann::json sets_json = nlohmann::json::array();
    for (const auto& s : sets) {
        sets_json.push_back(s.to_json());
    }
    j["sets"] = sets_json;

    return j;
}

nlohmann::json WorkoutSummary::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["user_id"] = user_id;
    if (name) j["name"] = *name;
    j["started_at"] = started_at;
    if (completed_at) {
        j["completed_at"] = *completed_at;
    } else {
        j["completed_at"] = nullptr;
    }
    if (duration_seconds) j["duration_seconds"] = *duration_seconds;
    j["status"] = status;
    j["exercise_count"] = exercise_count;
    j["set_count"] = set_count;
    j["total_volume_kg"] = total_volume_kg;
    j["pr_count"] = pr_count;
    return j;
}

// ── Static Helpers ────────────────────────────────────────────────────

double WorkoutService::calculate_estimated_1rm(double weight_kg, int reps) {
    if (reps <= 0 || weight_kg <= 0.0) return 0.0;
    if (reps == 1) return weight_kg;
    // Epley formula: 1RM = weight * (1 + reps/30)
    return weight_kg * (1.0 + static_cast<double>(reps) / 30.0);
}

// ── Private Helpers ───────────────────────────────────────────────────

bool WorkoutService::verify_workout_ownership(
    pqxx::work& txn,
    const std::string& workout_id,
    const std::string& user_id
) {
    auto result = txn.exec_params(
        "SELECT id FROM workouts "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        workout_id, user_id
    );
    return !result.empty();
}

double WorkoutService::calculate_total_volume(
    pqxx::work& txn,
    const std::string& workout_id
) {
    auto result = txn.exec_params(
        "SELECT COALESCE(SUM(weight_kg * reps), 0) AS total_volume "
        "FROM exercise_sets "
        "WHERE workout_id = $1 AND set_type = 'working' "
        "AND weight_kg IS NOT NULL AND reps IS NOT NULL",
        workout_id
    );
    return result[0]["total_volume"].as<double>();
}

std::pair<int, int> WorkoutService::get_next_set_order(
    pqxx::work& txn,
    const std::string& workout_id,
    const std::string& exercise_id
) {
    // Check if this exercise already has sets in this workout
    auto existing = txn.exec_params(
        "SELECT exercise_order, MAX(set_order) AS max_set_order "
        "FROM exercise_sets "
        "WHERE workout_id = $1 AND exercise_id = $2 "
        "GROUP BY exercise_order",
        workout_id, exercise_id
    );

    if (!existing.empty()) {
        // Exercise already in workout - increment set_order
        int exercise_order = existing[0]["exercise_order"].as<int>();
        int next_set_order = existing[0]["max_set_order"].as<int>() + 1;
        return {next_set_order, exercise_order};
    }

    // New exercise in this workout - get next exercise_order
    auto max_exercise = txn.exec_params(
        "SELECT COALESCE(MAX(exercise_order), 0) AS max_eo "
        "FROM exercise_sets "
        "WHERE workout_id = $1",
        workout_id
    );

    int next_exercise_order = max_exercise[0]["max_eo"].as<int>() + 1;
    return {1, next_exercise_order};
}

// ── Workout CRUD ──────────────────────────────────────────────────────

std::string WorkoutService::create_workout(
    const std::string& user_id,
    const std::optional<std::string>& name,
    const std::optional<std::string>& notes
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    std::string workout_id = UUID::generate();

    if (name && notes) {
        txn.exec_params(
            "INSERT INTO workouts (id, user_id, name, notes, started_at, status) "
            "VALUES ($1, $2, $3, $4, NOW(), 'in_progress')",
            workout_id, user_id, *name, *notes
        );
    } else if (name) {
        txn.exec_params(
            "INSERT INTO workouts (id, user_id, name, started_at, status) "
            "VALUES ($1, $2, $3, NOW(), 'in_progress')",
            workout_id, user_id, *name
        );
    } else if (notes) {
        txn.exec_params(
            "INSERT INTO workouts (id, user_id, notes, started_at, status) "
            "VALUES ($1, $2, $3, NOW(), 'in_progress')",
            workout_id, user_id, *notes
        );
    } else {
        txn.exec_params(
            "INSERT INTO workouts (id, user_id, started_at, status) "
            "VALUES ($1, $2, NOW(), 'in_progress')",
            workout_id, user_id
        );
    }

    txn.commit();
    return workout_id;
}

std::vector<WorkoutSummary> WorkoutService::list_workouts(
    const std::string& user_id,
    int limit,
    int offset
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT w.id, w.user_id, w.name, "
        "       TO_CHAR(w.started_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS started_at, "
        "       TO_CHAR(w.completed_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS completed_at, "
        "       w.duration_seconds, w.status, "
        "       COUNT(DISTINCT es.exercise_id) AS exercise_count, "
        "       COUNT(es.id) AS set_count, "
        "       COALESCE(SUM(CASE WHEN es.set_type = 'working' AND es.weight_kg IS NOT NULL "
        "           AND es.reps IS NOT NULL THEN es.weight_kg * es.reps ELSE 0 END), 0) AS total_volume_kg, "
        "       COALESCE(SUM(CASE WHEN es.is_pr THEN 1 ELSE 0 END), 0) AS pr_count "
        "FROM workouts w "
        "LEFT JOIN exercise_sets es ON w.id = es.workout_id "
        "WHERE w.user_id = $1 AND w.deleted_at IS NULL "
        "GROUP BY w.id, w.user_id, w.name, w.started_at, w.completed_at, "
        "         w.duration_seconds, w.status "
        "ORDER BY w.started_at DESC "
        "LIMIT $2 OFFSET $3",
        user_id, limit, offset
    );

    std::vector<WorkoutSummary> workouts;
    for (const auto& row : result) {
        WorkoutSummary ws;
        ws.id = row["id"].as<std::string>();
        ws.user_id = row["user_id"].as<std::string>();
        if (!row["name"].is_null()) ws.name = row["name"].as<std::string>();
        ws.started_at = row["started_at"].as<std::string>();
        if (!row["completed_at"].is_null()) ws.completed_at = row["completed_at"].as<std::string>();
        if (!row["duration_seconds"].is_null()) ws.duration_seconds = row["duration_seconds"].as<int>();
        ws.status = row["status"].as<std::string>();
        ws.exercise_count = row["exercise_count"].as<int>();
        ws.set_count = row["set_count"].as<int>();
        ws.total_volume_kg = row["total_volume_kg"].as<double>();
        ws.pr_count = row["pr_count"].as<int>();
        workouts.push_back(ws);
    }

    return workouts;
}

std::optional<Workout> WorkoutService::get_workout(
    const std::string& workout_id,
    const std::string& user_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Get workout with epoch timestamps for easy parsing
    auto workout_result = txn.exec_params(
        "SELECT id, user_id, name, "
        "       EXTRACT(EPOCH FROM started_at)::BIGINT AS started_epoch, "
        "       EXTRACT(EPOCH FROM completed_at)::BIGINT AS completed_epoch, "
        "       duration_seconds, notes, status "
        "FROM workouts "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        workout_id, user_id
    );

    if (workout_result.empty()) {
        return std::nullopt;
    }

    auto row = workout_result[0];
    Workout workout;
    workout.id = row["id"].as<std::string>();
    workout.user_id = row["user_id"].as<std::string>();
    if (!row["name"].is_null()) workout.name = row["name"].as<std::string>();

    // Parse started_at from epoch
    auto started_epoch = row["started_epoch"].as<long>();
    workout.started_at = std::chrono::system_clock::from_time_t(started_epoch);

    if (!row["completed_epoch"].is_null()) {
        auto completed_epoch = row["completed_epoch"].as<long>();
        workout.completed_at = std::chrono::system_clock::from_time_t(completed_epoch);
    }
    if (!row["duration_seconds"].is_null()) {
        workout.duration_seconds = row["duration_seconds"].as<int>();
    }
    if (!row["notes"].is_null()) workout.notes = row["notes"].as<std::string>();
    workout.status = row["status"].as<std::string>();

    // Get all sets with exercise names
    auto sets_result = txn.exec_params(
        "SELECT es.id, es.workout_id, es.exercise_id, es.set_order, "
        "       es.exercise_order, es.set_type, es.reps, es.weight_kg, "
        "       es.duration_seconds, es.rpe, es.rest_seconds, es.is_pr, es.notes, "
        "       e.name AS exercise_name, e.muscle_group, e.equipment "
        "FROM exercise_sets es "
        "JOIN exercises e ON es.exercise_id = e.id "
        "WHERE es.workout_id = $1 "
        "ORDER BY es.exercise_order ASC, es.set_order ASC",
        workout_id
    );

    for (const auto& set_row : sets_result) {
        ExerciseSet es;
        es.id = set_row["id"].as<std::string>();
        es.workout_id = set_row["workout_id"].as<std::string>();
        es.exercise_id = set_row["exercise_id"].as<std::string>();
        es.set_order = set_row["set_order"].as<int>();
        es.exercise_order = set_row["exercise_order"].as<int>();
        es.set_type = set_row["set_type"].as<std::string>();
        if (!set_row["reps"].is_null()) es.reps = set_row["reps"].as<int>();
        if (!set_row["weight_kg"].is_null()) es.weight_kg = set_row["weight_kg"].as<double>();
        if (!set_row["duration_seconds"].is_null()) es.duration_seconds = set_row["duration_seconds"].as<int>();
        if (!set_row["rpe"].is_null()) es.rpe = set_row["rpe"].as<double>();
        if (!set_row["rest_seconds"].is_null()) es.rest_seconds = set_row["rest_seconds"].as<int>();
        es.is_pr = set_row["is_pr"].as<bool>();
        if (!set_row["notes"].is_null()) es.notes = set_row["notes"].as<std::string>();
        workout.sets.push_back(es);
    }

    return workout;
}

std::optional<Workout> WorkoutService::complete_workout(
    const std::string& workout_id,
    const std::string& user_id,
    const std::optional<std::string>& notes
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Verify ownership and that workout is in_progress
    auto check = txn.exec_params(
        "SELECT id, status FROM workouts "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        workout_id, user_id
    );

    if (check.empty()) {
        return std::nullopt;
    }

    std::string current_status = check[0]["status"].as<std::string>();
    if (current_status != "in_progress") {
        throw std::invalid_argument("WORKOUT_NOT_IN_PROGRESS");
    }

    // Calculate duration from started_at to now
    // Update workout: set completed_at, duration_seconds, status, notes
    if (notes) {
        txn.exec_params(
            "UPDATE workouts SET "
            "  completed_at = NOW(), "
            "  duration_seconds = EXTRACT(EPOCH FROM (NOW() - started_at))::INTEGER, "
            "  status = 'completed', "
            "  notes = $3 "
            "WHERE id = $1 AND user_id = $2",
            workout_id, user_id, *notes
        );
    } else {
        txn.exec_params(
            "UPDATE workouts SET "
            "  completed_at = NOW(), "
            "  duration_seconds = EXTRACT(EPOCH FROM (NOW() - started_at))::INTEGER, "
            "  status = 'completed' "
            "WHERE id = $1 AND user_id = $2",
            workout_id, user_id
        );
    }

    txn.commit();

    // Return the completed workout with all sets
    return get_workout(workout_id, user_id);
}

bool WorkoutService::delete_workout(
    const std::string& workout_id,
    const std::string& user_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "UPDATE workouts SET deleted_at = NOW() "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        workout_id, user_id
    );

    txn.commit();
    return result.affected_rows() > 0;
}

// ── Exercise Sets ─────────────────────────────────────────────────────

AddSetResponse WorkoutService::add_set(
    const std::string& workout_id,
    const std::string& user_id,
    const std::string& exercise_id,
    const std::optional<double>& weight_kg,
    const std::optional<int>& reps,
    const std::optional<double>& rpe,
    const std::string& set_type,
    const std::optional<int>& duration_seconds,
    const std::optional<int>& rest_seconds,
    const std::optional<std::string>& notes
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Verify workout ownership and status
    auto workout_check = txn.exec_params(
        "SELECT id, status FROM workouts "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        workout_id, user_id
    );

    if (workout_check.empty()) {
        throw std::invalid_argument("WORKOUT_NOT_FOUND");
    }

    std::string workout_status = workout_check[0]["status"].as<std::string>();
    if (workout_status != "in_progress") {
        throw std::invalid_argument("WORKOUT_NOT_IN_PROGRESS");
    }

    // Verify exercise exists
    auto exercise_check = txn.exec_params(
        "SELECT id FROM exercises WHERE id = $1",
        exercise_id
    );

    if (exercise_check.empty()) {
        throw std::invalid_argument("EXERCISE_NOT_FOUND");
    }

    // Get next set_order and exercise_order
    auto [next_set_order, exercise_order] = get_next_set_order(txn, workout_id, exercise_id);

    // Detect PR using Epley formula
    bool is_pr = false;
    std::optional<double> estimated_1rm;
    std::optional<double> previous_best_1rm;

    if (set_type == "working" && weight_kg && reps && *weight_kg > 0 && *reps > 0) {
        double current_1rm = calculate_estimated_1rm(*weight_kg, *reps);
        estimated_1rm = current_1rm;

        // Get user's best estimated 1RM for this exercise (across all workouts)
        auto best_result = txn.exec_params(
            "SELECT weight_kg, reps FROM exercise_sets es "
            "JOIN workouts w ON es.workout_id = w.id "
            "WHERE es.exercise_id = $1 AND w.user_id = $2 "
            "AND w.deleted_at IS NULL AND es.set_type = 'working' "
            "AND es.weight_kg IS NOT NULL AND es.reps IS NOT NULL "
            "AND es.weight_kg > 0 AND es.reps > 0",
            exercise_id, user_id
        );

        double best_1rm = 0.0;
        for (const auto& row : best_result) {
            double w = row["weight_kg"].as<double>();
            int r = row["reps"].as<int>();
            double rm = calculate_estimated_1rm(w, r);
            if (rm > best_1rm) best_1rm = rm;
        }

        if (best_1rm > 0.0) {
            previous_best_1rm = best_1rm;
        }

        if (current_1rm > best_1rm) {
            is_pr = true;
        }
    }

    // Insert the set
    std::string set_id = UUID::generate();

    // Build INSERT with explicit NULL handling for optional fields
    std::ostringstream insert_query;
    insert_query << "INSERT INTO exercise_sets "
                 << "(id, workout_id, exercise_id, set_order, exercise_order, set_type, "
                 << "reps, weight_kg, duration_seconds, rpe, rest_seconds, is_pr, notes) "
                 << "VALUES ("
                 << conn->quote(set_id) << ", "
                 << conn->quote(workout_id) << ", "
                 << conn->quote(exercise_id) << ", "
                 << next_set_order << ", "
                 << exercise_order << ", "
                 << conn->quote(set_type) << ", ";

    if (reps) insert_query << *reps; else insert_query << "NULL";
    insert_query << ", ";
    if (weight_kg) insert_query << *weight_kg; else insert_query << "NULL";
    insert_query << ", ";
    if (duration_seconds) insert_query << *duration_seconds; else insert_query << "NULL";
    insert_query << ", ";
    if (rpe) insert_query << *rpe; else insert_query << "NULL";
    insert_query << ", ";
    if (rest_seconds) insert_query << *rest_seconds; else insert_query << "NULL";
    insert_query << ", ";
    insert_query << (is_pr ? "true" : "false") << ", ";
    if (notes) insert_query << conn->quote(*notes); else insert_query << "NULL";
    insert_query << ")";

    txn.exec(insert_query.str());

    txn.commit();

    // Build response
    ExerciseSet es;
    es.id = set_id;
    es.workout_id = workout_id;
    es.exercise_id = exercise_id;
    es.set_order = next_set_order;
    es.exercise_order = exercise_order;
    es.set_type = set_type;
    es.reps = reps;
    es.weight_kg = weight_kg;
    es.duration_seconds = duration_seconds;
    es.rpe = rpe;
    es.rest_seconds = rest_seconds;
    es.is_pr = is_pr;
    es.notes = notes;

    AddSetResponse response;
    response.set = es;
    response.is_pr = is_pr;
    response.estimated_1rm = estimated_1rm;
    response.previous_best_1rm = previous_best_1rm;

    return response;
}

std::optional<AddSetResponse> WorkoutService::update_set(
    const std::string& workout_id,
    const std::string& set_id,
    const std::string& user_id,
    const std::optional<double>& weight_kg,
    const std::optional<int>& reps,
    const std::optional<double>& rpe,
    const std::optional<std::string>& set_type,
    const std::optional<int>& duration_seconds,
    const std::optional<int>& rest_seconds,
    const std::optional<std::string>& notes
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Verify workout ownership
    if (!verify_workout_ownership(txn, workout_id, user_id)) {
        return std::nullopt;
    }

    // Get existing set
    auto existing = txn.exec_params(
        "SELECT es.* FROM exercise_sets es "
        "JOIN workouts w ON es.workout_id = w.id "
        "WHERE es.id = $1 AND es.workout_id = $2 AND w.user_id = $3",
        set_id, workout_id, user_id
    );

    if (existing.empty()) {
        return std::nullopt;
    }

    auto row = existing[0];
    std::string exercise_id = row["exercise_id"].as<std::string>();

    // Build dynamic update
    std::ostringstream query;
    query << "UPDATE exercise_sets SET updated_at = NOW()";

    // Use new values or keep existing
    double final_weight = weight_kg.value_or(
        row["weight_kg"].is_null() ? 0.0 : row["weight_kg"].as<double>()
    );
    int final_reps = reps.value_or(
        row["reps"].is_null() ? 0 : row["reps"].as<int>()
    );
    std::string final_set_type = set_type.value_or(row["set_type"].as<std::string>());

    if (weight_kg) {
        query << ", weight_kg = " << *weight_kg;
    }
    if (reps) {
        query << ", reps = " << *reps;
    }
    if (rpe) {
        query << ", rpe = " << *rpe;
    }
    if (set_type) {
        query << ", set_type = " << conn->quote(*set_type);
    }
    if (duration_seconds) {
        query << ", duration_seconds = " << *duration_seconds;
    }
    if (rest_seconds) {
        query << ", rest_seconds = " << *rest_seconds;
    }
    if (notes) {
        query << ", notes = " << conn->quote(*notes);
    }

    // Re-evaluate PR status
    bool is_pr = false;
    std::optional<double> estimated_1rm;
    std::optional<double> previous_best_1rm;

    if (final_set_type == "working" && final_weight > 0.0 && final_reps > 0) {
        double current_1rm = calculate_estimated_1rm(final_weight, final_reps);
        estimated_1rm = current_1rm;

        // Get best 1RM excluding this set
        auto best_result = txn.exec_params(
            "SELECT weight_kg, reps FROM exercise_sets es "
            "JOIN workouts w ON es.workout_id = w.id "
            "WHERE es.exercise_id = $1 AND w.user_id = $2 "
            "AND w.deleted_at IS NULL AND es.set_type = 'working' "
            "AND es.weight_kg IS NOT NULL AND es.reps IS NOT NULL "
            "AND es.weight_kg > 0 AND es.reps > 0 "
            "AND es.id != $3",
            exercise_id, user_id, set_id
        );

        double best_1rm = 0.0;
        for (const auto& r : best_result) {
            double w = r["weight_kg"].as<double>();
            int rep_count = r["reps"].as<int>();
            double rm = calculate_estimated_1rm(w, rep_count);
            if (rm > best_1rm) best_1rm = rm;
        }

        if (best_1rm > 0.0) previous_best_1rm = best_1rm;

        if (current_1rm > best_1rm) {
            is_pr = true;
        }
    }

    query << ", is_pr = " << (is_pr ? "true" : "false");
    query << " WHERE id = " << conn->quote(set_id);

    txn.exec(query.str());
    txn.commit();

    // Build response with updated values
    ExerciseSet es;
    es.id = set_id;
    es.workout_id = workout_id;
    es.exercise_id = exercise_id;
    es.set_order = row["set_order"].as<int>();
    es.exercise_order = row["exercise_order"].as<int>();
    es.set_type = final_set_type;
    es.reps = final_reps > 0 ? std::optional<int>(final_reps) : std::nullopt;
    es.weight_kg = final_weight > 0.0 ? std::optional<double>(final_weight) : std::nullopt;
    if (duration_seconds) {
        es.duration_seconds = *duration_seconds;
    } else if (!row["duration_seconds"].is_null()) {
        es.duration_seconds = row["duration_seconds"].as<int>();
    }
    if (rpe) {
        es.rpe = *rpe;
    } else if (!row["rpe"].is_null()) {
        es.rpe = row["rpe"].as<double>();
    }
    if (rest_seconds) {
        es.rest_seconds = *rest_seconds;
    } else if (!row["rest_seconds"].is_null()) {
        es.rest_seconds = row["rest_seconds"].as<int>();
    }
    if (notes) {
        es.notes = *notes;
    } else if (!row["notes"].is_null()) {
        es.notes = row["notes"].as<std::string>();
    }
    es.is_pr = is_pr;

    AddSetResponse response;
    response.set = es;
    response.is_pr = is_pr;
    response.estimated_1rm = estimated_1rm;
    response.previous_best_1rm = previous_best_1rm;

    return response;
}

bool WorkoutService::delete_set(
    const std::string& workout_id,
    const std::string& set_id,
    const std::string& user_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Verify workout ownership
    if (!verify_workout_ownership(txn, workout_id, user_id)) {
        return false;
    }

    auto result = txn.exec_params(
        "DELETE FROM exercise_sets "
        "WHERE id = $1 AND workout_id = $2",
        set_id, workout_id
    );

    txn.commit();
    return result.affected_rows() > 0;
}

// ── Exercise Library ──────────────────────────────────────────────────

std::vector<Exercise> WorkoutService::list_exercises(
    const std::optional<std::string>& muscle_group,
    const std::optional<std::string>& search,
    const std::optional<std::string>& equipment,
    int limit,
    int offset
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    std::ostringstream query;
    query << "SELECT id, name, muscle_group, equipment, is_custom, created_by "
          << "FROM exercises WHERE 1=1";

    if (muscle_group) {
        query << " AND muscle_group = " << conn->quote(*muscle_group);
    }
    if (equipment) {
        query << " AND equipment = " << conn->quote(*equipment);
    }
    if (search) {
        // Case-insensitive search by name
        query << " AND name ILIKE " << conn->quote("%" + *search + "%");
    }

    query << " ORDER BY is_custom ASC, name ASC";
    query << " LIMIT " << limit << " OFFSET " << offset;

    auto result = txn.exec(query.str());

    std::vector<Exercise> exercises;
    for (const auto& row : result) {
        Exercise ex;
        ex.id = row["id"].as<std::string>();
        ex.name = row["name"].as<std::string>();
        ex.muscle_group = row["muscle_group"].as<std::string>();
        ex.equipment = row["equipment"].as<std::string>();
        ex.is_custom = row["is_custom"].as<bool>();
        if (!row["created_by"].is_null()) {
            ex.created_by = row["created_by"].as<std::string>();
        }
        exercises.push_back(ex);
    }

    return exercises;
}

std::optional<Exercise> WorkoutService::get_exercise(const std::string& exercise_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT id, name, muscle_group, equipment, is_custom, created_by "
        "FROM exercises WHERE id = $1",
        exercise_id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    auto row = result[0];
    Exercise ex;
    ex.id = row["id"].as<std::string>();
    ex.name = row["name"].as<std::string>();
    ex.muscle_group = row["muscle_group"].as<std::string>();
    ex.equipment = row["equipment"].as<std::string>();
    ex.is_custom = row["is_custom"].as<bool>();
    if (!row["created_by"].is_null()) {
        ex.created_by = row["created_by"].as<std::string>();
    }

    return ex;
}

// ── Exercise History & PR Detection ───────────────────────────────────

std::vector<ExerciseHistoryEntry> WorkoutService::get_exercise_history(
    const std::string& exercise_id,
    const std::string& user_id,
    int limit
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Get the last N distinct workouts containing this exercise
    auto workout_ids_result = txn.exec_params(
        "SELECT w.id, w.name, "
        "       TO_CHAR(w.started_at, 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS workout_date, "
        "       w.started_at "
        "FROM workouts w "
        "JOIN exercise_sets es ON w.id = es.workout_id "
        "WHERE w.user_id = $1 AND es.exercise_id = $2 "
        "AND w.deleted_at IS NULL AND w.status = 'completed' "
        "GROUP BY w.id, w.name, w.started_at "
        "ORDER BY w.started_at DESC "
        "LIMIT $3",
        user_id, exercise_id, limit
    );

    std::vector<ExerciseHistoryEntry> history;

    for (const auto& w_row : workout_ids_result) {
        ExerciseHistoryEntry entry;
        entry.workout_id = w_row["id"].as<std::string>();
        entry.workout_date = w_row["workout_date"].as<std::string>();
        if (!w_row["name"].is_null()) {
            entry.workout_name = w_row["name"].as<std::string>();
        }

        // Get sets for this workout and exercise
        auto sets_result = txn.exec_params(
            "SELECT id, workout_id, exercise_id, set_order, exercise_order, "
            "       set_type, reps, weight_kg, duration_seconds, rpe, "
            "       rest_seconds, is_pr, notes "
            "FROM exercise_sets "
            "WHERE workout_id = $1 AND exercise_id = $2 "
            "ORDER BY set_order ASC",
            entry.workout_id, exercise_id
        );

        for (const auto& set_row : sets_result) {
            ExerciseSet es;
            es.id = set_row["id"].as<std::string>();
            es.workout_id = set_row["workout_id"].as<std::string>();
            es.exercise_id = set_row["exercise_id"].as<std::string>();
            es.set_order = set_row["set_order"].as<int>();
            es.exercise_order = set_row["exercise_order"].as<int>();
            es.set_type = set_row["set_type"].as<std::string>();
            if (!set_row["reps"].is_null()) es.reps = set_row["reps"].as<int>();
            if (!set_row["weight_kg"].is_null()) es.weight_kg = set_row["weight_kg"].as<double>();
            if (!set_row["duration_seconds"].is_null()) es.duration_seconds = set_row["duration_seconds"].as<int>();
            if (!set_row["rpe"].is_null()) es.rpe = set_row["rpe"].as<double>();
            if (!set_row["rest_seconds"].is_null()) es.rest_seconds = set_row["rest_seconds"].as<int>();
            es.is_pr = set_row["is_pr"].as<bool>();
            if (!set_row["notes"].is_null()) es.notes = set_row["notes"].as<std::string>();
            entry.sets.push_back(es);
        }

        history.push_back(entry);
    }

    return history;
}

std::optional<double> WorkoutService::get_best_1rm(
    const std::string& exercise_id,
    const std::string& user_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT weight_kg, reps FROM exercise_sets es "
        "JOIN workouts w ON es.workout_id = w.id "
        "WHERE es.exercise_id = $1 AND w.user_id = $2 "
        "AND w.deleted_at IS NULL AND es.set_type = 'working' "
        "AND es.weight_kg IS NOT NULL AND es.reps IS NOT NULL "
        "AND es.weight_kg > 0 AND es.reps > 0",
        exercise_id, user_id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    double best = 0.0;
    for (const auto& row : result) {
        double w = row["weight_kg"].as<double>();
        int r = row["reps"].as<int>();
        double rm = calculate_estimated_1rm(w, r);
        if (rm > best) best = rm;
    }

    return best > 0.0 ? std::optional<double>(best) : std::nullopt;
}

} // namespace forge
