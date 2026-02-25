#pragma once

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

namespace forge::test {

// Set environment variables needed for Config singleton and JWT
// Call this BEFORE any code that triggers Config::instance()
inline void setup_test_environment() {
    // Only set if not already set (respect CI/CD environment)
    if (!std::getenv("DATABASE_URL")) {
        setenv("DATABASE_URL", "postgresql://forge:forge@localhost:5432/forge_test", 0);
    }
    if (!std::getenv("JWT_SECRET")) {
        setenv("JWT_SECRET", "test-jwt-secret-that-is-at-least-32-characters-long-for-testing", 0);
    }
}

// Test fixture for integration tests requiring a database connection
class DatabaseTestFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        setup_test_environment();
        // Initialize database once for all tests in this suite
        // Database::instance().initialize(Config::instance().database_url());
    }

    void SetUp() override {
        // Truncate tables between tests for isolation
        // This requires a running PostgreSQL with the forge_test database
        // auto conn = Database::instance().get_connection();
        // pqxx::work txn(*conn);
        // txn.exec("TRUNCATE users, user_profiles, refresh_tokens, "
        //          "workouts, exercise_sets, nutrition_log, custom_foods CASCADE");
        // txn.commit();
    }

    void TearDown() override {
        // Cleanup if needed
    }

    // Helper: create a test user directly via SQL and return user_id
    // std::string create_test_user(const std::string& email = "test@example.com",
    //                              const std::string& username = "testuser",
    //                              const std::string& password = "TestPass123") {
    //     auto [user, tokens] = AuthService::register_user({
    //         .email = email,
    //         .username = username,
    //         .password = password,
    //         .display_name = "Test User"
    //     });
    //     return user.id;
    // }
};

// Test fixture for unit tests that need JWT_SECRET set
class ConfigTestFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        setup_test_environment();
    }
};

} // namespace forge::test
