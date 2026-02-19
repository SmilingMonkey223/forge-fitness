#include "program_service.hpp"
#include <stdexcept>
#include <sstream>

namespace forge {

// Program JSON serialization
nlohmann::json Program::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;
    if (author_id) j["author_id"] = *author_id;
    j["goal"] = goal;
    j["experience_level"] = experience_level;
    j["duration_weeks"] = duration_weeks;
    j["workouts_per_week"] = workouts_per_week;
    j["is_official"] = is_official;
    j["is_public"] = is_public;
    return j;
}

Program Program::from_db_row(pqxx::row row) {
    Program p;
    p.id = row["id"].as<std::string>();
    p.name = row["name"].as<std::string>();
    p.description = row["description"].as<std::string>();
    if (!row["author_id"].is_null()) {
        p.author_id = row["author_id"].as<std::string>();
    }
    p.goal = row["goal"].as<std::string>();
    p.experience_level = row["experience_level"].as<std::string>();
    p.duration_weeks = row["duration_weeks"].as<int>();
    p.workouts_per_week = row["workouts_per_week"].as<int>();
    p.is_official = row["is_official"].as<bool>();
    p.is_public = row["is_public"].as<bool>();
    return p;
}

nlohmann::json ProgramWeek::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["program_id"] = program_id;
    j["week_number"] = week_number;
    j["deload_week"] = deload_week;
    j["intensity_multiplier"] = intensity_multiplier;
    j["volume_multiplier"] = volume_multiplier;
    if (notes) j["notes"] = *notes;
    return j;
}

nlohmann::json ProgramWorkout::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["program_id"] = program_id;
    j["week_number"] = week_number;
    j["day_number"] = day_number;
    j["name"] = name;
    j["description"] = description;
    if (estimated_duration_minutes) j["estimated_duration_minutes"] = *estimated_duration_minutes;
    return j;
}

nlohmann::json ProgramExercise::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["program_workout_id"] = program_workout_id;
    j["exercise_id"] = exercise_id;
    j["exercise_order"] = exercise_order;
    j["target_sets"] = target_sets;
    if (target_reps_min) j["target_reps_min"] = *target_reps_min;
    if (target_reps_max) j["target_reps_max"] = *target_reps_max;
    if (target_rir) j["target_rir"] = *target_rir;
    j["target_rest_seconds"] = target_rest_seconds;
    j["is_superset"] = is_superset;
    if (superset_group) j["superset_group"] = *superset_group;
    j["progression_scheme"] = progression_scheme;
    if (notes) j["notes"] = *notes;
    return j;
}

nlohmann::json UserCoachingState::to_json() const {
    nlohmann::json j;
    j["user_id"] = user_id;
    j["mode"] = mode;
    if (current_program_id) j["current_program_id"] = *current_program_id;
    j["current_week"] = current_week;
    if (program_start_date) j["program_start_date"] = *program_start_date;
    if (last_check_in) j["last_check_in"] = *last_check_in;
    if (tdee_adaptive) j["tdee_adaptive"] = *tdee_adaptive;
    if (energy_balance) j["energy_balance"] = *energy_balance;
    return j;
}

nlohmann::json WeeklyCheckIn::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["user_id"] = user_id;
    j["week_start"] = week_start;
    j["week_end"] = week_end;
    if (hunger_rating) j["hunger_rating"] = *hunger_rating;
    if (energy_rating) j["energy_rating"] = *energy_rating;
    if (recovery_rating) j["recovery_rating"] = *recovery_rating;
    if (sleep_quality_rating) j["sleep_quality_rating"] = *sleep_quality_rating;
    if (average_weight_lbs) j["average_weight_lbs"] = *average_weight_lbs;
    if (weight_change_lbs) j["weight_change_lbs"] = *weight_change_lbs;
    j["total_workouts_completed"] = total_workouts_completed;
    j["total_sets_completed"] = total_sets_completed;
    if (ai_recommendation) j["ai_recommendation"] = *ai_recommendation;
    if (adjustment_made) j["adjustment_made"] = *adjustment_made;
    if (notes) j["notes"] = *notes;
    return j;
}

