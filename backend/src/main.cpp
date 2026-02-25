#include <crow.h>
#include "../include/config.hpp"
#include "../include/database.hpp"
#include "../include/jwt.hpp"
#include "../include/profile_service.hpp"
#include "../include/workout_service.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

namespace forge {

// Auth middleware
struct AuthMiddleware {
    struct context {
        std::string user_id;
        std::string username;
    };

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        // Skip auth for public endpoints
        std::string path = req.url;
        if (path.find("/api/auth/") == 0 ||
            path == "/health" ||
            path == "/api/exercises") {
            return;
        }

        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            res.code = 401;
            res.write(json({
                {"error", {
                    {"code", "UNAUTHORIZED"},
                    {"message", "Missing or invalid authorization header"}
                }}
            }).dump());
            res.end();
            return;
        }

        std::string token = auth_header.substr(7);

        try {
            auto decoded = JWT::verify(token);
            ctx.user_id = JWT::get_user_id(decoded);
            ctx.username = JWT::get_username(decoded);
        } catch (const std::exception& e) {
            res.code = 401;
            res.write(json({
                {"error", {
                    {"code", "UNAUTHORIZED"},
                    {"message", "Invalid or expired token"}
                }}
            }).dump());
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response&, context&) {}
};

// CORS middleware
struct CORSMiddleware {
    struct context {};

    void before_handle(crow::request&, crow::response& res, context&) {
        res.add_header("Access-Control-Allow-Origin", Config::instance().cors_origins());
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.add_header("Access-Control-Allow-Credentials", "true");
    }

    void after_handle(crow::request&, crow::response&, context&) {}
};

} // namespace forge

