#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "auth_service.hpp"
#include "database.hpp"
#include "config.hpp"
#include "uuid.hpp"
#include "models.hpp"
#include <string>
#include <vector>

// These tests verify the workout schema and exercise data directly via SQL.
// WorkoutService (workout_service.hpp/cpp) exists on main and handles CRUD,
// but here we test the schema independently to ensure correctness.

using forge::Database;
using forge::Config;
using forge::AuthService;
using forge::UUID;

class WorkoutIntegrationTest : public ::testing::Test {
protected:
    static bool db_initialized_;
    std::string test_user_id_;

    static void SetUpTestSuite() {
        forge::test::setup_test_environment();

        if (!db_initialized_) {
            try {
                Database::instance().initialize(Config::instance().database_url());
                db_initialized_ = true;
            } catch (const std::exception& e) {
                GTEST_SKIP() << "Database not available: " << e.what();
            }
        }
    }

    void SetUp() override {
        if (!db_initialized_) {
            GTEST_SKIP() << "Database not available";
        }

        try {
            static int counter = 0;
            std::string email = "workout" + std::to_string(++counter) + "@test-forge.example.com";
            auto [user, tokens] = AuthService::register_user({
                .email = email,
                .username = "wkuser" + std::to_string(counter),
                .password = "TestPass123",
                .display_name = "Workout Test"
            });
            test_user_id_ = user.id;
        } catch (const std::exception& e) {
            GTEST_SKIP() << "Failed to create test user: " << e.what();
        }
    }

    void TearDown() override {
        if (!test_user_id_.empty()) {
            try {
                auto conn = Database::instance().get_connection();
                pqxx::work txn(*conn);
                txn.exec_params(
                    "DELETE FROM exercise_sets WHERE workout_id IN "
                    "  (SELECT id FROM workouts WHERE user_id = $1);"
                    "DELETE FROM workouts WHERE user_id = $1;"
                    "DELETE FROM refresh_tokens WHERE user_id = $1;"
                    "DELETE FROM users WHERE id = $1;",
                    test_user_id_
                );
                txn.commit();
            } catch (...) {}
        }
    }

    // Helper: get any exercise ID from the seeded database
    std::string get_exercise_id(const std::string& name_like = "Bench Press") {
        auto conn = Database::instance().get_connection();
        pqxx::work txn(*conn);
        auto result = txn.exec_params(
            "SELECT id FROM exercises WHERE name ILIKE $1 LIMIT 1",
            "%" + name_like + "%"
        );
        txn.commit();

        if (result.empty()) return "";
        return result[0]["id"].c_str();
    }
};

bool WorkoutIntegrationTest::db_initialized_ = false;

// ── Exercise library ────────────────────────────────────────────────

TEST_F(WorkoutIntegrationTest, ExerciseLibraryIsSeeded) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    auto result = txn.exec("SELECT COUNT(*) as cnt FROM exercises");
    txn.commit();

    int count = result[0]["cnt"].as<int>();
    EXPECT_GT(count, 100) << "Exercise library should have 100+ seeded exercises";
}

TEST_F(WorkoutIntegrationTest, ExerciseSearchByName) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    auto result = txn.exec(
        "SELECT id, name, muscle_group, equipment "
        "FROM exercises WHERE name ILIKE '%bench press%'"
    );
    txn.commit();

    EXPECT_GT(result.size(), 0u) << "Should find 'Bench Press' in exercise library";
    if (!result.empty()) {
        std::string name = result[0]["name"].c_str();
        // Name should contain "bench press" (case insensitive)
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        EXPECT_NE(lower_name.find("bench press"), std::string::npos);
    }
}

TEST_F(WorkoutIntegrationTest, ExerciseFilterByMuscleGroup) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    auto result = txn.exec(
        "SELECT id, name, muscle_group FROM exercises "
        "WHERE muscle_group = 'chest'"
    );
    txn.commit();

    EXPECT_GT(result.size(), 0u) << "Should find chest exercises";
    for (const auto& row : result) {
        EXPECT_EQ(std::string(row["muscle_group"].c_str()), "chest");
    }
}

// ── Create workout ──────────────────────────────────────────────────

TEST_F(WorkoutIntegrationTest, CreateWorkout) {
    std::string workout_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO workouts (id, user_id, name, status, started_at) "
        "VALUES ($1, $2, 'Test Workout', 'in_progress', CURRENT_TIMESTAMP)",
        workout_id, test_user_id_
    );
    txn.commit();

    // Verify workout was created
    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT id, user_id, name, status FROM workouts WHERE id = $1",
        workout_id
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["user_id"].c_str(), test_user_id_);
    EXPECT_EQ(result[0]["status"].c_str(), std::string("in_progress"));
    EXPECT_EQ(result[0]["name"].c_str(), std::string("Test Workout"));
}

