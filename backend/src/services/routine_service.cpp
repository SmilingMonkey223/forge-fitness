#include "../../include/routine_service.hpp"
#include "../../include/uuid.hpp"
#include <chrono>
#include <ctime>

namespace forge {

RoutineDetail RoutineService::create_routine(
    const std::string& user_id, const CreateRoutineRequest& req
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    std::string routine_id = UUID::generate();

    if (req.description) {
        txn.exec_params(
            "INSERT INTO routines (id, user_id, name, description) VALUES ($1, $2, $3, $4)",
            routine_id, user_id, req.name, *req.description
        );
    } else {
        txn.exec_params(
            "INSERT INTO routines (id, user_id, name) VALUES ($1, $2, $3)",
            routine_id, user_id, req.name
        );
    }

    int exercise_order = 1;
    for (const auto& ex_req : req.exercises) {
        std::string re_id = UUID::generate();

        if (ex_req.notes) {
            txn.exec_params(
                "INSERT INTO routine_exercises (id, routine_id, exercise_id, exercise_order, notes) "
                "VALUES ($1, $2, $3, $4, $5)",
                re_id, routine_id, ex_req.exercise_id, exercise_order, *ex_req.notes
            );
        } else {
            txn.exec_params(
                "INSERT INTO routine_exercises (id, routine_id, exercise_id, exercise_order) "
                "VALUES ($1, $2, $3, $4)",
                re_id, routine_id, ex_req.exercise_id, exercise_order
            );
        }

        int set_order = 1;
        for (const auto& set_req : ex_req.sets) {
            std::string rs_id = UUID::generate();
            std::optional<int> reps_min = set_req.target_reps;
            std::optional<int> reps_max = set_req.target_reps_max
                ? set_req.target_reps_max : set_req.target_reps;

            if (reps_min && set_req.target_duration_seconds) {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type, "
                    "  target_reps_min, target_reps_max, target_duration_seconds) "
                    "VALUES ($1, $2, $3, $4, $5, $6, $7)",
                    rs_id, re_id, set_order, set_req.set_type,
                    *reps_min, *reps_max, *set_req.target_duration_seconds
                );
            } else if (reps_min) {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type, "
                    "  target_reps_min, target_reps_max) "
                    "VALUES ($1, $2, $3, $4, $5, $6)",
                    rs_id, re_id, set_order, set_req.set_type, *reps_min, *reps_max
                );
            } else if (set_req.target_duration_seconds) {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type, "
                    "  target_duration_seconds) "
                    "VALUES ($1, $2, $3, $4, $5)",
                    rs_id, re_id, set_order, set_req.set_type, *set_req.target_duration_seconds
                );
            } else {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type) "
                    "VALUES ($1, $2, $3, $4)",
                    rs_id, re_id, set_order, set_req.set_type
                );
            }
            set_order++;
        }
        exercise_order++;
    }

    txn.commit();
    return get_routine(user_id, routine_id).value();
}

std::vector<RoutineSummary> RoutineService::list_routines(const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT r.id, r.name, r.description, "
        "       COUNT(re.id) AS exercise_count "
        "FROM routines r "
        "LEFT JOIN routine_exercises re ON re.routine_id = r.id "
        "WHERE r.user_id = $1 AND r.deleted_at IS NULL "
        "GROUP BY r.id "
        "ORDER BY r.created_at DESC",
        user_id
    );

    txn.commit();

    std::vector<RoutineSummary> result;
    for (const auto& row : r) {
        RoutineSummary rs;
        rs.id = row["id"].as<std::string>();
        rs.name = row["name"].as<std::string>();
        if (!row["description"].is_null())
            rs.description = row["description"].as<std::string>();
        rs.exercise_count = row["exercise_count"].as<int>(0);
        result.push_back(std::move(rs));
    }
    return result;
}

