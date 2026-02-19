#include <crow.h>
#include "../include/config.hpp"
#include "../include/database.hpp"
#include "../include/jwt.hpp"
#include "../include/video_service.hpp"
#include "../include/plate_calculator.hpp"
#include "../include/warmup_planner.hpp"
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