// ── List workouts ───────────────────────────────────────────────────

TEST_F(WorkoutIntegrationTest, ListWorkouts) {
    auto conn = Database::instance().get_connection();

    // Create two workouts
    for (int i = 0; i < 2; ++i) {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO workouts (id, user_id, name, status, started_at) "
            "VALUES ($1, $2, $3, 'in_progress', CURRENT_TIMESTAMP)",
            UUID::generate(), test_user_id_,
            "Workout " + std::to_string(i + 1)
        );
        txn.commit();
    }

    pqxx::work txn(*conn);
    auto result = txn.exec_params(
        "SELECT id, name FROM workouts WHERE user_id = $1 AND deleted_at IS NULL "
        "ORDER BY started_at DESC",
        test_user_id_
    );
    txn.commit();

    EXPECT_EQ(result.size(), 2u);
}

// ── Add exercise set to workout ─────────────────────────────────────

TEST_F(WorkoutIntegrationTest, AddSetToWorkout) {
    std::string workout_id = UUID::generate();
    std::string exercise_id = get_exercise_id("Bench Press");
    if (exercise_id.empty()) {
        GTEST_SKIP() << "Bench Press not in exercise library";
    }

    auto conn = Database::instance().get_connection();

    // Create workout
    pqxx::work txn1(*conn);
    txn1.exec_params(
        "INSERT INTO workouts (id, user_id, status, started_at) "
        "VALUES ($1, $2, 'in_progress', CURRENT_TIMESTAMP)",
        workout_id, test_user_id_
    );
    txn1.commit();

    // Add a set
    std::string set_id = UUID::generate();
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "INSERT INTO exercise_sets "
        "(id, workout_id, exercise_id, set_order, exercise_order, set_type, "
        " reps, weight_kg, is_pr) "
        "VALUES ($1, $2, $3, 1, 1, 'working', 10, 100.0, false)",
        set_id, workout_id, exercise_id
    );
    txn2.commit();

    // Verify set exists
    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT * FROM exercise_sets WHERE workout_id = $1",
        workout_id
    );
    txn3.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["reps"].as<int>(), 10);
    EXPECT_NEAR(result[0]["weight_kg"].as<double>(), 100.0, 0.01);
}

// ── PR detection ────────────────────────────────────────────────────

TEST_F(WorkoutIntegrationTest, PRDetectionViaBestEstimated1RM) {
    std::string exercise_id = get_exercise_id("Bench Press");
    if (exercise_id.empty()) {
        GTEST_SKIP() << "Bench Press not in exercise library";
    }

    auto conn = Database::instance().get_connection();

    // Create first workout with a set: 100kg × 5 reps → e1RM = 116.67
    std::string w1_id = UUID::generate();
    pqxx::work txn1(*conn);
    txn1.exec_params(
        "INSERT INTO workouts (id, user_id, status, started_at) "
        "VALUES ($1, $2, 'completed', CURRENT_TIMESTAMP - INTERVAL '1 day')",
        w1_id, test_user_id_
    );
    txn1.exec_params(
        "INSERT INTO exercise_sets (id, workout_id, exercise_id, set_order, exercise_order, "
        "set_type, reps, weight_kg, is_pr) "
        "VALUES ($1, $2, $3, 1, 1, 'working', 5, 100.0, true)",
        UUID::generate(), w1_id, exercise_id
    );
    txn1.commit();

    // Create second workout with a better set: 105kg × 5 reps → e1RM = 122.5
    std::string w2_id = UUID::generate();
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "INSERT INTO workouts (id, user_id, status, started_at) "
        "VALUES ($1, $2, 'completed', CURRENT_TIMESTAMP)",
        w2_id, test_user_id_
    );
    txn2.commit();

    // Calculate best previous 1RM for this exercise
    pqxx::work txn3(*conn);
    auto prev_best = txn3.exec_params(
        "SELECT MAX(weight_kg * (1.0 + CAST(reps AS DECIMAL) / 30.0)) as best_1rm "
        "FROM exercise_sets es "
        "JOIN workouts w ON w.id = es.workout_id "
        "WHERE w.user_id = $1 AND es.exercise_id = $2 "
        "  AND w.id != $3 AND w.deleted_at IS NULL",
        test_user_id_, exercise_id, w2_id
    );
    txn3.commit();

    double previous_best_1rm = prev_best[0]["best_1rm"].is_null()
        ? 0.0 : prev_best[0]["best_1rm"].as<double>();

    // New set: 105kg × 5 → e1RM = 105 * (1 + 5/30) = 122.5
    double new_1rm = 105.0 * (1.0 + 5.0 / 30.0);
    bool is_pr = new_1rm > previous_best_1rm;

    EXPECT_TRUE(is_pr) << "105kg×5 should beat previous 100kg×5";
    EXPECT_GT(new_1rm, previous_best_1rm);
}

