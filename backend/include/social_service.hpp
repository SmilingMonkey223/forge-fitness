#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace forge {

struct FeedPost {
    std::string id;
    std::string user_id;
    std::string post_type;
    std::optional<std::string> workout_id;
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<std::string> image_url;
    std::string visibility;
    std::string created_at;

    // Aggregated data (from joins)
    std::string username;
    std::optional<std::string> avatar_url;
    int kudos_count;
    int comment_count;
    bool has_kudos_from_user; // Whether current user has given kudos

    nlohmann::json to_json() const;
};

struct Comment {
    std::string id;
    std::string post_id;
    std::string user_id;
    std::string content;
    std::string created_at;

    // User info
    std::string username;
    std::optional<std::string> avatar_url;

    nlohmann::json to_json() const;
};

struct UserProfile {
    std::string user_id;
    std::optional<std::string> bio;
    std::optional<std::string> avatar_url;
    std::optional<std::string> location;
    std::string profile_visibility;
    std::string message_privacy;
    int follower_count;
    int following_count;
    int workout_count;
    long total_volume_lifetime;

    // Relationship with current user
    bool is_following;
    bool is_follower;

    nlohmann::json to_json() const;
};

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    std::string icon;
    std::string category;
    std::string tier;

    nlohmann::json to_json() const;
};

class SocialService {
public:
    explicit SocialService(Database& db) : db_(db) {}

    // Feed management
    std::vector<FeedPost> get_personalized_feed(
        const std::string& user_id,
        int limit = 20,
        int offset = 0
    );

    std::vector<FeedPost> get_latest_feed(
        const std::string& user_id,
        int limit = 20,
        int offset = 0
    );

    std::optional<FeedPost> get_post_by_id(
        const std::string& post_id,
        const std::optional<std::string>& viewer_id = std::nullopt
    );

    std::string create_post(
        const std::string& user_id,
        const std::string& post_type,
        const std::optional<std::string>& workout_id,
        const std::optional<std::string>& title,
        const std::optional<std::string>& description,
        const std::string& visibility = "everyone"
    );

    bool delete_post(const std::string& post_id, const std::string& user_id);

    // Kudos system (permanent, cannot undo)
    bool give_kudos(const std::string& post_id, const std::string& user_id);
    bool has_kudos(const std::string& post_id, const std::string& user_id);
    int get_kudos_count(const std::string& post_id);

    // Comments
    std::vector<Comment> get_post_comments(const std::string& post_id);
    std::string add_comment(
        const std::string& post_id,
        const std::string& user_id,
        const std::string& content
    );
    bool delete_comment(const std::string& comment_id, const std::string& user_id);

    // Follow system
    bool follow_user(const std::string& follower_id, const std::string& following_id);
    bool unfollow_user(const std::string& follower_id, const std::string& following_id);
    bool is_following(const std::string& follower_id, const std::string& following_id);
    std::vector<UserProfile> get_followers(const std::string& user_id, int limit = 50);
    std::vector<UserProfile> get_following(const std::string& user_id, int limit = 50);

    // User profiles
    std::optional<UserProfile> get_user_profile(
        const std::string& user_id,
        const std::optional<std::string>& viewer_id = std::nullopt
    );
    bool update_profile(
        const std::string& user_id,
        const nlohmann::json& updates
    );

    // Achievements
    std::vector<Achievement> get_user_achievements(const std::string& user_id);
    void check_and_award_achievements(const std::string& user_id);

private:
    Database& db_;

    // Helper: Check if viewer has access to post/profile
    bool can_view_post(const FeedPost& post, const std::string& viewer_id);
};

} // namespace forge
