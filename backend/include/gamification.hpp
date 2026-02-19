#pragma once

#include "database.hpp"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>

namespace forge {

// Badge definition
struct Badge {
    std::string id;
    std::string code;
    std::string name;
    std::string description;
    std::string category;  // workout, nutrition, consistency, milestone
    std::string icon;
    std::string tier;      // bronze, silver, gold, platinum
    nlohmann::json criteria;
};

// User's earned badge
struct UserBadge {
    std::string id;
    std::string badge_id;
    Badge badge;
    std::chrono::system_clock::time_point earned_at;
    nlohmann::json progress_data;
};

// Streak information
struct Streaks {
    // Workout streaks
    int current_workout_streak;
    int best_workout_streak;
    std::optional<std::chrono::sys_days> last_workout_date;

    // Nutrition logging streaks
    int current_logging_streak;
    int best_logging_streak;
    std::optional<std::chrono::sys_days> last_logging_date;

    // Weight tracking streaks
    int current_weight_streak;
    int best_weight_streak;
    std::optional<std::chrono::sys_days> last_weight_date;

    // Streak freezes available
    int workout_freezes_available;
    int logging_freezes_available;
    int weight_freezes_available;
};

/**
 * Gamification service for badges and streaks
 */
class Gamification {
public:
    explicit Gamification(std::shared_ptr<Database> db);
    ~Gamification() = default;

    // Badge operations

    // Get all available badges
    std::vector<Badge> get_all_badges();

    // Get user's earned badges
    std::vector<UserBadge> get_user_badges(const std::string& user_id);

    // Check and award badges after an action
    std::vector<Badge> check_and_award_badges(
        const std::string& user_id,
        const std::string& action_type,  // "workout_completed", "meal_logged", "weight_logged", etc.
        const nlohmann::json& action_data = {}
    );

    // Award a specific badge
    bool award_badge(const std::string& user_id, const std::string& badge_code);

    // Streak operations

    // Get user's current streaks
    std::optional<Streaks> get_streaks(const std::string& user_id);

    // Update workout streak
    void update_workout_streak(const std::string& user_id, const std::chrono::sys_days& date);

    // Update logging streak
    void update_logging_streak(const std::string& user_id, const std::chrono::sys_days& date);

    // Update weight streak
    void update_weight_streak(const std::string& user_id, const std::chrono::sys_days& date);

    // Use a streak freeze
    bool use_streak_freeze(const std::string& user_id, const std::string& streak_type);

private:
    std::shared_ptr<Database> db_;

    // Check if criteria is met for a badge
    bool check_badge_criteria(
        const std::string& user_id,
        const Badge& badge,
        const nlohmann::json& action_data
    );

    // Update streak helper
    void update_streak_internal(
        const std::string& user_id,
        const std::chrono::sys_days& date,
        const std::string& streak_type  // "workout", "logging", "weight"
    );

    // Calculate streak from dates
    int calculate_streak(
        const std::vector<std::chrono::sys_days>& dates,
        const std::chrono::sys_days& current_date
    );
};

} // namespace forge
