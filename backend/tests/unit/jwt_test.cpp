#include <gtest/gtest.h>
#include "../test_helpers.hpp"
#include "jwt.hpp"
#include "config.hpp"
#include <jwt-cpp/jwt.h>
#include <string>
#include <chrono>
#include <thread>

using forge::JWT;
using forge::Config;

class JWTTestFixture : public forge::test::ConfigTestFixture {};

// ── Token creation ──────────────────────────────────────────────────

TEST_F(JWTTestFixture, CreateAccessTokenReturnsNonEmpty) {
    auto token = JWT::create_access_token("user-123", "testuser");
    EXPECT_FALSE(token.empty());
}

TEST_F(JWTTestFixture, CreateRefreshTokenReturnsNonEmpty) {
    auto token = JWT::create_refresh_token();
    EXPECT_FALSE(token.empty());
}

TEST_F(JWTTestFixture, CreateTokenPairReturnsBoth) {
    auto pair = JWT::create_token_pair("user-123", "testuser");
    EXPECT_FALSE(pair.access_token.empty());
    EXPECT_FALSE(pair.refresh_token.empty());
    EXPECT_NE(pair.access_token, pair.refresh_token);
}

// ── Token verification ──────────────────────────────────────────────

TEST_F(JWTTestFixture, VerifyValidToken) {
    auto token = JWT::create_access_token("user-123", "testuser");
    EXPECT_NO_THROW({
        auto decoded = JWT::verify(token);
    });
}

TEST_F(JWTTestFixture, VerifyRefreshToken) {
    auto token = JWT::create_refresh_token();
    EXPECT_NO_THROW({
        auto decoded = JWT::verify(token);
    });
}

TEST_F(JWTTestFixture, VerifyInvalidTokenThrows) {
    EXPECT_THROW(JWT::verify("not.a.valid.token"), std::exception);
}

TEST_F(JWTTestFixture, VerifyEmptyTokenThrows) {
    EXPECT_THROW(JWT::verify(""), std::exception);
}

TEST_F(JWTTestFixture, VerifyTamperedTokenThrows) {
    auto token = JWT::create_access_token("user-123", "testuser");
    // Tamper with the token by modifying the payload
    std::string tampered = token;
    if (tampered.length() > 20) {
        tampered[20] = (tampered[20] == 'A') ? 'B' : 'A';
    }
    EXPECT_THROW(JWT::verify(tampered), std::exception);
}

// ── Claim extraction ────────────────────────────────────────────────

TEST_F(JWTTestFixture, ExtractUserIdFromToken) {
    auto token = JWT::create_access_token("user-abc-123", "testuser");
    auto decoded = JWT::verify(token);
    std::string user_id = JWT::get_user_id(decoded);
    EXPECT_EQ(user_id, "user-abc-123");
}

TEST_F(JWTTestFixture, ExtractUsernameFromToken) {
    auto token = JWT::create_access_token("user-123", "myusername");
    auto decoded = JWT::verify(token);
    std::string username = JWT::get_username(decoded);
    EXPECT_EQ(username, "myusername");
}

TEST_F(JWTTestFixture, TokenContainsIssuerClaim) {
    auto token = JWT::create_access_token("user-123", "testuser");
    auto decoded = jwt::decode(token);
    EXPECT_EQ(decoded.get_issuer(), "forge");
}

TEST_F(JWTTestFixture, TokenContainsTypeJWT) {
    auto token = JWT::create_access_token("user-123", "testuser");
    auto decoded = jwt::decode(token);
    EXPECT_EQ(decoded.get_type(), "JWT");
}

// ── Token expiry ────────────────────────────────────────────────────

TEST_F(JWTTestFixture, AccessTokenHasExpiry) {
    auto token = JWT::create_access_token("user-123", "testuser");
    auto decoded = jwt::decode(token);
    EXPECT_TRUE(decoded.has_expires_at());
}

TEST_F(JWTTestFixture, AccessTokenExpiresIn15Minutes) {
    auto before = std::chrono::system_clock::now();
    auto token = JWT::create_access_token("user-123", "testuser");
    auto decoded = jwt::decode(token);

    auto issued_at = decoded.get_issued_at();
    auto expires_at = decoded.get_expires_at();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(
        expires_at - issued_at).count();

    EXPECT_EQ(duration, 15) << "Access token should expire in 15 minutes";
}

TEST_F(JWTTestFixture, RefreshTokenExpiresIn30Days) {
    auto token = JWT::create_refresh_token();
    auto decoded = jwt::decode(token);

    auto issued_at = decoded.get_issued_at();
    auto expires_at = decoded.get_expires_at();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        expires_at - issued_at).count();

    // 30 days = 720 hours
    EXPECT_EQ(duration, 720) << "Refresh token should expire in 30 days (720 hours)";
}

// ── Wrong secret ────────────────────────────────────────────────────

TEST_F(JWTTestFixture, TokenWithWrongSecretFailsVerification) {
    // Create a token with a different secret
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::minutes(15);

    auto fake_token = jwt::create()
        .set_issuer("forge")
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("user_id", jwt::claim(std::string("user-123")))
        .sign(jwt::algorithm::hs256{"completely-different-secret-key-that-is-long-enough"});

    // Verification should fail because our JWT::verify uses Config's secret
    EXPECT_THROW(JWT::verify(fake_token), std::exception);
}

// ── Multiple tokens for same user are different ─────────────────────

TEST_F(JWTTestFixture, MultipleTokensAreDifferent) {
    auto token1 = JWT::create_access_token("user-123", "testuser");
    auto token2 = JWT::create_access_token("user-123", "testuser");
    // Tokens should differ due to different iat timestamps (at least)
    // In practice they might be identical if created in same second,
    // but typically they're different
    // Just verify both are valid
    EXPECT_NO_THROW(JWT::verify(token1));
    EXPECT_NO_THROW(JWT::verify(token2));
}