nlohmann::json CoachingQuestionnaire::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["user_id"] = user_id;
    j["experience_level"] = experience_level;
    j["years_training"] = years_training;
    j["current_frequency"] = current_frequency;
    j["primary_goal"] = primary_goal;
    if (secondary_goal) j["secondary_goal"] = *secondary_goal;
    if (goal_timeframe_weeks) j["goal_timeframe_weeks"] = *goal_timeframe_weeks;
    j["available_days_per_week"] = available_days_per_week;
    j["session_duration_minutes"] = session_duration_minutes;
    j["available_equipment"] = available_equipment;
    if (injuries_or_limitations) j["injuries_or_limitations"] = *injuries_or_limitations;
    j["exercises_to_avoid"] = exercises_to_avoid;
    if (preferred_rep_range) j["preferred_rep_range"] = *preferred_rep_range;
    if (preferred_intensity) j["preferred_intensity"] = *preferred_intensity;
    return j;
}

// Program CRUD
std::vector<Program> ProgramService::get_all_programs(
    const std::optional<std::string>& goal,
    const std::optional<std::string>& experience_level,
    bool official_only
) {
    auto conn = db_.get_connection();
    std::ostringstream query;
    query << "SELECT * FROM programs WHERE is_public = true";

    if (official_only) {
        query << " AND is_official = true";
    }
    if (goal) {
        query << " AND goal = " << conn->quote(*goal);
    }
    if (experience_level) {
        query << " AND experience_level = " << conn->quote(*experience_level);
    }

    query << " ORDER BY is_official DESC, name ASC";

    pqxx::work txn(*conn);
    pqxx::result result = txn.exec(query.str());

    std::vector<Program> programs;
    for (const auto& row : result) {
        programs.push_back(Program::from_db_row(row));
    }

    return programs;
}

std::optional<Program> ProgramService::get_program_by_id(const std::string& program_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT * FROM programs WHERE id = $1",
        program_id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    return Program::from_db_row(result[0]);
}

std::string ProgramService::create_program(const Program& program) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "INSERT INTO programs (name, description, author_id, goal, experience_level, "
        "duration_weeks, workouts_per_week, is_official, is_public) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) RETURNING id",
        program.name,
        program.description,
        program.author_id.value_or(""),
        program.goal,
        program.experience_level,
        program.duration_weeks,
        program.workouts_per_week,
        program.is_official,
        program.is_public
    );

    txn.commit();
    return result[0]["id"].as<std::string>();
}

// Get program workouts for a specific week
std::vector<ProgramWorkout> ProgramService::get_program_workouts(
    const std::string& program_id,
    int week_number
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT * FROM program_workouts "
        "WHERE program_id = $1 AND week_number = $2 "
        "ORDER BY day_number ASC",
        program_id,
        week_number
    );

    std::vector<ProgramWorkout> workouts;
    for (const auto& row : result) {
        ProgramWorkout w;
        w.id = row["id"].as<std::string>();
        w.program_id = row["program_id"].as<std::string>();
        w.week_number = row["week_number"].as<int>();
        w.day_number = row["day_number"].as<int>();
        w.name = row["name"].as<std::string>();
        w.description = row["description"].as<std::string>();
        if (!row["estimated_duration_minutes"].is_null()) {
            w.estimated_duration_minutes = row["estimated_duration_minutes"].as<int>();
        }
        workouts.push_back(w);
    }

    return workouts;
}

