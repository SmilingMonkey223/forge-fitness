#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"
#include "models.hpp"

namespace forge {

struct RoutineSetTemplate {
    std::string id;
    int set_order;
    std::string set_type;
    std::optional<int> target_reps;
    std::optional<int> target_reps_max;
    std::optional<int> target_duration_seconds;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"set_order", set_order},
            {"set_type", set_type}
        };
        if (target_reps) j["target_reps"] = *target_reps;
        if (target_duration_seconds) j["target_duration_seconds"] = *target_duration_seconds;
        return j;
    }
};

struct RoutineExercise {
    std::string id;
    std::string exercise_id;
    std::string exercise_name;
    int order_index;
    std::optional<std::string> notes;
    std::vector<RoutineSetTemplate> sets;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"exercise_id", exercise_id},
            {"exercise_name", exercise_name},
            {"order_index", order_index}
        };
        if (notes) j["notes"] = *notes;
        nlohmann::json sets_arr = nlohmann::json::array();
        for (const auto& s : sets) {
            sets_arr.push_back(s.to_json());
        }
        j["sets"] = sets_arr;
        return j;
    }
};

struct RoutineDetail {
    std::string id;
    std::string user_id;
    std::string name;
    std::optional<std::string> description;
    std::vector<RoutineExercise> exercises;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"user_id", user_id},
            {"name", name}
        };
        if (description) j["description"] = *description;
        nlohmann::json ex_arr = nlohmann::json::array();
        for (const auto& e : exercises) {
            ex_arr.push_back(e.to_json());
        }
        j["exercises"] = ex_arr;
        return j;
    }
};

struct RoutineSummary {
    std::string id;
    std::string name;
    std::optional<std::string> description;
    int exercise_count;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"name", name},
            {"exercise_count", exercise_count}
        };
        if (description) j["description"] = *description;
        return j;
    }
};

struct CreateRoutineSetRequest {
    std::string set_type = "working";
    std::optional<int> target_reps;
    std::optional<int> target_reps_max;
    std::optional<int> target_duration_seconds;
};

struct CreateRoutineExerciseRequest {
    std::string exercise_id;
    std::optional<std::string> notes;
    std::vector<CreateRoutineSetRequest> sets;
};

struct CreateRoutineRequest {
    std::string name;
    std::optional<std::string> description;
    std::vector<CreateRoutineExerciseRequest> exercises;
};

class RoutineService {
public:
    explicit RoutineService(Database& db) : db_(db) {}

    RoutineDetail create_routine(const std::string& user_id, const CreateRoutineRequest& req);
    std::vector<RoutineSummary> list_routines(const std::string& user_id);
    std::optional<RoutineDetail> get_routine(const std::string& user_id, const std::string& routine_id);
    std::optional<RoutineDetail> update_routine(const std::string& user_id, const std::string& routine_id, const CreateRoutineRequest& req);
    bool delete_routine(const std::string& user_id, const std::string& routine_id);
    std::string start_from_routine(const std::string& user_id, const std::string& routine_id);

private:
    Database& db_;
    std::vector<RoutineExercise> load_routine_exercises(pqxx::work& txn, const std::string& routine_id);
};

} // namespace forge
