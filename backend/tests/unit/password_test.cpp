#include <gtest/gtest.h>
#include "crypto.hpp"
#include <chrono>
#include <string>

using forge::Crypto;

// ── Hash password ───────────────────────────────────────────────────

TEST(PasswordHashingUnit, HashReturnsNonEmpty) {
    std::string hash = Crypto::hash_password("test123");
    EXPECT_FALSE(hash.empty());
}

TEST(PasswordHashingUnit, HashHasPBKDF2Prefix) {
    std::string hash = Crypto::hash_password("test123");
    EXPECT_EQ(hash.substr(0, 8), "$pbkdf2$");
}

TEST(PasswordHashingUnit, TwoHashesAreDifferent) {
    // Different salts should produce different hashes
    std::string hash1 = Crypto::hash_password("test123");
    std::string hash2 = Crypto::hash_password("test123");
    EXPECT_NE(hash1, hash2) << "Two hashes of the same password should differ (different salts)";
}

TEST(PasswordHashingUnit, HashContainsCostSaltAndHash) {
    std::string hash = Crypto::hash_password("test123", 10);
    // Format: $pbkdf2$cost$salt_hex$hash_hex
    // Should have 4 '$'-separated sections
    int dollar_count = 0;
    for (char c : hash) {
        if (c == '$') dollar_count++;
    }
    EXPECT_EQ(dollar_count, 4) << "Hash format should be $pbkdf2$cost$salt$hash";
}

// ── Verify password ─────────────────────────────────────────────────

TEST(PasswordHashingUnit, VerifyCorrectPassword) {
    std::string hash = Crypto::hash_password("test123");
    EXPECT_TRUE(Crypto::verify_password("test123", hash));
}

TEST(PasswordHashingUnit, VerifyWrongPasswordFails) {
    std::string hash = Crypto::hash_password("test123");
    EXPECT_FALSE(Crypto::verify_password("wrong", hash));
}

TEST(PasswordHashingUnit, VerifyEmptyPasswordFails) {
    std::string hash = Crypto::hash_password("test123");
    EXPECT_FALSE(Crypto::verify_password("", hash));
}

TEST(PasswordHashingUnit, VerifyInvalidHashFormatFails) {
    EXPECT_FALSE(Crypto::verify_password("test123", "not-a-valid-hash"));
}

TEST(PasswordHashingUnit, VerifyEmptyHashFails) {
    EXPECT_FALSE(Crypto::verify_password("test123", ""));
}

TEST(PasswordHashingUnit, VerifyComplexPassword) {
    std::string complex = "C0mpl3x!P@$$w0rd_With-Special.Chars";
    std::string hash = Crypto::hash_password(complex);
    EXPECT_TRUE(Crypto::verify_password(complex, hash));
}

TEST(PasswordHashingUnit, VerifyUnicodePassword) {
    std::string unicode = "Passwörd123!";
    std::string hash = Crypto::hash_password(unicode);
    EXPECT_TRUE(Crypto::verify_password(unicode, hash));
}

// ── Different cost factors ──────────────────────────────────────────

TEST(PasswordHashingUnit, LowCostFactorWorks) {
    std::string hash = Crypto::hash_password("test123", 8);
    EXPECT_TRUE(Crypto::verify_password("test123", hash));
}

TEST(PasswordHashingUnit, DefaultCostFactorWorks) {
    // Default cost is 12
    std::string hash = Crypto::hash_password("test123");
    EXPECT_TRUE(Crypto::verify_password("test123", hash));
}

// ── Timing ──────────────────────────────────────────────────────────

TEST(PasswordHashingUnit, ConstantTimeComparison) {
    // Both correct and incorrect password verification should take
    // roughly similar time (within 3x) due to constant-time comparison.
    // The actual hashing dominates the time, so the difference should be small.
    std::string hash = Crypto::hash_password("test123", 8); // Low cost for speed

    auto start_correct = std::chrono::high_resolution_clock::now();
    Crypto::verify_password("test123", hash);
    auto end_correct = std::chrono::high_resolution_clock::now();
    auto correct_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_correct - start_correct).count();

    auto start_wrong = std::chrono::high_resolution_clock::now();
    Crypto::verify_password("wrong_password", hash);
    auto end_wrong = std::chrono::high_resolution_clock::now();
    auto wrong_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_wrong - start_wrong).count();

    // Both should take similar time (within 3x) since the PBKDF2 computation
    // dominates and the final comparison is constant-time
    if (correct_duration > 0 && wrong_duration > 0) {
        double ratio = static_cast<double>(std::max(correct_duration, wrong_duration)) /
                       static_cast<double>(std::min(correct_duration, wrong_duration));
        EXPECT_LT(ratio, 3.0) << "Correct and incorrect verification should take similar time. "
                               << "Correct: " << correct_duration << "us, "
                               << "Wrong: " << wrong_duration << "us";
    }
}

// ── SHA-256 ─────────────────────────────────────────────────────────

TEST(CryptoUnit, SHA256ProducesHexString) {
    std::string hash = Crypto::sha256("hello");
    // SHA-256 output is 64 hex characters
    EXPECT_EQ(hash.length(), 64u);

    // All characters should be hex
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            << "SHA-256 should produce lowercase hex, got: " << c;
    }
}

TEST(CryptoUnit, SHA256Deterministic) {
    std::string hash1 = Crypto::sha256("hello");
    std::string hash2 = Crypto::sha256("hello");
    EXPECT_EQ(hash1, hash2);
}

TEST(CryptoUnit, SHA256DifferentInputsDifferentOutputs) {
    std::string hash1 = Crypto::sha256("hello");
    std::string hash2 = Crypto::sha256("world");
    EXPECT_NE(hash1, hash2);
}

TEST(CryptoUnit, SHA256KnownValue) {
    // SHA-256("hello") = 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824
    std::string hash = Crypto::sha256("hello");
    EXPECT_EQ(hash, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

// ── Random token ────────────────────────────────────────────────────

TEST(CryptoUnit, RandomTokenNonEmpty) {
    std::string token = Crypto::random_token();
    EXPECT_FALSE(token.empty());
}

TEST(CryptoUnit, RandomTokenCorrectLength) {
    // 32 bytes → 64 hex characters
    std::string token = Crypto::random_token(32);
    EXPECT_EQ(token.length(), 64u);

    // 16 bytes → 32 hex characters
    std::string token16 = Crypto::random_token(16);
    EXPECT_EQ(token16.length(), 32u);
}

TEST(CryptoUnit, RandomTokensAreDifferent) {
    std::string token1 = Crypto::random_token();
    std::string token2 = Crypto::random_token();
    EXPECT_NE(token1, token2);
}
