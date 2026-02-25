#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"
#include "models.hpp"

namespace forge {

// Response struct for sets that may be PRs
struct AddSetResponse {
    ExerciseSet set;
    bool is_pr;
    std::optional<double> estimated_1rm;
    std::optional<double> previous_best_1rm;

    nlohmann::json to_json() const;
};

// Struct for grouping exercise history by workout date
struct ExerciseHistoryEntry {
    std::string workout_id;
    std::string workout_date;
    std::optional<std::string> workout_name;
    std::vector<ExerciseSet> sets;

    nlohmann::json to_json() const;
};

// Summary for workout listing (without full set details)
struct WorkoutSummary {
    std::string id;
    std::string user_id;
    std::optional<std::string> name;
    std::string started_at;
    std::optional<std::string> completed_at;
    std::optional<int> duration_seconds;
    std::string status;
    int exercise_count;
    int set_count;
    double total_volume_kg;
    int pr_count;

    nlohmann::json to_json() const;
};

class WorkoutService {
public:
    explicit WorkoutService(Database& db) : db_(db) {}

    // ── Workout CRUD ──────────────────────────────────────────────

    // POST /api/workouts - Start a new workout
    std::string create_workout(
        const std::string& user_id,
        const std::optional<std::string>& name = std::nullopt,
        const std::optional<std::string>& notes = std::nullopt
    );

    // GET /api/workouts - List user's workouts (paginated, most recent first)
    std::vector<WorkoutSummary> list_workouts(
        const std::string& user_id,
        int limit = 20,
        int offset = 0
    );

    // GET /api/workouts/:id - Get workout detail with all sets
    std::optional<Workout> get_workout(
        const std::string& workout_id,
        const std::string& user_id
    );

    // PUT /api/workouts/:id/complete - Complete workout
    std::optional<Workout> complete_workout(
        const std::string& workout_id,
        const std::string& user_id,
        const std::optional<std::string>& notes = std::nullopt
    );

    // DELETE /api/workouts/:id - Soft delete workout
    bool delete_workout(
        const std::string& workout_id,
        const std::string& user_id
    );

    // ── Exercise Sets ─────────────────────────────────────────────

    // POST /api/workouts/:id/sets - Add a set to workout
    AddSetResponse add_set(
        const std::string& workout_id,
        const std::string& user_id,
        const std::string& exercise_id,
        const std::optional<double>& weight_kg,
        const std::optional<int>& reps,
        const std::optional<double>& rpe,
        const std::string& set_type = "working",
        const std::optional<int>& duration_seconds = std::nullopt,
        const std::optional<int>& rest_seconds = std::nullopt,
        const std::optional<std::string>& notes = std::nullopt
    );

    // PUT /api/workouts/:id/sets/:set_id - Update a set
    std::optional<AddSetResponse> update_set(
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
    );

    // DELETE /api/workouts/:id/sets/:set_id - Delete a set
    bool delete_set(
        const std::string& workout_id,
        const std::string& set_id,
        const std::string& user_id
    );

    // ── Exercise Library ──────────────────────────────────────────

    // GET /api/exercises - List/search exercises
    std::vector<Exercise> list_exercises(
        const std::optional<std::string>& muscle_group = std::nullopt,
        const std::optional<std::string>& search = std::nullopt,
        const std::optional<std::string>& equipment = std::nullopt,
        int limit = 50,
        int offset = 0
    );

    // GET /api/exercises/:id - Get exercise detail
    std::optional<Exercise> get_exercise(const std::string& exercise_id);

    // ── Exercise History & PR Detection ───────────────────────────

    // GET /api/exercises/:id/history - Get last 5 performances
    std::vector<ExerciseHistoryEntry> get_exercise_history(
        const std::string& exercise_id,
        const std::string& user_id,
        int limit = 5
    );

    // Calculate estimated 1RM using Epley formula
    static double calculate_estimated_1rm(double weight_kg, int reps);

    // Get user's best estimated 1RM for an exercise
    std::optional<double> get_best_1rm(
        const std::string& exercise_id,
        const std::string& user_id
    );

private:
    Database& db_;

    // Helper: verify workout belongs to user and is not deleted
    bool verify_workout_ownership(
        pqxx::work& txn,
        const std::string& workout_id,
        const std::string& user_id
    );

    // Helper: calculate total volume for a workout
    double calculate_total_volume(pqxx::work& txn, const std::string& workout_id);

    // Helper: determine next set_order and exercise_order
    std::pair<int, int> get_next_set_order(
        pqxx::work& txn,
        const std::string& workout_id,
        const std::string& exercise_id
    );
};

} // namespace forge