// Get exercises for a workout
std::vector<ProgramExercise> ProgramService::get_workout_exercises(const std::string& workout_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT * FROM program_exercises "
        "WHERE program_workout_id = $1 "
        "ORDER BY exercise_order ASC",
        workout_id
    );

    std::vector<ProgramExercise> exercises;
    for (const auto& row : result) {
        ProgramExercise e;
        e.id = row["id"].as<std::string>();
        e.program_workout_id = row["program_workout_id"].as<std::string>();
        e.exercise_id = row["exercise_id"].as<std::string>();
        e.exercise_order = row["exercise_order"].as<int>();
        e.target_sets = row["target_sets"].as<int>();
        if (!row["target_reps_min"].is_null()) e.target_reps_min = row["target_reps_min"].as<int>();
        if (!row["target_reps_max"].is_null()) e.target_reps_max = row["target_reps_max"].as<int>();
        if (!row["target_rir"].is_null()) e.target_rir = row["target_rir"].as<int>();
        e.target_rest_seconds = row["target_rest_seconds"].as<int>();
        e.is_superset = row["is_superset"].as<bool>();
        if (!row["superset_group"].is_null()) e.superset_group = row["superset_group"].as<int>();
        e.progression_scheme = row["progression_scheme"].as<std::string>();
        if (!row["notes"].is_null()) e.notes = row["notes"].as<std::string>();
        exercises.push_back(e);
    }

    return exercises;
}

// User coaching state
std::optional<UserCoachingState> ProgramService::get_coaching_state(const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT * FROM user_coaching_state WHERE user_id = $1",
        user_id
    );

    if (result.empty()) {
        return std::nullopt;
    }

    UserCoachingState state;
    auto row = result[0];
    state.user_id = row["user_id"].as<std::string>();
    state.mode = row["mode"].as<std::string>();
    if (!row["current_program_id"].is_null()) state.current_program_id = row["current_program_id"].as<std::string>();
    state.current_week = row["current_week"].as<int>();
    if (!row["program_start_date"].is_null()) state.program_start_date = row["program_start_date"].as<std::string>();
    if (!row["last_check_in"].is_null()) state.last_check_in = row["last_check_in"].as<std::string>();
    if (!row["tdee_adaptive"].is_null()) state.tdee_adaptive = row["tdee_adaptive"].as<double>();
    if (!row["energy_balance"].is_null()) state.energy_balance = row["energy_balance"].as<std::string>();

    return state;
}

void ProgramService::set_coaching_mode(const std::string& user_id, const std::string& mode) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "INSERT INTO user_coaching_state (user_id, mode) "
        "VALUES ($1, $2) "
        "ON CONFLICT (user_id) DO UPDATE SET mode = $2, updated_at = NOW()",
        user_id,
        mode
    );

    txn.commit();
}

void ProgramService::start_program(const std::string& user_id, const std::string& program_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "INSERT INTO user_coaching_state (user_id, current_program_id, current_week, program_start_date) "
        "VALUES ($1, $2, 1, CURRENT_DATE) "
        "ON CONFLICT (user_id) DO UPDATE SET "
        "current_program_id = $2, current_week = 1, program_start_date = CURRENT_DATE, updated_at = NOW()",
        user_id,
        program_id
    );

    txn.commit();
}

void ProgramService::advance_week(const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "UPDATE user_coaching_state SET current_week = current_week + 1, updated_at = NOW() "
        "WHERE user_id = $1",
        user_id
    );

    txn.commit();
}

// Weekly check-ins
std::string ProgramService::submit_checkin(const WeeklyCheckIn& checkin) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "INSERT INTO weekly_checkins (user_id, week_start, week_end, hunger_rating, "
        "energy_rating, recovery_rating, sleep_quality_rating, average_weight_lbs, "
        "weight_change_lbs, total_workouts_completed, total_sets_completed, notes) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12) "
        "RETURNING id",
        checkin.user_id,
        checkin.week_start,
        checkin.week_end,
        checkin.hunger_rating.value_or(0),
        checkin.energy_rating.value_or(0),
        checkin.recovery_rating.value_or(0),
        checkin.sleep_quality_rating.value_or(0),
        checkin.average_weight_lbs.value_or(0.0),
        checkin.weight_change_lbs.value_or(0.0),
        checkin.total_workouts_completed,
        checkin.total_sets_completed,
        checkin.notes.value_or("")
    );

    txn.commit();
    return result[0]["id"].as<std::string>();
}

