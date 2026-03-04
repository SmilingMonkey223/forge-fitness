#include <crow.h>
#include "../include/config.hpp"
#include "../include/database.hpp"
#include "../include/jwt.hpp"
#include "../include/auth_service.hpp"
#include "../include/profile_service.hpp"
#include "../include/workout_service.hpp"
#include "../include/weight_service.hpp"
#include "../include/analytics_service.hpp"
#include "../include/nutrition_service.hpp"
#include "../include/dashboard_service.hpp"
#include "../include/routine_service.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <algorithm>

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
        forge::WeightService weight_service(forge::Database::instance());
        forge::AnalyticsService analytics_service(forge::Database::instance());
        forge::NutritionService nutrition_service(forge::Database::instance());
        forge::DashboardService dashboard_service(forge::Database::instance());
        forge::RoutineService routine_service(forge::Database::instance());

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

                // Return full workout object as frontend expects
                auto workout = workout_service.get_workout(workout_id, ctx.user_id);
                res.code = 201;
                if (workout) {
                    res.write(workout->to_json().dump());
                } else {
                    res.write(json({
                        {"id", workout_id},
                        {"status", "in_progress"},
                        {"sets", json::array()},
                        {"name", name.value_or("Workout")}
                    }).dump());
                }
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
                int limit = limit_param ? std::min(std::stoi(limit_param), 100) : 20;
                int offset = offset_param ? std::max(std::stoi(offset_param), 0) : 0;

                auto workouts = workout_service.list_workouts(ctx.user_id, limit, offset);

                json workouts_json = json::array();
                for (const auto& w : workouts) {
                    workouts_json.push_back(w.to_json());
                }

                res.write(workouts_json.dump());
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

                // Validate set_type
                if (set_type != "working" && set_type != "warmup" &&
                    set_type != "dropset" && set_type != "failure") {
                    res.code = 400;
                    res.write(json({
                        {"error", {
                            {"code", "INVALID_SET_TYPE"},
                            {"message", "set_type must be one of: working, warmup, dropset, failure"}
                        }}
                    }).dump());
                    res.end();
                    return;
                }

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

                return crow::response(200, exercises_json.dump());

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

                res.write(history_json.dump());
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

        // ── Dashboard Endpoint ────────────────────────────────────────
        CROW_ROUTE(app, "/api/dashboard")
        ([&dashboard_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto data = dashboard_service.get_dashboard(ctx.user_id);
                res.write(data.to_json().dump());
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

        // ── Weight Tracking Endpoints ─────────────────────────────

        // POST /api/weight - Log a weight entry
        CROW_ROUTE(app, "/api/weight").methods("POST"_method)
        ([&weight_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                double weight_kg = body["weight_kg"].get<double>();
                std::optional<std::string> notes;
                if (body.contains("notes") && !body["notes"].is_null()) {
                    notes = body["notes"].get<std::string>();
                }

                auto entry = weight_service.log_weight(ctx.user_id, weight_kg, notes);

                res.code = 201;
                res.write(entry.to_json().dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                res.code = 400;
                res.write(json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Failed to log weight"}
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

        // GET /api/weight - Get weight history
        CROW_ROUTE(app, "/api/weight")
        ([&weight_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto limit_param = req.url_params.get("limit");
                int limit = limit_param ? std::stoi(limit_param) : 90;

                auto entries = weight_service.get_weight_history(ctx.user_id, limit);

                json data = json::array();
                for (const auto& e : entries) {
                    data.push_back(e.to_json());
                }

                res.write(json({{"data", data}}).dump());
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

        // GET /api/weight/trend - Get weight trend analysis
        CROW_ROUTE(app, "/api/weight/trend")
        ([&weight_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto trend = weight_service.get_weight_trend(ctx.user_id);
                res.write(trend.to_json().dump());
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

        // DELETE /api/weight/:id - Delete a weight entry
        CROW_ROUTE(app, "/api/weight/<string>").methods("DELETE"_method)
        ([&weight_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& weight_id) {
            try {
                bool deleted = weight_service.delete_weight(weight_id, ctx.user_id);
                if (!deleted) {
                    res.code = 404;
                    res.write(json({
                        {"error", {{"code", "NOT_FOUND"}, {"message", "Weight entry not found"}}}
                    }).dump());
                    res.end();
                    return;
                }
                res.write(json({{"success", true}}).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // ── Analytics Endpoints ──────────────────────────────────────

        // GET /api/analytics/muscle-distribution
        CROW_ROUTE(app, "/api/analytics/muscle-distribution")
        ([&analytics_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto days_param = req.url_params.get("days");
                int days = days_param ? std::stoi(days_param) : 30;

                auto distribution = analytics_service.get_muscle_distribution(ctx.user_id, days);

                json data = json::array();
                for (const auto& v : distribution) {
                    data.push_back(v.to_json());
                }

                res.write(json({{"data", data}}).dump());
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

        // GET /api/analytics/prs - Personal record history
        CROW_ROUTE(app, "/api/analytics/prs")
        ([&analytics_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto limit_param = req.url_params.get("limit");
                int limit = limit_param ? std::stoi(limit_param) : 20;

                auto prs = analytics_service.get_pr_history(ctx.user_id, limit);

                json prs_json = json::array();
                for (const auto& pr : prs) {
                    prs_json.push_back(pr.to_json());
                }

                res.write(json({{"prs", prs_json}}).dump());
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

        // GET /api/analytics/consistency - Training consistency
        CROW_ROUTE(app, "/api/analytics/consistency")
        ([&analytics_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto days_param = req.url_params.get("days");
                int days = days_param ? std::stoi(days_param) : 30;

                auto points = analytics_service.get_training_consistency(ctx.user_id, days);

                json data = json::array();
                for (const auto& pt : points) {
                    data.push_back(pt.to_json());
                }

                res.write(json({{"data", data}}).dump());
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

        // GET /api/analytics/streaks - Streak data
        CROW_ROUTE(app, "/api/analytics/streaks")
        ([&analytics_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto streaks = analytics_service.get_streaks(ctx.user_id);
                res.write(streaks.to_json().dump());
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

        // POST /api/analytics/nutrition-checkin - Weekly nutrition check-in
        CROW_ROUTE(app, "/api/analytics/nutrition-checkin").methods("POST"_method)
        ([&analytics_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto recommendation = analytics_service.generate_nutrition_checkin(ctx.user_id);
                res.write(recommendation.to_json().dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                res.code = 400;
                res.write(json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Nutrition check-in failed"}
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

        // ── Nutrition Tracking Endpoints ──────────────────────────────

        // POST /api/nutrition/log - Log a food entry
        CROW_ROUTE(app, "/api/nutrition/log").methods("POST"_method)
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::NutritionService::LogFoodRequest food_req;
                food_req.food_name = body["food_name"].get<std::string>();
                food_req.calories = body["calories"].get<double>();
                food_req.protein_g = body["protein_g"].get<double>();
                food_req.carbs_g = body["carbs_g"].get<double>();
                food_req.fat_g = body["fat_g"].get<double>();

                if (body.contains("meal_type") && !body["meal_type"].is_null())
                    food_req.meal_type = body["meal_type"].get<std::string>();
                if (body.contains("brand") && !body["brand"].is_null())
                    food_req.brand = body["brand"].get<std::string>();
                if (body.contains("serving_size") && !body["serving_size"].is_null())
                    food_req.serving_size = body["serving_size"].get<double>();
                if (body.contains("serving_unit") && !body["serving_unit"].is_null())
                    food_req.serving_unit = body["serving_unit"].get<std::string>();
                if (body.contains("quantity") && !body["quantity"].is_null())
                    food_req.quantity = body["quantity"].get<double>();
                if (body.contains("fiber_g") && !body["fiber_g"].is_null())
                    food_req.fiber_g = body["fiber_g"].get<double>();
                if (body.contains("sugar_g") && !body["sugar_g"].is_null())
                    food_req.sugar_g = body["sugar_g"].get<double>();
                if (body.contains("sodium_mg") && !body["sodium_mg"].is_null())
                    food_req.sodium_mg = body["sodium_mg"].get<double>();
                if (body.contains("is_custom") && !body["is_custom"].is_null())
                    food_req.is_custom = body["is_custom"].get<bool>();
                if (body.contains("source") && !body["source"].is_null())
                    food_req.source = body["source"].get<std::string>();

                auto log = nutrition_service.log_food(ctx.user_id, food_req);
                res.code = 201;
                res.write(log.to_json().dump());
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

        // GET /api/nutrition/log?date=YYYY-MM-DD - Get logs for a date
        CROW_ROUTE(app, "/api/nutrition/log")
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto date_param = req.url_params.get("date");
                if (!date_param) {
                    res.code = 400;
                    res.write(json({
                        {"error", {{"code", "MISSING_DATE"}, {"message", "date parameter required"}}}
                    }).dump());
                    res.end();
                    return;
                }

                auto logs = nutrition_service.get_logs_for_date(ctx.user_id, date_param);
                json arr = json::array();
                for (const auto& log : logs) {
                    arr.push_back(log.to_json());
                }
                res.write(arr.dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // PUT /api/nutrition/log/:id - Update a nutrition log entry
        CROW_ROUTE(app, "/api/nutrition/log/<string>").methods("PUT"_method)
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& log_id) {
            try {
                auto body = json::parse(req.body);

                forge::NutritionService::UpdateFoodRequest update_req;
                if (body.contains("meal_type") && !body["meal_type"].is_null())
                    update_req.meal_type = body["meal_type"].get<std::string>();
                if (body.contains("food_name") && !body["food_name"].is_null())
                    update_req.food_name = body["food_name"].get<std::string>();
                if (body.contains("serving_size") && !body["serving_size"].is_null())
                    update_req.serving_size = body["serving_size"].get<double>();
                if (body.contains("serving_unit") && !body["serving_unit"].is_null())
                    update_req.serving_unit = body["serving_unit"].get<std::string>();
                if (body.contains("quantity") && !body["quantity"].is_null())
                    update_req.quantity = body["quantity"].get<double>();
                if (body.contains("calories") && !body["calories"].is_null())
                    update_req.calories = body["calories"].get<double>();
                if (body.contains("protein_g") && !body["protein_g"].is_null())
                    update_req.protein_g = body["protein_g"].get<double>();
                if (body.contains("carbs_g") && !body["carbs_g"].is_null())
                    update_req.carbs_g = body["carbs_g"].get<double>();
                if (body.contains("fat_g") && !body["fat_g"].is_null())
                    update_req.fat_g = body["fat_g"].get<double>();

                auto log = nutrition_service.update_log(ctx.user_id, log_id, update_req);
                res.write(log.to_json().dump());
                res.end();

            } catch (const std::invalid_argument& e) {
                res.code = 404;
                res.write(json({
                    {"error", {{"code", e.what()}, {"message", "Log not found"}}}
                }).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // DELETE /api/nutrition/log/:id - Delete a nutrition log entry
        CROW_ROUTE(app, "/api/nutrition/log/<string>").methods("DELETE"_method)
        ([&nutrition_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& log_id) {
            try {
                bool deleted = nutrition_service.delete_log(ctx.user_id, log_id);
                if (!deleted) {
                    res.code = 404;
                    res.write(json({
                        {"error", {{"code", "NOT_FOUND"}, {"message", "Log not found"}}}
                    }).dump());
                    res.end();
                    return;
                }
                res.write(json({{"success", true}}).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/nutrition/summary?date=YYYY-MM-DD - Daily summary
        CROW_ROUTE(app, "/api/nutrition/summary")
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto date_param = req.url_params.get("date");
                if (!date_param) {
                    res.code = 400;
                    res.write(json({
                        {"error", {{"code", "MISSING_DATE"}, {"message", "date parameter required"}}}
                    }).dump());
                    res.end();
                    return;
                }
                auto summary = nutrition_service.get_daily_summary(ctx.user_id, date_param);
                res.write(summary.to_json().dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/nutrition/summary/range?start=...&end=... - Range summary
        CROW_ROUTE(app, "/api/nutrition/summary/range")
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto start_param = req.url_params.get("start");
                auto end_param = req.url_params.get("end");
                if (!start_param || !end_param) {
                    res.code = 400;
                    res.write(json({
                        {"error", {{"code", "MISSING_PARAMS"}, {"message", "start and end required"}}}
                    }).dump());
                    res.end();
                    return;
                }
                auto summaries = nutrition_service.get_summary_range(ctx.user_id, start_param, end_param);
                json arr = json::array();
                for (const auto& s : summaries) {
                    arr.push_back(s.to_json());
                }
                res.write(arr.dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // POST /api/nutrition/foods - Create custom food
        CROW_ROUTE(app, "/api/nutrition/foods").methods("POST"_method)
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::NutritionService::CreateCustomFoodRequest food_req;
                food_req.name = body["name"].get<std::string>();
                food_req.serving_size = body["serving_size"].get<double>();
                food_req.serving_unit = body["serving_unit"].get<std::string>();
                food_req.calories = body["calories"].get<double>();
                food_req.protein_g = body["protein_g"].get<double>();
                food_req.carbs_g = body["carbs_g"].get<double>();
                food_req.fat_g = body["fat_g"].get<double>();
                if (body.contains("brand") && !body["brand"].is_null())
                    food_req.brand = body["brand"].get<std::string>();

                auto food = nutrition_service.create_custom_food(ctx.user_id, food_req);
                res.code = 201;
                res.write(food.to_json().dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/nutrition/foods - List custom foods
        CROW_ROUTE(app, "/api/nutrition/foods")
        ([&nutrition_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto foods = nutrition_service.get_custom_foods(ctx.user_id);
                json arr = json::array();
                for (const auto& f : foods) {
                    arr.push_back(f.to_json());
                }
                res.write(arr.dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // DELETE /api/nutrition/foods/:id - Delete custom food
        CROW_ROUTE(app, "/api/nutrition/foods/<string>").methods("DELETE"_method)
        ([&nutrition_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& food_id) {
            try {
                bool deleted = nutrition_service.delete_custom_food(ctx.user_id, food_id);
                if (!deleted) {
                    res.code = 404;
                    res.write(json({
                        {"error", {{"code", "NOT_FOUND"}, {"message", "Custom food not found"}}}
                    }).dump());
                    res.end();
                    return;
                }
                res.write(json({{"success", true}}).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/nutrition/recent - Recent foods
        CROW_ROUTE(app, "/api/nutrition/recent")
        ([&nutrition_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto foods = nutrition_service.get_recent_foods(ctx.user_id);
                json arr = json::array();
                for (const auto& f : foods) {
                    arr.push_back(f.to_json());
                }
                res.write(arr.dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/nutrition/search?q=... - Search USDA foods
        CROW_ROUTE(app, "/api/nutrition/search")
        ([&nutrition_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context&) {
            try {
                auto q = req.url_params.get("q");
                if (!q) {
                    res.code = 400;
                    res.write(json({
                        {"error", {{"code", "MISSING_QUERY"}, {"message", "q parameter required"}}}
                    }).dump());
                    res.end();
                    return;
                }
                auto foods = nutrition_service.search_usda(q);
                json arr = json::array();
                for (const auto& f : foods) {
                    arr.push_back(f.to_json());
                }
                res.write(arr.dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // ── Routine Endpoints ────────────────────────────────────────

        // POST /api/routines - Create a routine
        CROW_ROUTE(app, "/api/routines").methods("POST"_method)
        ([&routine_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::CreateRoutineRequest routine_req;
                routine_req.name = body["name"].get<std::string>();
                if (body.contains("description") && !body["description"].is_null())
                    routine_req.description = body["description"].get<std::string>();

                if (body.contains("exercises")) {
                    for (const auto& ex : body["exercises"]) {
                        forge::CreateRoutineExerciseRequest ex_req;
                        ex_req.exercise_id = ex["exercise_id"].get<std::string>();
                        if (ex.contains("notes") && !ex["notes"].is_null())
                            ex_req.notes = ex["notes"].get<std::string>();

                        if (ex.contains("sets")) {
                            for (const auto& s : ex["sets"]) {
                                forge::CreateRoutineSetRequest set_req;
                                if (s.contains("set_type") && !s["set_type"].is_null())
                                    set_req.set_type = s["set_type"].get<std::string>();
                                if (s.contains("target_reps") && !s["target_reps"].is_null())
                                    set_req.target_reps = s["target_reps"].get<int>();
                                if (s.contains("target_duration_seconds") && !s["target_duration_seconds"].is_null())
                                    set_req.target_duration_seconds = s["target_duration_seconds"].get<int>();
                                ex_req.sets.push_back(std::move(set_req));
                            }
                        }
                        routine_req.exercises.push_back(std::move(ex_req));
                    }
                }

                auto routine = routine_service.create_routine(ctx.user_id, routine_req);
                res.code = 201;
                res.write(routine.to_json().dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/routines - List routines
        CROW_ROUTE(app, "/api/routines")
        ([&routine_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx) {
            try {
                auto routines = routine_service.list_routines(ctx.user_id);
                json arr = json::array();
                for (const auto& r : routines) {
                    arr.push_back(r.to_json());
                }
                res.write(arr.dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // GET /api/routines/:id - Get routine detail
        CROW_ROUTE(app, "/api/routines/<string>")
        ([&routine_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& routine_id) {
            try {
                auto routine = routine_service.get_routine(ctx.user_id, routine_id);
                if (!routine) {
                    res.code = 404;
                    res.write(json({
                        {"error", {{"code", "NOT_FOUND"}, {"message", "Routine not found"}}}
                    }).dump());
                    res.end();
                    return;
                }
                res.write(routine->to_json().dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // PUT /api/routines/:id - Update routine
        CROW_ROUTE(app, "/api/routines/<string>").methods("PUT"_method)
        ([&routine_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& routine_id) {
            try {
                auto body = json::parse(req.body);

                forge::CreateRoutineRequest routine_req;
                routine_req.name = body["name"].get<std::string>();
                if (body.contains("description") && !body["description"].is_null())
                    routine_req.description = body["description"].get<std::string>();

                if (body.contains("exercises")) {
                    for (const auto& ex : body["exercises"]) {
                        forge::CreateRoutineExerciseRequest ex_req;
                        ex_req.exercise_id = ex["exercise_id"].get<std::string>();
                        if (ex.contains("notes") && !ex["notes"].is_null())
                            ex_req.notes = ex["notes"].get<std::string>();

                        if (ex.contains("sets")) {
                            for (const auto& s : ex["sets"]) {
                                forge::CreateRoutineSetRequest set_req;
                                if (s.contains("set_type") && !s["set_type"].is_null())
                                    set_req.set_type = s["set_type"].get<std::string>();
                                if (s.contains("target_reps") && !s["target_reps"].is_null())
                                    set_req.target_reps = s["target_reps"].get<int>();
                                if (s.contains("target_duration_seconds") && !s["target_duration_seconds"].is_null())
                                    set_req.target_duration_seconds = s["target_duration_seconds"].get<int>();
                                ex_req.sets.push_back(std::move(set_req));
                            }
                        }
                        routine_req.exercises.push_back(std::move(ex_req));
                    }
                }

                auto routine = routine_service.update_routine(ctx.user_id, routine_id, routine_req);
                if (!routine) {
                    res.code = 404;
                    res.write(json({
                        {"error", {{"code", "NOT_FOUND"}, {"message", "Routine not found"}}}
                    }).dump());
                    res.end();
                    return;
                }
                res.write(routine->to_json().dump());
                res.end();

            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // DELETE /api/routines/:id - Delete routine
        CROW_ROUTE(app, "/api/routines/<string>").methods("DELETE"_method)
        ([&routine_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& routine_id) {
            try {
                bool deleted = routine_service.delete_routine(ctx.user_id, routine_id);
                if (!deleted) {
                    res.code = 404;
                    res.write(json({
                        {"error", {{"code", "NOT_FOUND"}, {"message", "Routine not found"}}}
                    }).dump());
                    res.end();
                    return;
                }
                res.write(json({{"success", true}}).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // POST /api/routines/:id/start - Start workout from routine
        CROW_ROUTE(app, "/api/routines/<string>/start").methods("POST"_method)
        ([&routine_service, &workout_service](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& routine_id) {
            try {
                std::string workout_id = routine_service.start_from_routine(ctx.user_id, routine_id);

                // Return full workout object as frontend expects
                auto workout = workout_service.get_workout(workout_id, ctx.user_id);
                res.code = 201;
                if (workout) {
                    res.write(workout->to_json().dump());
                } else {
                    res.write(json({
                        {"id", workout_id},
                        {"status", "in_progress"},
                        {"sets", json::array()}
                    }).dump());
                }
                res.end();
            } catch (const std::invalid_argument& e) {
                res.code = 404;
                res.write(json({
                    {"error", {{"code", e.what()}, {"message", "Routine not found"}}}
                }).dump());
                res.end();
            } catch (const std::exception& e) {
                res.code = 500;
                res.write(json({
                    {"error", {{"code", "INTERNAL_ERROR"}, {"message", e.what()}}}
                }).dump());
                res.end();
            }
        });

        // ── Stub Endpoints (Food Recognition & Coaching) ─────────────

        // POST /api/nutrition/recognize - Food recognition stub
        CROW_ROUTE(app, "/api/nutrition/recognize").methods("POST"_method)
        ([](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context&) {
            res.code = 200;
            res.write(json({
                {"task_id", "stub-not-implemented"}
            }).dump());
            res.end();
        });

        // GET /api/nutrition/recognize/:id - Food recognition status stub
        CROW_ROUTE(app, "/api/nutrition/recognize/<string>")
        ([](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context&, const std::string&) {
            res.code = 200;
            res.write(json({
                {"status", "not_implemented"},
                {"message", "Food recognition coming soon"}
            }).dump());
            res.end();
        });

        // POST /api/nutrition/recognize/:id/confirm - Confirm recognition stub
        CROW_ROUTE(app, "/api/nutrition/recognize/<string>/confirm").methods("POST"_method)
        ([](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context&, const std::string&) {
            res.code = 200;
            res.write(json({
                {"success", true},
                {"message", "Confirmed (stub)"}
            }).dump());
            res.end();
        });

        // POST /api/coaching/weekly-checkin - Coaching stub
        CROW_ROUTE(app, "/api/coaching/weekly-checkin").methods("POST"_method)
        ([](const crow::request&, crow::response& res,
          forge::AuthMiddleware::context&) {
            res.code = 200;
            res.write(json({
                {"message", "Coaching feature coming soon"}
            }).dump());
            res.end();
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
