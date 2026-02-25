#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "auth_service.hpp"
#include "database.hpp"
#include "config.hpp"
#include "uuid.hpp"
#include "models.hpp"
#include <string>
#include <optional>

// These tests verify the database schema and profile-related queries directly via SQL.
// ProfileService (profile_service.hpp/cpp) exists on main and handles onboarding/updates,
// but here we test the schema independently to ensure correctness.

using forge::Database;
using forge::Config;
using forge::AuthService;
using forge::UUID;
using json = nlohmann::json;

class ProfileIntegrationTest : public ::testing::Test {
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

        // Create a test user for profile tests
        try {
            static int counter = 0;
            std::string email = "profile" + std::to_string(++counter) + "@test-forge.example.com";
            auto [user, tokens] = AuthService::register_user({
                .email = email,
                .username = "profuser" + std::to_string(counter),
                .password = "TestPass123",
                .display_name = "Profile Test"
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
                    "DELETE FROM user_profiles WHERE user_id = $1;"
                    "DELETE FROM refresh_tokens WHERE user_id = $1;"
                    "DELETE FROM users WHERE id = $1;",
                    test_user_id_
                );
                txn.commit();
            } catch (...) {}
        }
    }
};

bool ProfileIntegrationTest::db_initialized_ = false;

// ── Profile does not exist initially ────────────────────────────────

TEST_F(ProfileIntegrationTest, NoProfileForNewUser) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT * FROM user_profiles WHERE user_id = $1",
        test_user_id_
    );
    txn.commit();

    EXPECT_TRUE(result.empty()) << "New user should not have a profile";
}

// ── Create profile (simulates onboarding) ───────────────────────────

TEST_F(ProfileIntegrationTest, CreateProfileViaSQL) {
    std::string profile_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "INSERT INTO user_profiles "
        "(id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        " activity_level, fitness_goal, unit_preference, "
        " tdee_calories, target_calories, target_protein_g, target_carbs_g, target_fat_g) "
        "VALUES ($1, $2, '1995-06-15', 'male', 180.0, 80.0, "
        " 'moderately_active', 'build_muscle', 'metric', "
        " 2759.0, 3059.0, 160.0, 442.75, 72.0)",
        profile_id, test_user_id_
    );
    txn.commit();

    // Verify profile was created
    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT * FROM user_profiles WHERE user_id = $1",
        test_user_id_
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]["sex"].c_str(), std::string("male"));
    EXPECT_NEAR(result[0]["height_cm"].as<double>(), 180.0, 0.01);
    EXPECT_NEAR(result[0]["weight_kg"].as<double>(), 80.0, 0.01);
    EXPECT_EQ(result[0]["fitness_goal"].c_str(), std::string("build_muscle"));
}

// ── Read profile back ───────────────────────────────────────────────

TEST_F(ProfileIntegrationTest, ReadProfileAfterCreation) {
    std::string profile_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO user_profiles "
        "(id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        " activity_level, fitness_goal, unit_preference) "
        "VALUES ($1, $2, '1995-06-15', 'female', 165.0, 60.0, "
        " 'lightly_active', 'lose_fat', 'metric')",
        profile_id, test_user_id_
    );
    txn.commit();

    // Read it back
    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        "activity_level, fitness_goal, unit_preference, "
        "tdee_calories, target_calories, target_protein_g, target_carbs_g, target_fat_g "
        "FROM user_profiles WHERE user_id = $1",
        test_user_id_
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    auto row = result[0];

    // Map to UserProfile struct
    forge::UserProfile profile;
    profile.id = row["id"].c_str();
    profile.user_id = row["user_id"].c_str();
    profile.sex = row["sex"].c_str();
    profile.height_cm = row["height_cm"].as<double>();
    profile.weight_kg = row["weight_kg"].as<double>();

    EXPECT_EQ(profile.user_id, test_user_id_);
    EXPECT_EQ(profile.sex, "female");
    EXPECT_NEAR(profile.height_cm, 165.0, 0.01);
    EXPECT_NEAR(profile.weight_kg, 60.0, 0.01);
}

// ── Update profile ──────────────────────────────────────────────────

TEST_F(ProfileIntegrationTest, UpdateProfileWeight) {
    std::string profile_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO user_profiles "
        "(id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        " activity_level, fitness_goal, unit_preference) "
        "VALUES ($1, $2, '1995-06-15', 'male', 180.0, 80.0, "
        " 'moderately_active', 'build_muscle', 'metric')",
        profile_id, test_user_id_
    );
    txn.commit();

    // Update weight
    pqxx::work txn2(*conn);
    txn2.exec_params(
        "UPDATE user_profiles SET weight_kg = 82.5 WHERE user_id = $1",
        test_user_id_
    );
    txn2.commit();

    // Verify update
    pqxx::work txn3(*conn);
    auto result = txn3.exec_params(
        "SELECT weight_kg FROM user_profiles WHERE user_id = $1",
        test_user_id_
    );
    txn3.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_NEAR(result[0]["weight_kg"].as<double>(), 82.5, 0.01);
}

// ── Profile target calculation consistency ──────────────────────────

TEST_F(ProfileIntegrationTest, TDEEAndMacroTargetsStored) {
    std::string profile_id = UUID::generate();

    double tdee = 2759.0;
    double target_cal = tdee + 300; // build_muscle
    double protein = 80.0 * 2.0;    // 160g
    double fat = 80.0 * 0.9;        // 72g
    double carbs = (target_cal - protein * 4 - fat * 9) / 4.0;

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO user_profiles "
        "(id, user_id, date_of_birth, sex, height_cm, weight_kg, "
        " activity_level, fitness_goal, unit_preference, "
        " tdee_calories, target_calories, target_protein_g, target_carbs_g, target_fat_g) "
        "VALUES ($1, $2, '1995-06-15', 'male', 180.0, 80.0, "
        " 'moderately_active', 'build_muscle', 'metric', "
        " $3, $4, $5, $6, $7)",
        profile_id, test_user_id_,
        tdee, target_cal, protein, carbs, fat
    );
    txn.commit();

    // Verify stored values
    pqxx::work txn2(*conn);
    auto result = txn2.exec_params(
        "SELECT tdee_calories, target_calories, target_protein_g, target_carbs_g, target_fat_g "
        "FROM user_profiles WHERE user_id = $1",
        test_user_id_
    );
    txn2.commit();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_NEAR(result[0]["tdee_calories"].as<double>(), tdee, 0.1);
    EXPECT_NEAR(result[0]["target_calories"].as<double>(), target_cal, 0.1);

    // Verify macros sum to target calories
    double stored_protein = result[0]["target_protein_g"].as<double>();
    double stored_carbs = result[0]["target_carbs_g"].as<double>();
    double stored_fat = result[0]["target_fat_g"].as<double>();
    double total_cal = stored_protein * 4 + stored_carbs * 4 + stored_fat * 9;
    EXPECT_NEAR(total_cal, target_cal, 1.0);
}
