#include <gtest/gtest.h>
#include "auth_service.hpp"
#include <string>

using forge::AuthService;

// ── Email validation ────────────────────────────────────────────────

TEST(EmailValidationUnit, ValidStandardEmail) {
    EXPECT_TRUE(AuthService::validate_email("user@example.com"));
}

TEST(EmailValidationUnit, ValidEmailWithSubdomain) {
    EXPECT_TRUE(AuthService::validate_email("user@mail.example.com"));
}

TEST(EmailValidationUnit, ValidEmailWithPlus) {
    EXPECT_TRUE(AuthService::validate_email("user+tag@example.com"));
}

TEST(EmailValidationUnit, ValidEmailWithDots) {
    EXPECT_TRUE(AuthService::validate_email("first.last@example.com"));
}

TEST(EmailValidationUnit, ValidEmailWithNumbers) {
    EXPECT_TRUE(AuthService::validate_email("user123@example.com"));
}

TEST(EmailValidationUnit, ValidEmailWithPercent) {
    EXPECT_TRUE(AuthService::validate_email("user%tag@example.com"));
}

TEST(EmailValidationUnit, InvalidEmpty) {
    EXPECT_FALSE(AuthService::validate_email(""));
}

TEST(EmailValidationUnit, InvalidNoAtSign) {
    EXPECT_FALSE(AuthService::validate_email("notanemail"));
}

TEST(EmailValidationUnit, InvalidNoTLD) {
    EXPECT_FALSE(AuthService::validate_email("user@example"));
}

TEST(EmailValidationUnit, InvalidMultipleAtSigns) {
    EXPECT_FALSE(AuthService::validate_email("user@@example.com"));
}

TEST(EmailValidationUnit, InvalidSpaces) {
    EXPECT_FALSE(AuthService::validate_email("user @example.com"));
}

TEST(EmailValidationUnit, InvalidSingleCharTLD) {
    EXPECT_FALSE(AuthService::validate_email("user@example.c"));
}

TEST(EmailValidationUnit, InvalidNoLocalPart) {
    EXPECT_FALSE(AuthService::validate_email("@example.com"));
}

// ── Username validation ─────────────────────────────────────────────

TEST(UsernameValidationUnit, ValidAlphanumeric) {
    EXPECT_TRUE(AuthService::validate_username("testuser"));
}

TEST(UsernameValidationUnit, ValidWithUnderscore) {
    EXPECT_TRUE(AuthService::validate_username("test_user"));
}

TEST(UsernameValidationUnit, ValidWithNumbers) {
    EXPECT_TRUE(AuthService::validate_username("user123"));
}

TEST(UsernameValidationUnit, ValidMinLength3Chars) {
    EXPECT_TRUE(AuthService::validate_username("abc"));
}

TEST(UsernameValidationUnit, ValidMaxLength24Chars) {
    EXPECT_TRUE(AuthService::validate_username("abcdefghijklmnopqrstuvwx")); // 24 chars
}

TEST(UsernameValidationUnit, InvalidTooShort) {
    EXPECT_FALSE(AuthService::validate_username("ab")); // 2 chars
}

TEST(UsernameValidationUnit, InvalidTooLong) {
    EXPECT_FALSE(AuthService::validate_username("abcdefghijklmnopqrstuvwxy")); // 25 chars
}

TEST(UsernameValidationUnit, InvalidEmpty) {
    EXPECT_FALSE(AuthService::validate_username(""));
}

TEST(UsernameValidationUnit, InvalidWithSpaces) {
    EXPECT_FALSE(AuthService::validate_username("test user"));
}

TEST(UsernameValidationUnit, InvalidWithSpecialChars) {
    EXPECT_FALSE(AuthService::validate_username("test@user"));
    EXPECT_FALSE(AuthService::validate_username("test-user"));
    EXPECT_FALSE(AuthService::validate_username("test.user"));
    EXPECT_FALSE(AuthService::validate_username("test!user"));
}

TEST(UsernameValidationUnit, ValidAllUnderscores) {
    EXPECT_TRUE(AuthService::validate_username("___"));
}

TEST(UsernameValidationUnit, ValidAllDigits) {
    EXPECT_TRUE(AuthService::validate_username("123"));
}

// ── Password validation ─────────────────────────────────────────────

TEST(PasswordValidationUnit, ValidStrongPassword) {
    EXPECT_TRUE(AuthService::validate_password("TestPass1"));
}

TEST(PasswordValidationUnit, ValidComplexPassword) {
    EXPECT_TRUE(AuthService::validate_password("MyStr0ngP@ss!"));
}

TEST(PasswordValidationUnit, ValidExactly8Chars) {
    EXPECT_TRUE(AuthService::validate_password("Abcdef1g"));
}

TEST(PasswordValidationUnit, InvalidTooShort) {
    EXPECT_FALSE(AuthService::validate_password("Test1")); // < 8 chars
}

