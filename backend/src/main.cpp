#include <crow.h>
#include "../include/config.hpp"
#include "../include/database.hpp"
#include "../include/jwt.hpp"
#include "../include/video_service.hpp"
#include "../include/plate_calculator.hpp"
#include "../include/warmup_planner.hpp"
#include "../include/program_service.hpp"
#include "../include/social_service.hpp"
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
        forge::VideoService video_service(forge::Database::instance());
        forge::ProgramService program_service(forge::Database::instance());

        // Video endpoints
        // GET /api/exercises/:exercise_id/videos
        CROW_ROUTE(app, "/api/exercises/<string>/videos")
        ([&video_service](const std::string& exercise_id) {
            try {
                auto videos = video_service.get_exercise_videos(exercise_id);

                json response_data = json::array();
                for (const auto& video : videos) {
                    response_data.push_back(video.to_json());
                }

                return crow::response(200, json({
                    {"exercise_id", exercise_id},
                    {"videos", response_data}
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

        // GET /api/videos/:video_id
        CROW_ROUTE(app, "/api/videos/<string>")
        ([&video_service](const std::string& video_id) {
            try {
                auto video_opt = video_service.get_video_by_id(video_id);

                if (!video_opt) {
                    return crow::response(404, json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Video not found"}
                        }}
                    }).dump());
                }

                return crow::response(200, video_opt->to_json().dump());

            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        // POST /api/videos/:video_id/view (track analytics)
        CROW_ROUTE(app, "/api/videos/<string>/view").methods("POST"_method)
        ([&video_service](const crow::request& req, crow::response& res,
          forge::AuthMiddleware::context& ctx, const std::string& video_id) {
            try {
                auto body = json::parse(req.body);
                int watch_time = body.value("watch_time_seconds", 0);
                bool completed = body.value("completed", false);

                video_service.track_video_view(video_id, ctx.user_id, watch_time, completed);

                res.write(json({
                    {"success", true},
                    {"message", "View tracked"}
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

        // Utility endpoints
        // POST /api/utils/plate-calculator
        CROW_ROUTE(app, "/api/utils/plate-calculator").methods("POST"_method)
        ([](const crow::request& req) {
            try {
                auto body = json::parse(req.body);
                double target_weight = body["target_weight"];
                std::string units = body.value("units", "lbs");

                // Select appropriate configuration
                forge::PlateConfiguration config = (units == "kg") ?
                    forge::PlateCalculator::standard_kg() :
                    forge::PlateCalculator::standard_lbs();

                // Allow custom bar weight if provided
                if (body.contains("bar_weight")) {
                    config.bar_weight = body["bar_weight"];
                }

                auto result = forge::PlateCalculator::calculate(target_weight, config);

                return crow::response(200, result.to_json().dump());

            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        // POST /api/utils/warmup-planner
        CROW_ROUTE(app, "/api/utils/warmup-planner").methods("POST"_method)
        ([](const crow::request& req) {
            try {
                auto body = json::parse(req.body);
                double working_weight = body["working_weight"];
                int working_reps = body.value("working_reps", 5);
                double bar_weight = body.value("bar_weight", 45.0);

                auto warmup_sets = forge::WarmupPlanner::generate_warmup(
                    working_weight,
                    working_reps,
                    bar_weight
                );

                json warmup_json = json::array();
                for (const auto& set : warmup_sets) {
                    warmup_json.push_back(set.to_json());
                }

                return crow::response(200, json({
                    {"working_weight", working_weight},
                    {"working_reps", working_reps},
                    {"warmup_sets", warmup_json}
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

        // Program endpoints
        // GET /api/programs - List all programs
        CROW_ROUTE(app, "/api/programs")
        ([&program_service](const crow::request& req) {
            try {
                auto goal = req.url_params.get("goal");
                auto level = req.url_params.get("experience_level");
                auto official_only = req.url_params.get("official_only");

                std::optional<std::string> goal_opt = goal ? std::optional<std::string>(goal) : std::nullopt;
                std::optional<std::string> level_opt = level ? std::optional<std::string>(level) : std::nullopt;
                bool official = official_only && std::string(official_only) == "true";

                auto programs = program_service.get_all_programs(goal_opt, level_opt, official);

                json programs_json = json::array();
                for (const auto& program : programs) {
                    programs_json.push_back(program.to_json());
                }

                return crow::response(200, json({
                    {"programs", programs_json}
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

        // GET /api/programs/:id - Get program details
        CROW_ROUTE(app, "/api/programs/<string>")
        ([&program_service](const std::string& program_id) {
            try {
                auto program = program_service.get_program_by_id(program_id);

                if (!program) {
                    return crow::response(404, json({
                        {"error", {
                            {"code", "NOT_FOUND"},
                            {"message", "Program not found"}
                        }}
                    }).dump());
                }

                return crow::response(200, program->to_json().dump());

            } catch (const std::exception& e) {
                return crow::response(500, json({
                    {"error", {
                        {"code", "INTERNAL_ERROR"},
                        {"message", e.what()}
                    }}
                }).dump());
            }
        });

        // GET /api/programs/:id/workouts?week=1 - Get workouts for a week
        CROW_ROUTE(app, "/api/programs/<string>/workouts")
        ([&program_service](const crow::request& req, const std::string& program_id) {
            try {
                auto week_param = req.url_params.get("week");
                int week = week_param ? std::stoi(week_param) : 1;

                auto workouts = program_service.get_program_workouts(program_id, week);

                json workouts_json = json::array();
                for (const auto& workout : workouts) {
                    workouts_json.push_back(workout.to_json());
                }

                return crow::response(200, json({
                    {"program_id", program_id},
                    {"week_number", week},
                    {"workouts", workouts_json}
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

        // POST /api/coaching/questionnaire - Save AI coaching questionnaire
        CROW_ROUTE(app, "/api/coaching/questionnaire").methods("POST"_method)
        ([&program_service](const crow::request& req, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::CoachingQuestionnaire q;
                q.user_id = ctx.user_id;
                q.experience_level = body["experience_level"];
                q.years_training = body["years_training"];
                q.current_frequency = body["current_frequency"];
                q.primary_goal = body["primary_goal"];
                if (body.contains("secondary_goal")) q.secondary_goal = body["secondary_goal"];
                if (body.contains("goal_timeframe_weeks")) q.goal_timeframe_weeks = body["goal_timeframe_weeks"];
                q.available_days_per_week = body["available_days_per_week"];
                q.session_duration_minutes = body["session_duration_minutes"];
                q.available_equipment = body.value("available_equipment", json::array());
                if (body.contains("injuries_or_limitations")) q.injuries_or_limitations = body["injuries_or_limitations"];
                q.exercises_to_avoid = body.value("exercises_to_avoid", json::array());
                if (body.contains("preferred_rep_range")) q.preferred_rep_range = body["preferred_rep_range"];
                if (body.contains("preferred_intensity")) q.preferred_intensity = body["preferred_intensity"];

                std::string id = program_service.save_questionnaire(q);

                res.code = 201;
                res.write(json({
                    {"id", id},
                    {"message", "Questionnaire saved successfully"}
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

        // POST /api/coaching/generate-program - Generate program from questionnaire
        CROW_ROUTE(app, "/api/coaching/generate-program").methods("POST"_method)
        ([&program_service](const crow::request&, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                std::string program_id = program_service.generate_program_from_questionnaire(ctx.user_id);

                res.code = 201;
                res.write(json({
                    {"program_id", program_id},
                    {"message", "Program generated and assigned successfully"}
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

        // GET /api/coaching/state - Get user coaching state
        CROW_ROUTE(app, "/api/coaching/state")
        ([&program_service](const crow::request&, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto state = program_service.get_coaching_state(ctx.user_id);

                if (!state) {
                    res.write(json({
                        {"mode", "tracker"},
                        {"current_program_id", nullptr}
                    }).dump());
                } else {
                    res.write(state->to_json().dump());
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

        // POST /api/coaching/start-program - Start a program
        CROW_ROUTE(app, "/api/coaching/start-program").methods("POST"_method)
        ([&program_service](const crow::request& req, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);
                std::string program_id = body["program_id"];

                program_service.start_program(ctx.user_id, program_id);

                res.write(json({
                    {"success", true},
                    {"message", "Program started successfully"}
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

        // POST /api/coaching/weekly-checkin - Submit weekly check-in
        CROW_ROUTE(app, "/api/coaching/weekly-checkin").methods("POST"_method)
        ([&program_service](const crow::request& req, crow::response& res, forge::AuthMiddleware::context& ctx) {
            try {
                auto body = json::parse(req.body);

                forge::WeeklyCheckIn checkin;
                checkin.user_id = ctx.user_id;
                checkin.week_start = body["week_start"];
                checkin.week_end = body["week_end"];
                if (body.contains("hunger_rating")) checkin.hunger_rating = body["hunger_rating"];
                if (body.contains("energy_rating")) checkin.energy_rating = body["energy_rating"];
                if (body.contains("recovery_rating")) checkin.recovery_rating = body["recovery_rating"];
                if (body.contains("sleep_quality_rating")) checkin.sleep_quality_rating = body["sleep_quality_rating"];
                if (body.contains("average_weight_lbs")) checkin.average_weight_lbs = body["average_weight_lbs"];
                if (body.contains("weight_change_lbs")) checkin.weight_change_lbs = body["weight_change_lbs"];
                checkin.total_workouts_completed = body.value("total_workouts_completed", 0);
                checkin.total_sets_completed = body.value("total_sets_completed", 0);
                if (body.contains("notes")) checkin.notes = body["notes"];

                std::string id = program_service.submit_checkin(checkin);

                res.code = 201;
                res.write(json({
                    {"id", id},
                    {"message", "Check-in submitted successfully"}
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
