#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "auth_service.hpp"
#include "database.hpp"
#include "config.hpp"
#include <string>
#include <stdexcept>

using forge::AuthService;
using forge::Database;
using forge::Config;

class AuthIntegrationTest : public ::testing::Test {
protected:
    static bool db_initialized_;

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

        // Clean up test data before each test
        try {
            auto conn = Database::instance().get_connection();
            pqxx::work txn(*conn);
            txn.exec(
                "DELETE FROM refresh_tokens WHERE user_id IN "
                "(SELECT id FROM users WHERE email LIKE '%@test-forge.example.com');"
                "DELETE FROM user_profiles WHERE user_id IN "
                "(SELECT id FROM users WHERE email LIKE '%@test-forge.example.com');"
                "DELETE FROM users WHERE email LIKE '%@test-forge.example.com';"
            );
            txn.commit();
        } catch (const std::exception& e) {
            GTEST_SKIP() << "Failed to clean test data: " << e.what();
        }
    }

    // Helper to generate unique test emails
    static std::string unique_email(const std::string& prefix = "user") {
        static int counter = 0;
        return prefix + std::to_string(++counter) + "@test-forge.example.com";
    }
};

bool AuthIntegrationTest::db_initialized_ = false;

// ── Register ────────────────────────────────────────────────────────

TEST_F(AuthIntegrationTest, RegisterNewUserReturnsUserAndTokens) {
    std::string email = unique_email("register");
    auto [user, tokens] = AuthService::register_user({
        .email = email,
        .username = "regtest1",
        .password = "TestPass123",
        .display_name = "Register Test"
    });

    EXPECT_FALSE(user.id.empty()) << "User should have an ID";
    EXPECT_EQ(user.email, email);
    EXPECT_EQ(user.username, "regtest1");
    EXPECT_EQ(user.display_name, "Register Test");
    EXPECT_FALSE(tokens.access_token.empty());
    EXPECT_FALSE(tokens.refresh_token.empty());
}

TEST_F(AuthIntegrationTest, RegisterDuplicateEmailThrows) {
    std::string email = unique_email("dupemail");
    AuthService::register_user({
        .email = email,
        .username = "dupemail1",
        .password = "TestPass123",
        .display_name = "First User"
    });

    EXPECT_THROW({
        AuthService::register_user({
            .email = email,
            .username = "dupemail2",
            .password = "TestPass123",
            .display_name = "Second User"
        });
    }, std::invalid_argument);

    try {
        AuthService::register_user({
            .email = email,
            .username = "dupemail2",
            .password = "TestPass123",
            .display_name = "Second User"
        });
        FAIL() << "Expected EMAIL_TAKEN exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "EMAIL_TAKEN");
    }
}

TEST_F(AuthIntegrationTest, RegisterDuplicateUsernameThrows) {
    std::string email1 = unique_email("dupuser1");
    std::string email2 = unique_email("dupuser2");

    AuthService::register_user({
        .email = email1,
        .username = "samename",
        .password = "TestPass123",
        .display_name = "First"
    });

    try {
        AuthService::register_user({
            .email = email2,
            .username = "samename",
            .password = "TestPass123",
            .display_name = "Second"
        });
        FAIL() << "Expected USERNAME_TAKEN exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "USERNAME_TAKEN");
    }
}

TEST_F(AuthIntegrationTest, RegisterWeakPasswordThrows) {
    try {
        AuthService::register_user({
            .email = unique_email("weakpw"),
            .username = "weakpwuser",
            .password = "weak",
            .display_name = "Weak"
        });
        FAIL() << "Expected WEAK_PASSWORD exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "WEAK_PASSWORD");
    }
}

TEST_F(AuthIntegrationTest, RegisterInvalidEmailThrows) {
    try {
        AuthService::register_user({
            .email = "not-an-email",
            .username = "bademail",
            .password = "TestPass123",
            .display_name = "Bad Email"
        });
        FAIL() << "Expected INVALID_EMAIL exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "INVALID_EMAIL");
    }
}

// ── Login ───────────────────────────────────────────────────────────

TEST_F(AuthIntegrationTest, LoginWithValidCredentials) {
    std::string email = unique_email("login");
    AuthService::register_user({
        .email = email,
        .username = "loginuser",
        .password = "TestPass123",
        .display_name = "Login Test"
    });

    auto [user, tokens] = AuthService::login({
        .email = email,
        .password = "TestPass123"
    });

    EXPECT_EQ(user.email, email);
    EXPECT_EQ(user.username, "loginuser");
    EXPECT_FALSE(tokens.access_token.empty());
    EXPECT_FALSE(tokens.refresh_token.empty());
}

TEST_F(AuthIntegrationTest, LoginWithWrongPasswordThrows) {
    std::string email = unique_email("wrongpw");
    AuthService::register_user({
        .email = email,
        .username = "wrongpwuser",
        .password = "TestPass123",
        .display_name = "Wrong PW"
    });

    try {
        AuthService::login({
            .email = email,
            .password = "WrongPassword1"
        });
        FAIL() << "Expected INVALID_CREDENTIALS exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "INVALID_CREDENTIALS");
    }
}

TEST_F(AuthIntegrationTest, LoginWithNonexistentEmailThrows) {
    try {
        AuthService::login({
            .email = "nobody@test-forge.example.com",
            .password = "TestPass123"
        });
        FAIL() << "Expected INVALID_CREDENTIALS exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "INVALID_CREDENTIALS");
    }
}

// ── Token refresh ───────────────────────────────────────────────────

TEST_F(AuthIntegrationTest, RefreshTokenReturnsNewPair) {
    std::string email = unique_email("refresh");
    auto [user, tokens] = AuthService::register_user({
        .email = email,
        .username = "refreshuser",
        .password = "TestPass123",
        .display_name = "Refresh Test"
    });

    auto new_tokens = AuthService::refresh(tokens.refresh_token);

    EXPECT_FALSE(new_tokens.access_token.empty());
    EXPECT_FALSE(new_tokens.refresh_token.empty());
    // New tokens should be different from old ones
    EXPECT_NE(new_tokens.refresh_token, tokens.refresh_token);
}

// ── User JSON serialization (security) ──────────────────────────────

TEST_F(AuthIntegrationTest, RegisteredUserJsonExcludesPassword) {
    std::string email = unique_email("nopw");
    auto [user, tokens] = AuthService::register_user({
        .email = email,
        .username = "nopwuser",
        .password = "TestPass123",
        .display_name = "No PW"
    });

    auto j = user.to_json();
    EXPECT_FALSE(j.contains("password_hash"));
    EXPECT_FALSE(j.contains("password"));
    EXPECT_TRUE(j.contains("id"));
    EXPECT_TRUE(j.contains("email"));
}