TEST(PasswordValidationUnit, InvalidTooShort7Chars) {
    EXPECT_FALSE(AuthService::validate_password("Test12a")); // 7 chars
}

TEST(PasswordValidationUnit, InvalidEmpty) {
    EXPECT_FALSE(AuthService::validate_password(""));
}

TEST(PasswordValidationUnit, InvalidNoUppercase) {
    EXPECT_FALSE(AuthService::validate_password("testpass1"));
}

TEST(PasswordValidationUnit, InvalidNoLowercase) {
    EXPECT_FALSE(AuthService::validate_password("TESTPASS1"));
}

TEST(PasswordValidationUnit, InvalidNoDigit) {
    EXPECT_FALSE(AuthService::validate_password("TestPasss"));
}

TEST(PasswordValidationUnit, InvalidAllLowercase) {
    EXPECT_FALSE(AuthService::validate_password("testpassword"));
}

TEST(PasswordValidationUnit, InvalidAllUppercase) {
    EXPECT_FALSE(AuthService::validate_password("TESTPASSWORD"));
}

TEST(PasswordValidationUnit, InvalidAllDigits) {
    EXPECT_FALSE(AuthService::validate_password("12345678"));
}

TEST(PasswordValidationUnit, ValidVeryLong) {
    // 128 chars should be accepted (max)
    std::string long_pass(126, 'a');
    long_pass += "A1"; // Add uppercase and digit
    EXPECT_TRUE(AuthService::validate_password(long_pass));
}

TEST(PasswordValidationUnit, InvalidTooLong) {
    // 129 chars should be rejected
    std::string too_long(127, 'a');
    too_long += "A1"; // 129 total
    EXPECT_FALSE(AuthService::validate_password(too_long));
}

// ── Date format validation (regex pattern from NutritionService) ────

namespace {
bool is_valid_date(const std::string& date) {
    // Replicates NutritionService::is_valid_date (private method)
    static const std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    return std::regex_match(date, pattern);
}
} // anonymous namespace

TEST(DateValidationUnit, ValidDate) {
    EXPECT_TRUE(is_valid_date("2026-02-25"));
}

TEST(DateValidationUnit, ValidDateStartOfYear) {
    EXPECT_TRUE(is_valid_date("2026-01-01"));
}

TEST(DateValidationUnit, ValidDateEndOfYear) {
    EXPECT_TRUE(is_valid_date("2026-12-31"));
}

TEST(DateValidationUnit, InvalidEmpty) {
    EXPECT_FALSE(is_valid_date(""));
}

TEST(DateValidationUnit, InvalidWrongFormat) {
    EXPECT_FALSE(is_valid_date("25-02-2026"));  // DD-MM-YYYY
    EXPECT_FALSE(is_valid_date("02/25/2026"));  // MM/DD/YYYY
    EXPECT_FALSE(is_valid_date("2026/02/25"));  // YYYY/MM/DD
}

TEST(DateValidationUnit, InvalidPartialDate) {
    EXPECT_FALSE(is_valid_date("2026-02"));
    EXPECT_FALSE(is_valid_date("2026"));
}

TEST(DateValidationUnit, InvalidWithTime) {
    EXPECT_FALSE(is_valid_date("2026-02-25T12:00:00"));
}

TEST(DateValidationUnit, InvalidExtraCharacters) {
    EXPECT_FALSE(is_valid_date("2026-02-250"));
    EXPECT_FALSE(is_valid_date(" 2026-02-25"));
}

// ── Meal type validation (from NutritionService) ────────────────────

namespace {
bool is_valid_meal_type(const std::string& meal_type) {
    // Replicates NutritionService::is_valid_meal_type (private method)
    return meal_type == "breakfast" || meal_type == "lunch" ||
           meal_type == "dinner"   || meal_type == "snack";
}
} // anonymous namespace

TEST(MealTypeValidationUnit, ValidBreakfast) {
    EXPECT_TRUE(is_valid_meal_type("breakfast"));
}

TEST(MealTypeValidationUnit, ValidLunch) {
    EXPECT_TRUE(is_valid_meal_type("lunch"));
}

TEST(MealTypeValidationUnit, ValidDinner) {
    EXPECT_TRUE(is_valid_meal_type("dinner"));
}

TEST(MealTypeValidationUnit, ValidSnack) {
    EXPECT_TRUE(is_valid_meal_type("snack"));
}

TEST(MealTypeValidationUnit, InvalidEmpty) {
    EXPECT_FALSE(is_valid_meal_type(""));
}

TEST(MealTypeValidationUnit, InvalidBrunch) {
    EXPECT_FALSE(is_valid_meal_type("brunch"));
}

TEST(MealTypeValidationUnit, InvalidUppercase) {
    EXPECT_FALSE(is_valid_meal_type("Breakfast"));
    EXPECT_FALSE(is_valid_meal_type("LUNCH"));
}

TEST(MealTypeValidationUnit, InvalidRandom) {
    EXPECT_FALSE(is_valid_meal_type("midnight_snack"));
}
