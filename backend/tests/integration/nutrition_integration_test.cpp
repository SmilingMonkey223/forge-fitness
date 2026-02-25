#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "auth_service.hpp"
#include "database.hpp"
#include "config.hpp"
#include "uuid.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

// NOTE: NutritionService does not exist on main yet.
// These tests verify the nutrition_log and custom_foods schema directly via SQL,
// documenting the expected behavior for when the service is implemented.

using forge::Database;
using forge::Config;
using forge::AuthService;
using forge::UUID;

class NutritionIntegrationTest : public ::testing::Test {
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
            std::string email = "nutr" + std::to_string(++counter) + "@test-forge.example.com";
            auto [user, tokens] = AuthService::register_user({
                .email = email,
                .username = "nutruser" + std::to_string(counter),
                .password = "TestPass123",
                .display_name = "Nutrition Test"
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
                    "DELETE FROM nutrition_log WHERE user_id = $1;"
                    "DELETE FROM custom_foods WHERE user_id = $1;"
                    "DELETE FROM refresh_tokens WHERE user_id = $1;"
                    "DELETE FROM users WHERE id = $1;",
                    test_user_id_
                );
                txn.commit();
            } catch (...) {}
        }
    }

    // Helper: get today's date as YYYY-MM-DD
    std::string today_date() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        gmtime_r(&time_t_now, &tm);
        char date_buf[11];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &tm);
        return std::string(date_buf);
    }
};

bool NutritionIntegrationTest::db_initialized_ = false;

// ── Log food ────────────────────────────────────────────────────────

TEST_F(NutritionIntegrationTest, LogFoodEntry) {
    std::string log_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, food_name, serving_size, serving_unit, quantity, "
        " calories, protein_g, carbs_g, fat_g, is_custom, source, logged_at) "
        "VALUES ($1, $2, 'Chicken Breast', 100.0, 'g', 1.0, "
        " 165.0, 31.0, 0.0, 3.6, false, 'manual', CURRENT_TIMESTAMP)",
        log_id, test_user_id_
    );
    txn.commit();

    // Verify entry was created
    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT * FROM nutrition_log WHERE id = $1",
        log_id
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["user_id"].c_str(), test_user_id_);
    EXPECT_EQ(result[0]["food_name"].c_str(), std::string("Chicken Breast"));
    EXPECT_NEAR(result[0]["calories"].as<double>(), 165.0, 0.01);
    EXPECT_NEAR(result[0]["protein_g"].as<double>(), 31.0, 0.01);
}

TEST_F(NutritionIntegrationTest, LogFoodWithMealType) {
    std::string log_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, food_name, serving_size, serving_unit, quantity, "
        " calories, protein_g, carbs_g, fat_g, meal_type, is_custom, source, logged_at) "
        "VALUES ($1, $2, 'Oatmeal', 100.0, 'g', 1.0, "
        " 150.0, 5.0, 27.0, 2.5, 'breakfast', false, 'manual', CURRENT_TIMESTAMP)",
        log_id, test_user_id_
    );
    txn.commit();

    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT meal_type FROM nutrition_log WHERE id = $1",
        log_id
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["meal_type"].c_str(), std::string("breakfast"));
}

// ── Get logs for date ───────────────────────────────────────────────

TEST_F(NutritionIntegrationTest, GetLogsForDateReturnsLoggedEntries) {
    auto conn = Database::instance().get_connection();

    // Log two entries for today
    for (int i = 0; i < 2; ++i) {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO nutrition_log "
            "(id, user_id, food_name, serving_size, serving_unit, quantity, "
            " calories, protein_g, carbs_g, fat_g, is_custom, source, logged_at) "
            "VALUES ($1, $2, $3, 100.0, 'g', 1.0, "
            " $4, 10.0, 20.0, 5.0, false, 'manual', CURRENT_TIMESTAMP)",
            UUID::generate(), test_user_id_,
            "Food " + std::to_string(i + 1),
            100.0 + i * 50.0
        );
        txn.commit();
    }

    std::string today = today_date();

    pqxx::work txn(*conn);
    auto result = txn.exec_params(
        "SELECT * FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date",
        test_user_id_, today
    );
    txn.commit();

    EXPECT_GE(result.size(), 2u) << "Should have at least 2 entries for today";
}

// ── Update log ──────────────────────────────────────────────────────

TEST_F(NutritionIntegrationTest, UpdateLogEntry) {
    std::string log_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, food_name, serving_size, serving_unit, quantity, "
        " calories, protein_g, carbs_g, fat_g, is_custom, source, logged_at) "
        "VALUES ($1, $2, 'Rice', 100.0, 'g', 1.0, "
        " 130.0, 2.7, 28.0, 0.3, false, 'manual', CURRENT_TIMESTAMP)",
        log_id, test_user_id_
    );
    txn.commit();

    // Update calories and quantity
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "UPDATE nutrition_log SET calories = 260.0, quantity = 2.0 WHERE id = $1 AND user_id = $2",
        log_id, test_user_id_
    );
    txn2.commit();

    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT calories, quantity FROM nutrition_log WHERE id = $1",
        log_id
    );
    txn3.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_NEAR(result[0]["calories"].as<double>(), 260.0, 0.01);
    EXPECT_NEAR(result[0]["quantity"].as<double>(), 2.0, 0.01);
}

// ── Delete log ──────────────────────────────────────────────────────

