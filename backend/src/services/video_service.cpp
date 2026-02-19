#include "video_service.hpp"
#include <stdexcept>
#include <sstream>

namespace forge {

nlohmann::json ExerciseVideo::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["exercise_id"] = exercise_id;
    j["type"] = video_type;
    j["youtube_video_id"] = youtube_video_id;
    j["youtube_url"] = youtube_url;

    // Create clean embed URL (no YouTube UI chrome)
    j["embed_url"] = "https://www.youtube-nocookie.com/embed/" + youtube_video_id;

    j["title"] = title;
    j["channel"] = channel_name;

    if (channel_id) j["channel_id"] = *channel_id;
    if (duration_seconds) j["duration"] = *duration_seconds;
    if (view_count) j["view_count"] = *view_count;
    if (upload_date) j["upload_date"] = *upload_date;

    j["timestamps"] = timestamps;
    j["is_verified"] = is_verified;
    j["quality_score"] = quality_score;
    j["forge_views"] = view_count_forge;

    return j;
}

ExerciseVideo ExerciseVideo::from_db_row(pqxx::row row) {
    ExerciseVideo video;
    video.id = row["id"].as<std::string>();
    video.exercise_id = row["exercise_id"].as<std::string>();
    video.video_type = row["video_type"].as<std::string>();
    video.youtube_video_id = row["youtube_video_id"].as<std::string>();
    video.youtube_url = row["youtube_url"].as<std::string>();
    video.title = row["title"].as<std::string>();
    video.channel_name = row["channel_name"].as<std::string>();

    if (!row["channel_id"].is_null()) {
        video.channel_id = row["channel_id"].as<std::string>();
    }
    if (!row["duration_seconds"].is_null()) {
        video.duration_seconds = row["duration_seconds"].as<int>();
    }
    if (!row["view_count"].is_null()) {
        video.view_count = row["view_count"].as<long>();
    }
    if (!row["upload_date"].is_null()) {
        video.upload_date = row["upload_date"].as<std::string>();
    }

    // Parse timestamps JSON
    std::string timestamps_str = row["timestamps"].as<std::string>();
    video.timestamps = nlohmann::json::parse(timestamps_str);

    video.is_verified = row["is_verified"].as<bool>();
    video.quality_score = row["quality_score"].as<int>();
    video.view_count_forge = row["view_count_forge"].as<int>();

    return video;
}

std::vector<ExerciseVideo> VideoService::get_exercise_videos(
    const std::string& exercise_id,
    const std::optional<std::string>& video_type
) {
    auto conn = db_.get_connection();
    std::vector<ExerciseVideo> videos;

    std::ostringstream query;
    query << "SELECT * FROM exercise_videos WHERE exercise_id = "
          << conn->quote(exercise_id);

    if (video_type) {
        query << " AND video_type = " << conn->quote(*video_type);
    }

    query << " ORDER BY quality_score DESC, created_at ASC";

    pqxx::work txn(*conn);
    pqxx::result result = txn.exec(query.str());

    for (const auto& row : result) {
        videos.push_back(ExerciseVideo::from_db_row(row));
    }

    return videos;
}

std::optional<ExerciseVideo> VideoService::get_video_by_id(const std::string& video_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT * FROM exercise_videos WHERE id = $1",
        video_id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    return ExerciseVideo::from_db_row(result[0]);
}

void VideoService::track_video_view(
    const std::string& video_id,
    const std::string& user_id,
    int watch_time_seconds,
    bool completed
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Increment view count
    txn.exec_params(
        "UPDATE exercise_videos SET view_count_forge = view_count_forge + 1 "
        "WHERE id = $1",
        video_id
    );

    // TODO: Store detailed analytics in separate table if needed
    // For now, just increment the counter

    txn.commit();
}

bool VideoService::verify_video_exists(const std::string& youtube_video_id) {
    // TODO: Implement YouTube API check
    // For now, just update last_checked timestamp
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "UPDATE exercise_videos SET last_checked = NOW() "
        "WHERE youtube_video_id = $1",
        youtube_video_id
    );

    txn.commit();
    return true; // Assume exists for now
}

std::string VideoService::add_exercise_video(const ExerciseVideo& video) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "INSERT INTO exercise_videos "
        "(exercise_id, video_type, youtube_video_id, youtube_url, title, "
        " channel_name, channel_id, duration_seconds, view_count, upload_date, "
        " timestamps, is_verified, quality_score) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13) "
        "RETURNING id",
        video.exercise_id,
        video.video_type,
        video.youtube_video_id,
        video.youtube_url,
        video.title,
        video.channel_name,
        video.channel_id.value_or(""),
        video.duration_seconds.value_or(0),
        video.view_count.value_or(0),
        video.upload_date.value_or(""),
        video.timestamps.dump(),
        video.is_verified,
        video.quality_score
    );

    txn.commit();
    return result[0]["id"].as<std::string>();
}

bool VideoService::update_video_metadata(
    const std::string& video_id,
    const nlohmann::json& updates
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Build dynamic UPDATE query based on provided fields
    std::ostringstream query;
    query << "UPDATE exercise_videos SET updated_at = NOW()";

    if (updates.contains("title")) {
        query << ", title = " << conn->quote(updates["title"].get<std::string>());
    }
    if (updates.contains("timestamps")) {
        query << ", timestamps = " << conn->quote(updates["timestamps"].dump()) << "::jsonb";
    }
    if (updates.contains("quality_score")) {
        query << ", quality_score = " << updates["quality_score"].get<int>();
    }

    query << " WHERE id = " << conn->quote(video_id);

    pqxx::result result = txn.exec(query.str());
    txn.commit();

    return result.affected_rows() > 0;
}

bool VideoService::delete_video(const std::string& video_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "DELETE FROM exercise_videos WHERE id = $1",
        video_id
    );

    txn.commit();
    return result.affected_rows() > 0;
}

std::vector<ExerciseVideo> VideoService::get_videos_needing_verification() {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec(
        "SELECT * FROM exercise_videos "
        "WHERE last_checked IS NULL "
        "   OR last_checked < NOW() - INTERVAL '30 days' "
        "ORDER BY last_checked ASC NULLS FIRST "
        "LIMIT 100"
    );

    std::vector<ExerciseVideo> videos;
    for (const auto& row : result) {
        videos.push_back(ExerciseVideo::from_db_row(row));
    }

    return videos;
}

} // namespace forge
