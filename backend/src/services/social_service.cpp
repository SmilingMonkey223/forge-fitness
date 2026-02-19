#include "social_service.hpp"
#include <stdexcept>
#include <sstream>

namespace forge {

nlohmann::json FeedPost::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["user_id"] = user_id;
    j["username"] = username;
    if (avatar_url) j["avatar_url"] = *avatar_url;
    j["post_type"] = post_type;
    if (workout_id) j["workout_id"] = *workout_id;
    if (title) j["title"] = *title;
    if (description) j["description"] = *description;
    if (image_url) j["image_url"] = *image_url;
    j["visibility"] = visibility;
    j["created_at"] = created_at;
    j["kudos_count"] = kudos_count;
    j["comment_count"] = comment_count;
    j["has_kudos_from_user"] = has_kudos_from_user;
    return j;
}

nlohmann::json Comment::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["post_id"] = post_id;
    j["user_id"] = user_id;
    j["username"] = username;
    if (avatar_url) j["avatar_url"] = *avatar_url;
    j["content"] = content;
    j["created_at"] = created_at;
    return j;
}

nlohmann::json UserProfile::to_json() const {
    nlohmann::json j;
    j["user_id"] = user_id;
    if (bio) j["bio"] = *bio;
    if (avatar_url) j["avatar_url"] = *avatar_url;
    if (location) j["location"] = *location;
    j["profile_visibility"] = profile_visibility;
    j["follower_count"] = follower_count;
    j["following_count"] = following_count;
    j["workout_count"] = workout_count;
    j["total_volume_lifetime"] = total_volume_lifetime;
    j["is_following"] = is_following;
    j["is_follower"] = is_follower;
    return j;
}

nlohmann::json Achievement::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;
    j["icon"] = icon;
    j["category"] = category;
    j["tier"] = tier;
    return j;
}

// Get personalized feed
std::vector<FeedPost> SocialService::get_personalized_feed(
    const std::string& user_id,
    int limit,
    int offset
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Get posts from users the current user follows + their own posts
    std::string query = R"(
        SELECT fp.*, u.username, up.avatar_url,
               (SELECT COUNT(*) FROM kudos k WHERE k.post_id = fp.id) as kudos_count,
               (SELECT COUNT(*) FROM comments c WHERE c.post_id = fp.id) as comment_count,
               EXISTS(SELECT 1 FROM kudos k WHERE k.post_id = fp.id AND k.user_id = $1) as has_kudos
        FROM feed_posts fp
        JOIN users u ON fp.user_id = u.id
        LEFT JOIN user_profiles up ON u.id = up.user_id
        WHERE (
            fp.user_id IN (SELECT following_id FROM follows WHERE follower_id = $1)
            OR fp.user_id = $1
        )
        AND (fp.visibility = 'everyone' OR fp.visibility = 'followers')
        ORDER BY fp.created_at DESC
        LIMIT $2 OFFSET $3
    )";

    pqxx::result result = txn.exec_params(query, user_id, limit, offset);

    std::vector<FeedPost> posts;
    for (const auto& row : result) {
        FeedPost post;
        post.id = row["id"].as<std::string>();
        post.user_id = row["user_id"].as<std::string>();
        post.username = row["username"].as<std::string>();
        if (!row["avatar_url"].is_null()) post.avatar_url = row["avatar_url"].as<std::string>();
        post.post_type = row["post_type"].as<std::string>();
        if (!row["workout_id"].is_null()) post.workout_id = row["workout_id"].as<std::string>();
        if (!row["title"].is_null()) post.title = row["title"].as<std::string>();
        if (!row["description"].is_null()) post.description = row["description"].as<std::string>();
        if (!row["image_url"].is_null()) post.image_url = row["image_url"].as<std::string>();
        post.visibility = row["visibility"].as<std::string>();
        post.created_at = row["created_at"].as<std::string>();
        post.kudos_count = row["kudos_count"].as<int>();
        post.comment_count = row["comment_count"].as<int>();
        post.has_kudos_from_user = row["has_kudos"].as<bool>();
        posts.push_back(post);
    }

    return posts;
}

// Create a feed post
std::string SocialService::create_post(
    const std::string& user_id,
    const std::string& post_type,
    const std::optional<std::string>& workout_id,
    const std::optional<std::string>& title,
    const std::optional<std::string>& description,
    const std::string& visibility
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "INSERT INTO feed_posts (user_id, post_type, workout_id, title, description, visibility) "
        "VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
        user_id,
        post_type,
        workout_id.value_or(""),
        title.value_or(""),
        description.value_or(""),
        visibility
    );

    txn.commit();
    return result[0]["id"].as<std::string>();
}