std::optional<RoutineDetail> RoutineService::get_routine(
    const std::string& user_id, const std::string& routine_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT id, user_id, name, description "
        "FROM routines WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        routine_id, user_id
    );

    if (r.empty()) {
        txn.commit();
        return std::nullopt;
    }

    RoutineDetail rd;
    rd.id = r[0]["id"].as<std::string>();
    rd.user_id = r[0]["user_id"].as<std::string>();
    rd.name = r[0]["name"].as<std::string>();
    if (!r[0]["description"].is_null())
        rd.description = r[0]["description"].as<std::string>();

    rd.exercises = load_routine_exercises(txn, routine_id);

    txn.commit();
    return rd;
}

std::optional<RoutineDetail> RoutineService::update_routine(
    const std::string& user_id, const std::string& routine_id, const CreateRoutineRequest& req
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto check = txn.exec_params(
        "SELECT id FROM routines WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        routine_id, user_id
    );
    if (check.empty()) {
        txn.commit();
        return std::nullopt;
    }

    if (req.description) {
        txn.exec_params(
            "UPDATE routines SET name = $1, description = $2, updated_at = CURRENT_TIMESTAMP WHERE id = $3",
            req.name, *req.description, routine_id
        );
    } else {
        txn.exec_params(
            "UPDATE routines SET name = $1, description = NULL, updated_at = CURRENT_TIMESTAMP WHERE id = $2",
            req.name, routine_id
        );
    }

    // Delete old exercises (cascades to routine_sets)
    txn.exec_params("DELETE FROM routine_exercises WHERE routine_id = $1", routine_id);

    int exercise_order = 1;
    for (const auto& ex_req : req.exercises) {
        std::string re_id = UUID::generate();

        if (ex_req.notes) {
            txn.exec_params(
                "INSERT INTO routine_exercises (id, routine_id, exercise_id, exercise_order, notes) "
                "VALUES ($1, $2, $3, $4, $5)",
                re_id, routine_id, ex_req.exercise_id, exercise_order, *ex_req.notes
            );
        } else {
            txn.exec_params(
                "INSERT INTO routine_exercises (id, routine_id, exercise_id, exercise_order) "
                "VALUES ($1, $2, $3, $4)",
                re_id, routine_id, ex_req.exercise_id, exercise_order
            );
        }

        int set_order = 1;
        for (const auto& set_req : ex_req.sets) {
            std::string rs_id = UUID::generate();
            std::optional<int> reps_min = set_req.target_reps;
            std::optional<int> reps_max = set_req.target_reps_max
                ? set_req.target_reps_max : set_req.target_reps;

            if (reps_min && set_req.target_duration_seconds) {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type, "
                    "  target_reps_min, target_reps_max, target_duration_seconds) "
                    "VALUES ($1, $2, $3, $4, $5, $6, $7)",
                    rs_id, re_id, set_order, set_req.set_type,
                    *reps_min, *reps_max, *set_req.target_duration_seconds
                );
            } else if (reps_min) {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type, "
                    "  target_reps_min, target_reps_max) "
                    "VALUES ($1, $2, $3, $4, $5, $6)",
                    rs_id, re_id, set_order, set_req.set_type, *reps_min, *reps_max
                );
            } else if (set_req.target_duration_seconds) {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type, "
                    "  target_duration_seconds) "
                    "VALUES ($1, $2, $3, $4, $5)",
                    rs_id, re_id, set_order, set_req.set_type, *set_req.target_duration_seconds
                );
            } else {
                txn.exec_params(
                    "INSERT INTO routine_sets (id, routine_exercise_id, set_order, set_type) "
                    "VALUES ($1, $2, $3, $4)",
                    rs_id, re_id, set_order, set_req.set_type
                );
            }
            set_order++;
        }
        exercise_order++;
    }

    txn.commit();
    return get_routine(user_id, routine_id);
}