// AI questionnaire
std::string ProgramService::save_questionnaire(const CoachingQuestionnaire& q) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "INSERT INTO coaching_questionnaire "
        "(user_id, experience_level, years_training, current_frequency, primary_goal, "
        "secondary_goal, goal_timeframe_weeks, available_days_per_week, session_duration_minutes, "
        "available_equipment, injuries_or_limitations, exercises_to_avoid, "
        "preferred_rep_range, preferred_intensity) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14) "
        "RETURNING id",
        q.user_id,
        q.experience_level,
        q.years_training,
        q.current_frequency,
        q.primary_goal,
        q.secondary_goal.value_or(""),
        q.goal_timeframe_weeks.value_or(0),
        q.available_days_per_week,
        q.session_duration_minutes,
        q.available_equipment.dump(),
        q.injuries_or_limitations.value_or(""),
        q.exercises_to_avoid.dump(),
        q.preferred_rep_range.value_or(""),
        q.preferred_intensity.value_or("")
    );

    txn.commit();
    return result[0]["id"].as<std::string>();
}

// Simple program selection based on questionnaire
std::optional<std::string> ProgramService::select_best_program(const CoachingQuestionnaire& q) {
    // Rule-based selection logic
    std::string selected_program_name;

    if (q.experience_level == "beginner") {
        selected_program_name = "FORGE Beginner Full Body";
    } else if (q.primary_goal == "strength" || q.primary_goal == "powerlifting") {
        if (q.experience_level == "advanced") {
            selected_program_name = "FORGE Powerlifting Peaking";
        } else {
            selected_program_name = "FORGE Strength 5/3/1";
        }
    } else if (q.primary_goal == "hypertrophy" || q.primary_goal == "bodybuilding") {
        if (q.available_days_per_week >= 6) {
            selected_program_name = "FORGE Hypertrophy Push/Pull/Legs";
        } else if (q.available_days_per_week == 5) {
            selected_program_name = "FORGE Bodybuilding Bro Split";
        } else {
            selected_program_name = "FORGE Upper/Lower Split";
        }
    } else {
        // Mixed goal
        if (q.available_days_per_week >= 5) {
            selected_program_name = "FORGE Upper/Lower Split";
        } else {
            selected_program_name = "FORGE Beginner Full Body";
        }
    }

    // Fetch the program ID
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);
    pqxx::result result = txn.exec_params(
        "SELECT id FROM programs WHERE name = $1 AND is_official = true",
        selected_program_name
    );

    if (result.empty()) {
        return std::nullopt;
    }

    return result[0]["id"].as<std::string>();
}

std::string ProgramService::generate_program_from_questionnaire(const std::string& user_id) {
    // Get the questionnaire
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    pqxx::result q_result = txn.exec_params(
        "SELECT * FROM coaching_questionnaire WHERE user_id = $1 ORDER BY created_at DESC LIMIT 1",
        user_id
    );

    if (q_result.empty()) {
        throw std::runtime_error("No questionnaire found for user");
    }

    CoachingQuestionnaire q;
    auto row = q_result[0];
    q.user_id = row["user_id"].as<std::string>();
    q.experience_level = row["experience_level"].as<std::string>();
    q.primary_goal = row["primary_goal"].as<std::string>();
    q.available_days_per_week = row["available_days_per_week"].as<int>();

    // Select best program
    auto program_id = select_best_program(q);
    if (!program_id) {
        throw std::runtime_error("Failed to select appropriate program");
    }

    // Start the program for this user
    start_program(user_id, *program_id);

    return *program_id;
}

} // namespace forge