int main() {
    try {
        // Load configuration
        auto& config = forge::Config::instance();
        std::cout << "Starting FORGE backend on port " << config.port() << std::endl;

        // Initialize database
        forge::Database::instance().initialize(config.database_url());
        std::cout << "Database connection established" << std::endl;

        // Create Crow app with middlewares
        crow::App<forge::CORSMiddleware, forge::AuthMiddleware> app;

        // Logging middleware
        app.get_middleware<forge::CORSMiddleware>();

        // Health check endpoint
        CROW_ROUTE(app, "/health")
        ([]() {
            return crow::response(200, json({
                {"status", "ok"},
                {"timestamp", std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now())}
            }).dump());
        });

        // Auth endpoints
        CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
        ([](const crow::request& req) {
            try {
                auto body = json::parse(req.body);

                forge::AuthService::RegisterRequest register_req;
                register_req.email = body["email"];
                register_req.username = body["username"];
                register_req.password = body["password"];
                register_req.display_name = body["display_name"];

                auto [user, tokens] = forge::AuthService::register_user(register_req);

                auto response = crow::response(201, json({
                    {"user", user.to_json()},
                    {"access_token", tokens.access_token},
                    {"refresh_token", tokens.refresh_token}
                }).dump());

                response.add_header("Content-Type", "application/json");

                // Set refresh token as HttpOnly cookie
                response.add_header("Set-Cookie",
                    "refresh_token=" + tokens.refresh_token +
                    "; HttpOnly; Secure; SameSite=Strict; Max-Age=2592000; Path=/");

                return response;

            } catch (const std::invalid_argument& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Registration failed"}
                    }}
                }).dump());
            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
        ([](const crow::request& req) {
            try {
                auto body = json::parse(req.body);

                forge::AuthService::LoginRequest login_req;
                login_req.email = body["email"];
                login_req.password = body["password"];

                auto [user, tokens] = forge::AuthService::login(login_req);

                auto response = crow::response(200, json({
                    {"user", user.to_json()},
                    {"access_token", tokens.access_token},
                    {"refresh_token", tokens.refresh_token}
                }).dump());

                response.add_header("Content-Type", "application/json");
                response.add_header("Set-Cookie",
                    "refresh_token=" + tokens.refresh_token +
                    "; HttpOnly; Secure; SameSite=Strict; Max-Age=2592000; Path=/");

                return response;

            } catch (const std::invalid_argument& e) {
                return crow::response(401, json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Login failed"}
                    }}
                }).dump());
            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        // Initialize services
        forge::WorkoutService workout_service(forge::Database::instance());

        // ── Workout Tracking Endpoints ──────────────────────────────

        // POST /api/workouts - Start a new workout
        CROW_ROUTE(app, "/api/workouts").methods("POST"_method)
        ([&workout_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                std::optional<std::string> name;
                std::optional<std::string> notes;

                if (!req.body.empty()) {
                    auto body = json::parse(req.body);
                    if (body.contains("name") && !body["name"].is_null()) {
                        name = body["name"].get<std::string>();
                    }
                    if (body.contains("notes") && !body["notes"].is_null()) {
                        notes = body["notes"].get<std::string>();
                    }
                }

                std::string workout_id = workout_service.create_workout(
                    ctx.user_id, name, notes
                );

                res.code = 201;
                res.write(json({
                    {"id", workout_id},
                    {"status", "in_progress"},
                    {"message", "Workout started"}
                }).dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // GET /api/workouts - List user's workouts (paginated)
        CROW_ROUTE(app, "/api/workouts")
        ([&workout_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto limit_param = req.url_params.get("limit");
                auto offset_param = req.url_params.get("offset");
                int limit = limit_param ? std::stoi(limit_param) : 20;
                int offset = offset_param ? std::stoi(offset_param) : 0;

                auto workouts = workout_service.list_workouts(ctx.user_id, limit, offset);

                json workouts_json = json::array();
                for (const auto& w : workouts) {
                    workouts_json.push_back(w.to_json());
                }

                res.write(json({
                    {"workouts", workouts_json},
                    {"limit", limit},
                    {"offset", offset}
                }).dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // GET /api/workouts/:id - Get workout detail with all sets
        CROW_ROUTE(app, "/api/workouts/<string>")
        ([&workout_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& workout_id) {
            try {
                auto workout = workout_service.get_workout(workout_id, ctx.user_id);

                if (!workout) {
                    res.code = 404;
                    res.write(json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Workout not found"}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

                res.write(workout->to_json().dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // PUT /api/workouts/:id/complete - Complete workout
        CROW_ROUTE(app, "/api/workouts/<string>/complete").methods("PUT"_method)
        ([&workout_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& workout_id) {
            try {
                std::optional<std::string> notes;
                if (!req.body.empty()) {
                    auto body = json::parse(req.body);
                    if (body.contains("notes") && !body["notes"].is_null()) {
                        notes = body["notes"].get<std::string>();
                    }
                }

                auto workout = workout_service.complete_workout(
                    workout_id, ctx.user_id, notes
                );

                if (!workout) {
                    res.code = 404;
                    res.write(json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Workout not found"}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

                res.write(workout->to_json().dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                std::string code = e.what();
                int status = (code == "WORKOUT_NOT_IN_PROGRESS") ? 400 : 404;
                res.code = status;
                res.write(json({
                    {"error", {
                        {"code", code},
                        {"message", code == "WORKOUT_NOT_IN_PROGRESS"
                            ? "Workout is not in progress"
                            : "Workout not found"}
                    }}
                }).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // DELETE /api/workouts/:id - Soft delete workout
        CROW_ROUTE(app, "/api/workouts/<string>").methods("DELETE"_method)
        ([&workout_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& workout_id) {
            try {
                bool deleted = workout_service.delete_workout(workout_id, ctx.user_id);

                if (!deleted) {
                    res.code = 404;
                    res.write(json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Workout not found"}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

                res.write(json({
                    {"success", true},
                    {"message", "Workout deleted"}
                }).dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // POST /api/workouts/:id/sets - Add a set to workout
        CROW_ROUTE(app, "/api/workouts/<string>/sets").methods("POST"_method)
        ([&workout_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& workout_id) {
            try {
                auto body = json::parse(req.body);

                std::string exercise_id = body["exercise_id"];
                std::string set_type = body.value("set_type", "working");

                std::optional<double> weight_kg;
                std::optional<int> reps;
                std::optional<double> rpe;
                std::optional<int> duration_seconds;
                std::optional<int> rest_seconds;
                std::optional<std::string> notes;

                if (body.contains("weight_kg") && !body["weight_kg"].is_null()) {
                    weight_kg = body["weight_kg"].get<double>();
                }
                if (body.contains("reps") && !body["reps"].is_null()) {
                    reps = body["reps"].get<int>();
                }
                if (body.contains("rpe") && !body["rpe"].is_null()) {
                    rpe = body["rpe"].get<double>();
                }
                if (body.contains("duration_seconds") && !body["duration_seconds"].is_null()) {
                    duration_seconds = body["duration_seconds"].get<int>();
                }
                if (body.contains("rest_seconds") && !body["rest_seconds"].is_null()) {
                    rest_seconds = body["rest_seconds"].get<int>();
                }
                if (body.contains("notes") && !body["notes"].is_null()) {
                    notes = body["notes"].get<std::string>();
                }

                auto result = workout_service.add_set(
                    workout_id, ctx.user_id, exercise_id,
                    weight_kg, reps, rpe, set_type,
                    duration_seconds, rest_seconds, notes
                );

                res.code = 201;
                res.write(result.to_json().dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                std::string code = e.what();
                int status = 400;
                if (code == "WORKOUT_NOT_FOUND" || code == "EXERCISE_NOT_FOUND") {
                    status = 404;
                }
                res.code = status;
                res.write(json({
                    {"error", {
                        {"code", code},
                        {"message", code}
                    }}
                }).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // PUT /api/workouts/:id/sets/:set_id - Update a set
        CROW_ROUTE(app, "/api/workouts/<string>/sets/<string>").methods("PUT"_method)
        ([&workout_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx,
          const std::string& workout_id, const std::string& set_id) {
            try {
                auto body = json::parse(req.body);

                std::optional<double> weight_kg;
                std::optional<int> reps;
                std::optional<double> rpe;
                std::optional<std::string> set_type;
                std::optional<int> duration_seconds;
                std::optional<int> rest_seconds;
                std::optional<std::string> notes;

                if (body.contains("weight_kg") && !body["weight_kg"].is_null()) {
                    weight_kg = body["weight_kg"].get<double>();
                }
                if (body.contains("reps") && !body["reps"].is_null()) {
                    reps = body["reps"].get<int>();
                }
                if (body.contains("rpe") && !body["rpe"].is_null()) {
                    rpe = body["rpe"].get<double>();
                }
                if (body.contains("set_type") && !body["set_type"].is_null()) {
                    set_type = body["set_type"].get<std::string>();
                }
                if (body.contains("duration_seconds") && !body["duration_seconds"].is_null()) {
                    duration_seconds = body["duration_seconds"].get<int>();
                }
                if (body.contains("rest_seconds") && !body["rest_seconds"].is_null()) {
                    rest_seconds = body["rest_seconds"].get<int>();
                }
                if (body.contains("notes") && !body["notes"].is_null()) {
                    notes = body["notes"].get<std::string>();
                }

                auto result = workout_service.update_set(
                    workout_id, set_id, ctx.user_id,
                    weight_kg, reps, rpe, set_type,
                    duration_seconds, rest_seconds, notes
                );

                if (!result) {
                    res.code = 404;
                    res.write(json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Set or workout not found"}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

                res.write(result->to_json().dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // DELETE /api/workouts/:id/sets/:set_id - Delete a set
        CROW_ROUTE(app, "/api/workouts/<string>/sets/<string>").methods("DELETE"_method)
        ([&workout_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx,
          const std::string& workout_id, const std::string& set_id) {
            try {
                bool deleted = workout_service.delete_set(workout_id, set_id, ctx.user_id);

                if (!deleted) {
                    res.code = 404;
                    res.write(json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Set or workout not found"}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

                res.write(json({
                    {"success", true},
                    {"message", "Set deleted"}
                }).dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // ── Exercise Library Endpoints ──────────────────────────────

        // GET /api/exercises - List/search exercises
        CROW_ROUTE(app, "/api/exercises")
        ([&workout_service](const crow::request& req) {
            try {
                auto muscle_group = req.url_params.get("muscle_group");
                auto search = req.url_params.get("search");
                auto equipment = req.url_params.get("equipment");
                auto limit_param = req.url_params.get("limit");
                auto offset_param = req.url_params.get("offset");

                std::optional<std::string> mg_opt = muscle_group
                    ? std::optional<std::string>(muscle_group) : std::nullopt;
                std::optional<std::string> search_opt = search
                    ? std::optional<std::string>(search) : std::nullopt;
                std::optional<std::string> eq_opt = equipment
                    ? std::optional<std::string>(equipment) : std::nullopt;
                int limit = limit_param ? std::stoi(limit_param) : 50;
                int offset = offset_param ? std::stoi(offset_param) : 0;

                auto exercises = workout_service.list_exercises(
                    mg_opt, search_opt, eq_opt, limit, offset
                );

                json exercises_json = json::array();
                for (const auto& ex : exercises) {
                    exercises_json.push_back(ex.to_json());
                }

                return crow::response(200, json({
                    {"exercises", exercises_json},
                    {"count", static_cast<int>(exercises.size())},
                    {"limit", limit},
                    {"offset", offset}
                }).dump());

            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        // GET /api/exercises/:id - Get exercise detail
        CROW_ROUTE(app, "/api/exercises/<string>")
        ([&workout_service](const std::string& exercise_id) {
            try {
                auto exercise = workout_service.get_exercise(exercise_id);

                if (!exercise) {
                    return crow::response(404, json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Exercise not found"}
                        }}
                    }).dump());
                }

                return crow::response(200, exercise->to_json().dump());

            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        // GET /api/exercises/:id/history - Get last 5 performances
        CROW_ROUTE(app, "/api/exercises/<string>/history")
        ([&workout_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& exercise_id) {
            try {
                auto limit_param = req.url_params.get("limit");
                int limit = limit_param ? std::stoi(limit_param) : 5;

                auto history = workout_service.get_exercise_history(
                    exercise_id, ctx.user_id, limit
                );

                json history_json = json::array();
                for (const auto& entry : history) {
                    history_json.push_back(entry.to_json());
                }

                auto best_1rm = workout_service.get_best_1rm(exercise_id, ctx.user_id);

                json response_data = {
                    {"exercise_id", exercise_id},
                    {"history", history_json},
                    {"session_count", static_cast<int>(history.size())}
                };
                if (best_1rm) {
                    response_data["best_estimated_1rm"] = *best_1rm;
                }

                res.write(response_data.dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // Placeholder for other endpoints
        CROW_ROUTE(app, "/api/dashboard")
        ([](const crow::request&, crow::response& res, forge::AuthMiddleware::context& ctx) {
            // TODO: Implement dashboard logic
            res.write(json({
                {"today", {
                    {"nutrition", {
                        {"calories", {{"consumed", 0}, {"target", 2400}}},
                        {"protein_g", {{"consumed", 0}, {"target", 176}}}
                    }},
                    {"workout", nullptr}
                }},
                {"week", {
                    {"workout_days", json::array()},
                    {"daily_calories", json::array()}
                }}
            }).dump());
            res.end();
        });

        // Profile endpoints
        CROW_ROUTE(app, "/api/profile")
        ([](const crow::request&, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto profile = forge::ProfileService::get_profile(ctx.user_id);

                if (!profile.has_value()) {
                    res.code = 404;
                    res.write(json({
                        {"error", {
                            {"code", "PROFILE_NOT_FOUND"},
                            {"message", "No profile found. Please complete onboarding."}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

                res.code = 200;
                res.add_header("Content-Type", "application/json");
                res.write(json({{"profile", profile->to_json()}}).dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        CROW_ROUTE(app, "/api/profile").methods("PUT"_method)
        ([](const crow::request& req, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::ProfileService::UpdateProfileRequest update_req;
                update_req.user_id = ctx.user_id;

                if (body.contains("date_of_birth"))
                    update_req.date_of_birth = body["date_of_birth"].get<std::string>();
                if (body.contains("sex"))
                    update_req.sex = body["sex"].get<std::string>();
                if (body.contains("height_cm"))
                    update_req.height_cm = body["height_cm"].get<double>();
                if (body.contains("weight_kg"))
                    update_req.weight_kg = body["weight_kg"].get<double>();
                if (body.contains("activity_level"))
                    update_req.activity_level = body["activity_level"].get<std::string>();
                if (body.contains("fitness_goal"))
                    update_req.fitness_goal = body["fitness_goal"].get<std::string>();
                if (body.contains("unit_preference"))
                    update_req.unit_preference = body["unit_preference"].get<std::string>();

                auto profile = forge::ProfileService::update_profile(update_req);

                res.code = 200;
                res.add_header("Content-Type", "application/json");
                res.write(json({{"profile", profile.to_json()}}).dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                res.code = 400;
                res.write(json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Profile update failed"}
                    }}
                }).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        CROW_ROUTE(app, "/api/profile/onboarding").methods("POST"_method)
        ([](const crow::request& req, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::ProfileService::OnboardingRequest onboarding_req;
                onboarding_req.user_id = ctx.user_id;
                onboarding_req.date_of_birth = body["date_of_birth"].get<std::string>();
                onboarding_req.sex = body["sex"].get<std::string>();
                onboarding_req.height_cm = body["height_cm"].get<double>();
                onboarding_req.weight_kg = body["weight_kg"].get<double>();
                onboarding_req.activity_level = body["activity_level"].get<std::string>();
                onboarding_req.fitness_goal = body["fitness_goal"].get<std::string>();

                if (body.contains("unit_preference"))
                    onboarding_req.unit_preference = body["unit_preference"].get<std::string>();

                auto profile = forge::ProfileService::complete_onboarding(onboarding_req);

                res.code = 201;
                res.add_header("Content-Type", "application/json");
                res.write(json({{"profile", profile.to_json()}}).dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                res.code = 400;
                res.write(json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Onboarding failed"}
                    }}
                }).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
                res.end();
            }
        });

        // Start server
        app.port(config.port())
           .multithreaded()
           .run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