bool RoutineService::delete_routine(
    const std::string& user_id, const std::string& routine_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "UPDATE routines SET deleted_at = CURRENT_TIMESTAMP "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL "
        "RETURNING id",
        routine_id, user_id
    );

    txn.commit();
    return !r.empty();
}

std::string RoutineService::start_from_routine(
    const std::string& user_id, const std::string& routine_id
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto rr = txn.exec_params(
        "SELECT name FROM routines WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        routine_id, user_id
    );
    if (rr.empty()) {
        throw std::invalid_argument("ROUTINE_NOT_FOUND");
    }

    std::string routine_name = rr[0]["name"].as<std::string>();
    std::string workout_id = UUID::generate();

    txn.exec_params(
        "INSERT INTO workouts (id, user_id, name, status) VALUES ($1, $2, $3, 'in_progress')",
        workout_id, user_id, routine_name
    );

    auto exercises = txn.exec_params(
        "SELECT re.exercise_id, re.exercise_order, re.id AS re_id "
        "FROM routine_exercises re "
        "WHERE re.routine_id = $1 ORDER BY re.exercise_order",
        routine_id
    );

    for (const auto& ex : exercises) {
        std::string exercise_id = ex["exercise_id"].as<std::string>();
        int exercise_order = ex["exercise_order"].as<int>();

        auto sets = txn.exec_params(
            "SELECT set_order, set_type "
            "FROM routine_sets WHERE routine_exercise_id = $1 ORDER BY set_order",
            ex["re_id"].as<std::string>()
        );

        for (const auto& s : sets) {
            std::string set_id = UUID::generate();
            txn.exec_params(
                "INSERT INTO exercise_sets (id, workout_id, exercise_id, set_order, "
                "  exercise_order, set_type, is_pr) "
                "VALUES ($1, $2, $3, $4, $5, $6, false)",
                set_id, workout_id, exercise_id,
                s["set_order"].as<int>(), exercise_order, s["set_type"].as<std::string>()
            );
        }
    }

    txn.commit();
    return workout_id;
}

std::vector<RoutineExercise> RoutineService::load_routine_exercises(
    pqxx::work& txn, const std::string& routine_id
) {
    auto r = txn.exec_params(
        "SELECT re.id, re.exercise_id, re.exercise_order, re.notes, e.name AS exercise_name "
        "FROM routine_exercises re "
        "JOIN exercises e ON e.id = re.exercise_id "
        "WHERE re.routine_id = $1 ORDER BY re.exercise_order",
        routine_id
    );

    std::vector<RoutineExercise> result;
    for (const auto& row : r) {
        RoutineExercise re;
        re.id = row["id"].as<std::string>();
        re.exercise_id = row["exercise_id"].as<std::string>();
        re.exercise_name = row["exercise_name"].as<std::string>();
        re.order_index = row["exercise_order"].as<int>();
        if (!row["notes"].is_null())
            re.notes = row["notes"].as<std::string>();

        auto sr = txn.exec_params(
            "SELECT id, set_order, set_type, target_reps_min, target_reps_max, "
            "       target_duration_seconds "
            "FROM routine_sets WHERE routine_exercise_id = $1 ORDER BY set_order",
            re.id
        );

        for (const auto& srow : sr) {
            RoutineSetTemplate rst;
            rst.id = srow["id"].as<std::string>();
            rst.set_order = srow["set_order"].as<int>();
            rst.set_type = srow["set_type"].as<std::string>();
            if (!srow["target_reps_min"].is_null())
                rst.target_reps = srow["target_reps_min"].as<int>();
            if (!srow["target_reps_max"].is_null())
                rst.target_reps_max = srow["target_reps_max"].as<int>();
            if (!srow["target_duration_seconds"].is_null())
                rst.target_duration_seconds = srow["target_duration_seconds"].as<int>();
            re.sets.push_back(std::move(rst));
        }

        result.push_back(std::move(re));
    }
    return result;
}

} // namespace forge