TEST_F(NutritionIntegrationTest, DeleteLogEntry) {
    std::string log_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, food_name, serving_size, serving_unit, quantity, "
        " calories, protein_g, carbs_g, fat_g, is_custom, source, logged_at) "
        "VALUES ($1, $2, 'To Delete', 100.0, 'g', 1.0, "
        " 100.0, 10.0, 10.0, 5.0, false, 'manual', CURRENT_TIMESTAMP)",
        log_id, test_user_id_
    );
    txn.commit();

    // Delete
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "DELETE FROM nutrition_log WHERE id = $1 AND user_id = $2",
        log_id, test_user_id_
    );
    txn2.commit();

    // Verify deleted
    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT * FROM nutrition_log WHERE id = $1",
        log_id
    );
    txn3.commit();

    EXPECT_EQ(result.size(), 0u);
}

// ── Daily summary aggregation ───────────────────────────────────────

TEST_F(NutritionIntegrationTest, DailySummaryAggregatesCorrectly) {
    auto conn = Database::instance().get_connection();

    // Log food entries with different quantities
    pqxx::work txn1(*conn);
    txn1.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, food_name, serving_size, serving_unit, quantity, "
        " calories, protein_g, carbs_g, fat_g, is_custom, source, logged_at) "
        "VALUES ($1, $2, 'Rice', 100.0, 'g', 2.0, "
        " 130.0, 2.7, 28.0, 0.3, false, 'manual', CURRENT_TIMESTAMP)",
        UUID::generate(), test_user_id_
    );
    txn1.commit();

    pqxx::work txn2(*conn);
    txn2.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, food_name, serving_size, serving_unit, quantity, "
        " calories, protein_g, carbs_g, fat_g, is_custom, source, logged_at) "
        "VALUES ($1, $2, 'Chicken', 100.0, 'g', 1.5, "
        " 165.0, 31.0, 0.0, 3.6, false, 'manual', CURRENT_TIMESTAMP)",
        UUID::generate(), test_user_id_
    );
    txn2.commit();

    std::string today = today_date();

    // Aggregate daily nutrition
    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT "
        "  SUM(calories * quantity) as total_calories, "
        "  SUM(protein_g * quantity) as total_protein_g, "
        "  COUNT(*) as entry_count "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date",
        test_user_id_, today
    );
    txn3.commit();

    ASSERT_EQ(result.size(), 1u);
    // Expected: rice 130*2 + chicken 165*1.5 = 260 + 247.5 = 507.5
    EXPECT_NEAR(result[0]["total_calories"].as<double>(), 507.5, 0.5);
    // Protein: 2.7*2 + 31.0*1.5 = 5.4 + 46.5 = 51.9
    EXPECT_NEAR(result[0]["total_protein_g"].as<double>(), 51.9, 0.5);
    EXPECT_EQ(result[0]["entry_count"].as<int>(), 2);
}

TEST_F(NutritionIntegrationTest, DailySummaryEmptyDayReturnsNulls) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    auto result = txn.exec_params(
        "SELECT "
        "  COALESCE(SUM(calories * quantity), 0) as total_calories, "
        "  COALESCE(SUM(protein_g * quantity), 0) as total_protein_g, "
        "  COUNT(*) as entry_count "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = '2020-01-01'",
        test_user_id_
    );
    txn.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_NEAR(result[0]["total_calories"].as<double>(), 0.0, 0.01);
    EXPECT_NEAR(result[0]["total_protein_g"].as<double>(), 0.0, 0.01);
    EXPECT_EQ(result[0]["entry_count"].as<int>(), 0);
}

// ── Custom foods ────────────────────────────────────────────────────

TEST_F(NutritionIntegrationTest, CreateCustomFood) {
    std::string food_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO custom_foods "
        "(id, user_id, name, serving_size, serving_unit, "
        " calories, protein_g, carbs_g, fat_g, brand) "
        "VALUES ($1, $2, 'My Protein Shake', 330.0, 'ml', "
        " 250.0, 30.0, 15.0, 8.0, 'HomeMade')",
        food_id, test_user_id_
    );
    txn.commit();

    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT * FROM custom_foods WHERE id = $1",
        food_id
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["name"].c_str(), std::string("My Protein Shake"));
    EXPECT_EQ(result[0]["user_id"].c_str(), test_user_id_);
    EXPECT_NEAR(result[0]["calories"].as<double>(), 250.0, 0.01);
}

TEST_F(NutritionIntegrationTest, ListCustomFoods) {
    auto conn = Database::instance().get_connection();

    for (int i = 0; i < 2; ++i) {
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO custom_foods "
            "(id, user_id, name, serving_size, serving_unit, "
            " calories, protein_g, carbs_g, fat_g) "
            "VALUES ($1, $2, $3, 100.0, 'g', $4, 10.0, 10.0, 5.0)",
            UUID::generate(), test_user_id_,
            "Custom Food " + std::to_string(i + 1),
            100.0 + i * 50.0
        );
        txn.commit();
    }

    pqxx::work txn(*conn);
    auto result = txn.exec_params(
        "SELECT * FROM custom_foods WHERE user_id = $1",
        test_user_id_
    );
    txn.commit();

    EXPECT_EQ(result.size(), 2u);
}

TEST_F(NutritionIntegrationTest, DeleteCustomFood) {
    std::string food_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn1(*conn);
    txn1.exec_params(
        "INSERT INTO custom_foods "
        "(id, user_id, name, serving_size, serving_unit, "
        " calories, protein_g, carbs_g, fat_g) "
        "VALUES ($1, $2, 'To Delete', 100.0, 'g', 100.0, 10.0, 10.0, 5.0)",
        food_id, test_user_id_
    );
    txn1.commit();

    pqxx::work txn2(*conn);
    txn2.exec_params(
        "DELETE FROM custom_foods WHERE id = $1 AND user_id = $2",
        food_id, test_user_id_
    );
    txn2.commit();

    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT * FROM custom_foods WHERE user_id = $1",
        test_user_id_
    );
    txn3.commit();

    EXPECT_EQ(result.size(), 0u);
}
