#include <gtest/gtest.h>
#include "uuid.hpp"
#include <set>
#include <regex>
#include <string>

using forge::UUID;

// ── Format ──────────────────────────────────────────────────────────

TEST(UUIDUnit, GeneratedUUIDIsCorrectLength) {
    std::string uuid = UUID::generate();
    EXPECT_EQ(uuid.length(), 36u) << "UUID should be 36 characters (8-4-4-4-12 + 4 dashes)";
}

TEST(UUIDUnit, GeneratedUUIDHasDashesInCorrectPositions) {
    std::string uuid = UUID::generate();
    EXPECT_EQ(uuid[8], '-');
    EXPECT_EQ(uuid[13], '-');
    EXPECT_EQ(uuid[18], '-');
    EXPECT_EQ(uuid[23], '-');
}

TEST(UUIDUnit, GeneratedUUIDContainsOnlyHexAndDashes) {
    std::string uuid = UUID::generate();
    for (size_t i = 0; i < uuid.length(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            EXPECT_EQ(uuid[i], '-');
        } else {
            char c = uuid[i];
            EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                << "Character at position " << i << " should be hex, got: " << c;
        }
    }
}

TEST(UUIDUnit, GeneratedUUIDMatchesRegex) {
    std::string uuid = UUID::generate();
    std::regex pattern(R"(^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$)");
    EXPECT_TRUE(std::regex_match(uuid, pattern))
        << "UUID should match v4 format: " << uuid;
}

TEST(UUIDUnit, GeneratedUUIDIsVersion4) {
    std::string uuid = UUID::generate();
    // Version 4: the 13th character (index 14) should be '4'
    EXPECT_EQ(uuid[14], '4') << "UUID version nibble should be 4, got: " << uuid;
}

TEST(UUIDUnit, GeneratedUUIDHasCorrectVariant) {
    std::string uuid = UUID::generate();
    // Variant: the 19th character (index 19) should be 8, 9, a, or b
    char variant = uuid[19];
    EXPECT_TRUE(variant == '8' || variant == '9' || variant == 'a' || variant == 'b')
        << "UUID variant nibble should be 8/9/a/b (RFC 4122), got: " << variant
        << " in UUID: " << uuid;
}

// ── Uniqueness ──────────────────────────────────────────────────────

TEST(UUIDUnit, TwoGeneratedUUIDsAreDifferent) {
    std::string uuid1 = UUID::generate();
    std::string uuid2 = UUID::generate();
    EXPECT_NE(uuid1, uuid2);
}

TEST(UUIDUnit, ManyGeneratedUUIDsAreUnique) {
    std::set<std::string> uuids;
    const int count = 1000;

    for (int i = 0; i < count; ++i) {
        std::string uuid = UUID::generate();
        auto [_, inserted] = uuids.insert(uuid);
        EXPECT_TRUE(inserted) << "Duplicate UUID generated at iteration " << i << ": " << uuid;
    }

    EXPECT_EQ(uuids.size(), static_cast<size_t>(count));
}

// ── Validation ──────────────────────────────────────────────────────

TEST(UUIDUnit, IsValidAcceptsGeneratedUUID) {
    std::string uuid = UUID::generate();
    EXPECT_TRUE(UUID::is_valid(uuid));
}

TEST(UUIDUnit, IsValidAcceptsLowercaseUUID) {
    EXPECT_TRUE(UUID::is_valid("550e8400-e29b-41d4-a716-446655440000"));
}

TEST(UUIDUnit, IsValidAcceptsUppercaseUUID) {
    EXPECT_TRUE(UUID::is_valid("550E8400-E29B-41D4-A716-446655440000"));
}

TEST(UUIDUnit, IsValidAcceptsMixedCaseUUID) {
    EXPECT_TRUE(UUID::is_valid("550e8400-E29B-41d4-A716-446655440000"));
}

TEST(UUIDUnit, IsValidRejectsEmptyString) {
    EXPECT_FALSE(UUID::is_valid(""));
}

TEST(UUIDUnit, IsValidRejectsTooShort) {
    EXPECT_FALSE(UUID::is_valid("550e8400-e29b-41d4-a716"));
}

TEST(UUIDUnit, IsValidRejectsTooLong) {
    EXPECT_FALSE(UUID::is_valid("550e8400-e29b-41d4-a716-4466554400001"));
}

TEST(UUIDUnit, IsValidRejectsMissingDashes) {
    EXPECT_FALSE(UUID::is_valid("550e8400e29b41d4a716446655440000"));
}

TEST(UUIDUnit, IsValidRejectsWrongDashPositions) {
    EXPECT_FALSE(UUID::is_valid("550e840-0e29b-41d4-a716-446655440000"));
}

TEST(UUIDUnit, IsValidRejectsNonHexCharacters) {
    EXPECT_FALSE(UUID::is_valid("550g8400-e29b-41d4-a716-446655440000"));
}

TEST(UUIDUnit, IsValidRejectsSpaces) {
    EXPECT_FALSE(UUID::is_valid("550e8400 e29b 41d4 a716 446655440000"));
}
