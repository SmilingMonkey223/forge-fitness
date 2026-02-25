#include <crow.h>
#include "../include/config.hpp"
#include "../include/database.hpp"
#include "../include/jwt.hpp"
#include "../include/profile_service.hpp"
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