// Give kudos (permanent)
bool SocialService::give_kudos(const std::string& post_id, const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    try {
        txn.exec_params(
            "INSERT INTO kudos (post_id, user_id) VALUES ($1, $2) ON CONFLICT DO NOTHING",
            post_id,
            user_id
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool SocialService::has_kudos(const std::string& post_id, const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT 1 FROM kudos WHERE post_id = $1 AND user_id = $2",
        post_id,
        user_id
    );

    return !result.empty();
}

// Get comments
std::vector<Comment> SocialService::get_post_comments(const std::string& post_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT c.*, u.username, up.avatar_url "
        "FROM comments c "
        "JOIN users u ON c.user_id = u.id "
        "LEFT JOIN user_profiles up ON u.id = up.user_id "
        "WHERE c.post_id = $1 ORDER BY c.created_at ASC",
        post_id
    );

    std::vector<Comment> comments;
    for (const auto& row : result) {
        Comment comment;
        comment.id = row["id"].as<std::string>();
        comment.post_id = row["post_id"].as<std::string>();
        comment.user_id = row["user_id"].as<std::string>();
        comment.username = row["username"].as<std::string>();
        if (!row["avatar_url"].is_null()) comment.avatar_url = row["avatar_url"].as<std::string>();
        comment.content = row["content"].as<std::string>();
        comment.created_at = row["created_at"].as<std::string>();
        comments.push_back(comment);
    }

    return comments;
}

// Add comment
std::string SocialService::add_comment(
    const std::string& post_id,
    const std::string& user_id,
    const std::string& content
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "INSERT INTO comments (post_id, user_id, content) VALUES ($1, $2, $3) RETURNING id",
        post_id,
        user_id,
        content
    );

    txn.commit();
    return result[0]["id"].as<std::string>();
}

// Follow user
bool SocialService::follow_user(const std::string& follower_id, const std::string& following_id) {
    if (follower_id == following_id) {
        return false; // Can't follow yourself
    }

    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    try {
        txn.exec_params(
            "INSERT INTO follows (follower_id, following_id) VALUES ($1, $2) ON CONFLICT DO NOTHING",
            follower_id,
            following_id
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// Unfollow user
bool SocialService::unfollow_user(const std::string& follower_id, const std::string& following_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "DELETE FROM follows WHERE follower_id = $1 AND following_id = $2",
        follower_id,
        following_id
    );

    txn.commit();
    return result.affected_rows() > 0;
}

// Check if following
bool SocialService::is_following(const std::string& follower_id, const std::string& following_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT 1 FROM follows WHERE follower_id = $1 AND following_id = $2",
        follower_id,
        following_id
    );

    return !result.empty();
}

// Get user profile
std::optional<UserProfile> SocialService::get_user_profile(
    const std::string& user_id,
    const std::optional<std::string>& viewer_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT * FROM user_profiles WHERE user_id = $1",
        user_id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    UserProfile profile;
    auto row = result[0];
    profile.user_id = row["user_id"].as<std::string>();
    if (!row["bio"].is_null()) profile.bio = row["bio"].as<std::string>();
    if (!row["avatar_url"].is_null()) profile.avatar_url = row["avatar_url"].as<std::string>();
    if (!row["location"].is_null()) profile.location = row["location"].as<std::string>();
    profile.profile_visibility = row["profile_visibility"].as<std::string>();
    profile.message_privacy = row["message_privacy"].as<std::string>();
    profile.follower_count = row["follower_count"].as<int>();
    profile.following_count = row["following_count"].as<int>();
    profile.workout_count = row["workout_count"].as<int>();
    profile.total_volume_lifetime = row["total_volume_lifetime"].as<long>();

    // Check relationship with viewer
    if (viewer_id) {
        profile.is_following = is_following(*viewer_id, user_id);
        profile.is_follower = is_following(user_id, *viewer_id);
    } else {
        profile.is_following = false;
        profile.is_follower = false;
    }

    return profile;
}

// Get latest feed (chronological)
std::vector<FeedPost> SocialService::get_latest_feed(
    const std::string& user_id,
    int limit,
    int offset
) {
    // For now, use same as personalized (can add algorithmic ranking later)
    return get_personalized_feed(user_id, limit, offset);
}

// Stub implementations for remaining methods
std::optional<FeedPost> SocialService::get_post_by_id(
    const std::string& post_id,
    const std::optional<std::string>& viewer_id
) {
    // TODO: Implement
    return std::nullopt;
}

bool SocialService::delete_post(const std::string& post_id, const std::string& user_id) {
    // TODO: Implement
    return false;
}

int SocialService::get_kudos_count(const std::string& post_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT COUNT(*) FROM kudos WHERE post_id = $1",
        post_id
    );

    return result[0][0].as<int>();
}

bool SocialService::delete_comment(const std::string& comment_id, const std::string& user_id) {
    // TODO: Implement
    return false;
}

std::vector<UserProfile> SocialService::get_followers(const std::string& user_id, int limit) {
    // TODO: Implement
    return {};
}

std::vector<UserProfile> SocialService::get_following(const std::string& user_id, int limit) {
    // TODO: Implement
    return {};
}

bool SocialService::update_profile(const std::string& user_id, const nlohmann::json& updates) {
    // TODO: Implement
    return false;
}

std::vector<Achievement> SocialService::get_user_achievements(const std::string& user_id) {
    // TODO: Implement
    return {};
}

void SocialService::check_and_award_achievements(const std::string& user_id) {
    // TODO: Implement
}

bool SocialService::can_view_post(const FeedPost& post, const std::string& viewer_id) {
    // TODO: Implement privacy check
    return true;
}

} // namespace forge