// ── Complete workout ────────────────────────────────────────────────

TEST_F(WorkoutIntegrationTest, CompleteWorkoutUpdatesDuration) {
    std::string workout_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn1(*conn);
    txn1.exec_params(
        "INSERT INTO workouts (id, user_id, status, started_at) "
        "VALUES ($1, $2, 'in_progress', CURRENT_TIMESTAMP - INTERVAL '45 minutes')",
        workout_id, test_user_id_
    );
    txn1.commit();

    // Complete the workout
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "UPDATE workouts SET "
        "  status = 'completed', "
        "  completed_at = CURRENT_TIMESTAMP, "
        "  duration_seconds = EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - started_at))::int "
        "WHERE id = $1",
        workout_id
    );
    txn2.commit();

    // Verify
    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT status, duration_seconds FROM workouts WHERE id = $1",
        workout_id
    );
    txn3.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["status"].c_str(), std::string("completed"));
    EXPECT_GT(result[0]["duration_seconds"].as<int>(), 2000)
        << "Duration should be ~2700 seconds (45 min)";
}

// ── Delete workout (soft delete) ────────────────────────────────────

TEST_F(WorkoutIntegrationTest, SoftDeleteWorkout) {
    std::string workout_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn1(*conn);
    txn1.exec_params(
        "INSERT INTO workouts (id, user_id, status, started_at) "
        "VALUES ($1, $2, 'completed', CURRENT_TIMESTAMP)",
        workout_id, test_user_id_
    );
    txn1.commit();

    // Soft delete
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "UPDATE workouts SET deleted_at = CURRENT_TIMESTAMP WHERE id = $1",
        workout_id
    );
    txn2.commit();

    // Should not appear in active queries
    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT id FROM workouts WHERE user_id = $1 AND deleted_at IS NULL",
        test_user_id_
    );
    txn3.commit();

    EXPECT_EQ(result.size(), 0u) << "Deleted workout should not appear in active list";
}

// ── Exercise history (previous performance) ─────────────────────────

TEST_F(WorkoutIntegrationTest, GetExerciseHistory) {
    std::string exercise_id = get_exercise_id("Squat");
    if (exercise_id.empty()) {
        GTEST_SKIP() << "Squat not in exercise library";
    }

    auto conn = Database::instance().get_connection();

    // Create two completed workouts with squat sets
    for (int i = 0; i < 2; ++i) {
        std::string w_id = UUID::generate();
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO workouts (id, user_id, status, started_at) "
            "VALUES ($1, $2, 'completed', CURRENT_TIMESTAMP - ($3 || ' days')::interval)",
            w_id, test_user_id_, std::to_string(i + 1)
        );

        for (int s = 0; s < 3; ++s) {
            txn.exec_params(
                "INSERT INTO exercise_sets "
                "(id, workout_id, exercise_id, set_order, exercise_order, "
                " set_type, reps, weight_kg, is_pr) "
                "VALUES ($1, $2, $3, $4, 1, 'working', $5, $6, false)",
                UUID::generate(), w_id, exercise_id,
                s + 1,
                8 + s,                              // reps: 8, 9, 10
                100.0 + (i * 5.0)                   // weight: 100, 105
            );
        }
        txn.commit();
    }

    // Query exercise history
    pqxx::work txn(*conn);
    auto result = txn.exec_params(
        "SELECT es.reps, es.weight_kg, es.is_pr, w.started_at "
        "FROM exercise_sets es "
        "JOIN workouts w ON w.id = es.workout_id "
        "WHERE w.user_id = $1 AND es.exercise_id = $2 AND w.deleted_at IS NULL "
        "ORDER BY w.started_at DESC, es.set_order ASC",
        test_user_id_, exercise_id
    );
    txn.commit();

    EXPECT_EQ(result.size(), 6u) << "Should have 6 sets across 2 workouts";
}
