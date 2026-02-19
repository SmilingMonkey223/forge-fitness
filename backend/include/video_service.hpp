#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace forge {

struct ExerciseVideo {
    std::string id;
    std::string exercise_id;
    std::string video_type; // primary, alternative, mistakes, advanced
    std::string youtube_video_id;
    std::string youtube_url;
    std::string title;
    std::string channel_name;
    std::optional<std::string> channel_id;
    std::optional<int> duration_seconds;
    std::optional<long> view_count;
    std::optional<std::string> upload_date;
    nlohmann::json timestamps; // {"setup": 0, "execution": 90, ...}
    bool is_verified;
    int quality_score;
    int view_count_forge;

    nlohmann::json to_json() const;
    static ExerciseVideo from_db_row(pqxx::row row);
};

class VideoService {
public:
    explicit VideoService(Database& db) : db_(db) {}

    // Get all videos for an exercise
    std::vector<ExerciseVideo> get_exercise_videos(
        const std::string& exercise_id,
        const std::optional<std::string>& video_type = std::nullopt
    );

    // Get single video by ID
    std::optional<ExerciseVideo> get_video_by_id(const std::string& video_id);

    // Track video view (analytics)
    void track_video_view(
        const std::string& video_id,
        const std::string& user_id,
        int watch_time_seconds,
        bool completed
    );

    // Verify video still exists (for cron job)
    bool verify_video_exists(const std::string& youtube_video_id);

    // Add new video
    std::string add_exercise_video(const ExerciseVideo& video);

    // Update video metadata
    bool update_video_metadata(
        const std::string& video_id,
        const nlohmann::json& updates
    );

    // Delete video
    bool delete_video(const std::string& video_id);

    // Get videos needing verification (last_checked > 30 days ago)
    std::vector<ExerciseVideo> get_videos_needing_verification();

private:
    Database& db_;
};

} // namespace forge
