#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace forge {

struct Program {
    std::string id;
    std::string name;
    std::string description;
    std::optional<std::string> author_id;
    std::string goal;
    std::string experience_level;
    int duration_weeks;
    int workouts_per_week;
    bool is_official;
    bool is_public;

    nlohmann::json to_json() const;
    static Program from_db_row(pqxx::row row);
};

struct ProgramWeek {
    std::string id;
    std::string program_id;
    int week_number;
    bool deload_week;
    double intensity_multiplier;
    double volume_multiplier;
    std::optional<std::string> notes;

    nlohmann::json to_json() const;
};

struct ProgramWorkout {
    std::string id;
    std::string program_id;
    int week_number;
    int day_number;
    std::string name;
    std::string description;
    std::optional<int> estimated_duration_minutes;

    nlohmann::json to_json() const;
};

struct ProgramExercise {
    std::string id;
    std::string program_workout_id;
    std::string exercise_id;
    int exercise_order;
    int target_sets;
    std::optional<int> target_reps_min;
    std::optional<int> target_reps_max;
    std::optional<int> target_rir;
    int target_rest_seconds;
    bool is_superset;
    std::optional<int> superset_group;
    std::string progression_scheme;
    std::optional<std::string> notes;

    nlohmann::json to_json() const;
};

struct UserCoachingState {
    std::string user_id;
    std::string mode; // 'ai_coaching' or 'tracker'
    std::optional<std::string> current_program_id;
    int current_week;
    std::optional<std::string> program_start_date;
    std::optional<std::string> last_check_in;
    std::optional<double> tdee_adaptive;
    std::optional<std::string> energy_balance;

    nlohmann::json to_json() const;
};

struct WeeklyCheckIn {
    std::string id;
    std::string user_id;
    std::string week_start;
    std::string week_end;
    std::optional<int> hunger_rating;
    std::optional<int> energy_rating;
    std::optional<int> recovery_rating;
    std::optional<int> sleep_quality_rating;
    std::optional<double> average_weight_lbs;
    std::optional<double> weight_change_lbs;
    int total_workouts_completed;
    int total_sets_completed;
    std::optional<std::string> ai_recommendation;
    std::optional<std::string> adjustment_made;
    std::optional<std::string> notes;

    nlohmann::json to_json() const;
};

struct CoachingQuestionnaire {
    std::string id;
    std::string user_id;
    std::string experience_level;
    double years_training;
    int current_frequency;
    std::string primary_goal;
    std::optional<std::string> secondary_goal;
    std::optional<int> goal_timeframe_weeks;
    int available_days_per_week;
    int session_duration_minutes;
    nlohmann::json available_equipment;
    std::optional<std::string> injuries_or_limitations;
    nlohmann::json exercises_to_avoid;
    std::optional<std::string> preferred_rep_range;
    std::optional<std::string> preferred_intensity;

    nlohmann::json to_json() const;
};

class ProgramService {
public:
    explicit ProgramService(Database& db) : db_(db) {}

    // Program CRUD
    std::vector<Program> get_all_programs(
        const std::optional<std::string>& goal = std::nullopt,
        const std::optional<std::string>& experience_level = std::nullopt,
        bool official_only = false
    );
    std::optional<Program> get_program_by_id(const std::string& program_id);
    std::string create_program(const Program& program);
    bool update_program(const std::string& program_id, const nlohmann::json& updates);
    bool delete_program(const std::string& program_id);

    // Program structure
    std::vector<ProgramWorkout> get_program_workouts(const std::string& program_id, int week_number);
    std::vector<ProgramExercise> get_workout_exercises(const std::string& workout_id);

    // User coaching state
    std::optional<UserCoachingState> get_coaching_state(const std::string& user_id);
    void set_coaching_mode(const std::string& user_id, const std::string& mode);
    void start_program(const std::string& user_id, const std::string& program_id);
    void advance_week(const std::string& user_id);

    // Weekly check-ins
    std::string submit_checkin(const WeeklyCheckIn& checkin);
    std::vector<WeeklyCheckIn> get_user_checkins(const std::string& user_id, int limit = 12);

    // AI coaching questionnaire
    std::string save_questionnaire(const CoachingQuestionnaire& questionnaire);
    std::optional<CoachingQuestionnaire> get_questionnaire(const std::string& user_id);

    // AI program generation (simplified rule-based for now)
    std::string generate_program_from_questionnaire(const std::string& user_id);

private:
    Database& db_;

    // Helper: Select best official program based on questionnaire
    std::optional<std::string> select_best_program(const CoachingQuestionnaire& q);
};

} // namespace forge
