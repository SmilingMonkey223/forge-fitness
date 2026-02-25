#include <crow.h>
#include "../include/config.hpp"
#include "../include/database.hpp"
#include "../include/jwt.hpp"
#include "../include/nutrition_service.hpp"
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

        // ── Nutrition Log CRUD ────────────────────────────────────────

        // POST /api/nutrition/log - Log a food entry
        CROW_ROUTE(app, "/api/nutrition/log").methods("POST"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                auto body = json::parse(req.body);

                forge::NutritionService::LogFoodRequest log_req;
                log_req.food_name = body.at("food_name").get<std::string>();
                log_req.calories = body.at("calories").get<double>();
                log_req.protein_g = body.at("protein_g").get<double>();
                log_req.carbs_g = body.at("carbs_g").get<double>();
                log_req.fat_g = body.at("fat_g").get<double>();
                log_req.serving_size = body.at("serving_size").get<double>();
                log_req.serving_unit = body.value("serving_unit", "g");
                log_req.quantity = body.value("quantity", 1.0);

                if (body.contains("meal_type") && !body["meal_type"].is_null()) {
                    log_req.meal_type = body["meal_type"].get<std::string>();
                }
                if (body.contains("logged_at") && !body["logged_at"].is_null()) {
                    log_req.logged_at = body["logged_at"].get<std::string>();
                }
                if (body.contains("brand") && !body["brand"].is_null()) {
                    log_req.brand = body["brand"].get<std::string>();
                }
                if (body.contains("fiber_g") && !body["fiber_g"].is_null()) {
                    log_req.fiber_g = body["fiber_g"].get<double>();
                }
                if (body.contains("sugar_g") && !body["sugar_g"].is_null()) {
                    log_req.sugar_g = body["sugar_g"].get<double>();
                }
                if (body.contains("sodium_mg") && !body["sodium_mg"].is_null()) {
                    log_req.sodium_mg = body["sodium_mg"].get<double>();
                }
                if (body.contains("is_custom")) {
                    log_req.is_custom = body["is_custom"].get<bool>();
                }
                if (body.contains("source") && !body["source"].is_null()) {
                    log_req.source = body["source"].get<std::string>();
                }

                auto entry = forge::NutritionService::log_food(ctx.user_id, log_req);

                auto response = crow::response(201, json({
                    {"entry", entry.to_json()}
                }).dump());
                response.add_header("Content-Type", "application/json");
                return response;

            } catch (const std::invalid_argument& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Failed to log food entry"}
                    }}
                }).dump());
            } catch (const json::exception& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", "INVALID_REQUEST"},
                        {"message", std::string("Invalid request body: ") + e.what()}
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

        // GET /api/nutrition/log?date=YYYY-MM-DD - Get entries for a date
        CROW_ROUTE(app, "/api/nutrition/log").methods("GET"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                std::string date = req.url_params.get("date") ? req.url_params.get("date") : "";
                if (date.empty()) {
                    return crow::response(400, json({
                        {"error", {
                            {"code", "MISSING_DATE"},
                            {"message", "date query parameter is required (YYYY-MM-DD)"}
                        }}
                    }).dump());
                }

                auto logs = forge::NutritionService::get_logs_for_date(ctx.user_id, date);

                json entries = json::array();
                for (const auto& log : logs) {
                    entries.push_back(log.to_json());
                }

                auto response = crow::response(200, json({
                    {"entries", entries},
                    {"count", static_cast<int>(logs.size())}
                }).dump());
                response.add_header("Content-Type", "application/json");
                return response;

            } catch (const std::invalid_argument& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Failed to retrieve nutrition log"}
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

        // PUT /api/nutrition/log/<string> - Update an entry
        CROW_ROUTE(app, "/api/nutrition/log/<string>").methods("PUT"_method)
        ([&app](const crow::request& req, const std::string& log_id) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                auto body = json::parse(req.body);

                forge::NutritionService::UpdateFoodRequest update_req;

                if (body.contains("food_name") && !body["food_name"].is_null()) {
                    update_req.food_name = body["food_name"].get<std::string>();
                }
                if (body.contains("calories") && !body["calories"].is_null()) {
                    update_req.calories = body["calories"].get<double>();
                }
                if (body.contains("protein_g") && !body["protein_g"].is_null()) {
                    update_req.protein_g = body["protein_g"].get<double>();
                }
                if (body.contains("carbs_g") && !body["carbs_g"].is_null()) {
                    update_req.carbs_g = body["carbs_g"].get<double>();
                }
                if (body.contains("fat_g") && !body["fat_g"].is_null()) {
                    update_req.fat_g = body["fat_g"].get<double>();
                }
                if (body.contains("serving_size") && !body["serving_size"].is_null()) {
                    update_req.serving_size = body["serving_size"].get<double>();
                }
                if (body.contains("serving_unit") && !body["serving_unit"].is_null()) {
                    update_req.serving_unit = body["serving_unit"].get<std::string>();
                }
                if (body.contains("quantity") && !body["quantity"].is_null()) {
                    update_req.quantity = body["quantity"].get<double>();
                }
                if (body.contains("meal_type") && !body["meal_type"].is_null()) {
                    update_req.meal_type = body["meal_type"].get<std::string>();
                }
                if (body.contains("logged_at") && !body["logged_at"].is_null()) {
                    update_req.logged_at = body["logged_at"].get<std::string>();
                }
                if (body.contains("brand") && !body["brand"].is_null()) {
                    update_req.brand = body["brand"].get<std::string>();
                }
                if (body.contains("fiber_g") && !body["fiber_g"].is_null()) {
                    update_req.fiber_g = body["fiber_g"].get<double>();
                }
                if (body.contains("sugar_g") && !body["sugar_g"].is_null()) {
                    update_req.sugar_g = body["sugar_g"].get<double>();
                }
                if (body.contains("sodium_mg") && !body["sodium_mg"].is_null()) {
                    update_req.sodium_mg = body["sodium_mg"].get<double>();
                }

                auto entry = forge::NutritionService::update_log(ctx.user_id, log_id, update_req);

                auto response = crow::response(200, json({
                    {"entry", entry.to_json()}
                }).dump());
                response.add_header("Content-Type", "application/json");
                return response;

            } catch (const std::invalid_argument& e) {
                std::string code = e.what();
                int status = (code == "LOG_NOT_FOUND") ? 404 : 400;
                return crow::response(status, json({
                    {"error", {
                        {"code", code},
                        {"message", "Failed to update nutrition log entry"}
                    }}
                }).dump());
            } catch (const json::exception& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", "INVALID_REQUEST"},
                        {"message", std::string("Invalid request body: ") + e.what()}
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

        // DELETE /api/nutrition/log/<string> - Delete an entry (soft delete)
        CROW_ROUTE(app, "/api/nutrition/log/<string>").methods("DELETE"_method)
        ([&app](const crow::request& req, const std::string& log_id) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                forge::NutritionService::delete_log(ctx.user_id, log_id);

                return crow::response(200, json({
                    {"message", "Entry deleted successfully"}
                }).dump());

            } catch (const std::invalid_argument& e) {
                std::string code = e.what();
                int status = (code == "LOG_NOT_FOUND") ? 404 : 400;
                return crow::response(status, json({
                    {"error", {
                        {"code", code},
                        {"message", "Failed to delete nutrition log entry"}
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

        // ── Daily Summary ───────────────────────────────────────────

        // GET /api/nutrition/summary?date=YYYY-MM-DD - Daily totals
        CROW_ROUTE(app, "/api/nutrition/summary").methods("GET"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                std::string date = req.url_params.get("date") ? req.url_params.get("date") : "";
                if (date.empty()) {
                    return crow::response(400, json({
                        {"error", {
                            {"code", "MISSING_DATE"},
                            {"message", "date query parameter is required (YYYY-MM-DD)"}
                        }}
                    }).dump());
                }

                auto summary = forge::NutritionService::get_daily_summary(ctx.user_id, date);

                return crow::response(200, json({
                    {"summary", summary.to_json()}
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

        // GET /api/nutrition/summary/range?start=YYYY-MM-DD&end=YYYY-MM-DD
        CROW_ROUTE(app, "/api/nutrition/summary/range").methods("GET"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                std::string start_date = req.url_params.get("start") ? req.url_params.get("start") : "";
                std::string end_date = req.url_params.get("end") ? req.url_params.get("end") : "";

                if (start_date.empty() || end_date.empty()) {
                    return crow::response(400, json({
                        {"error", {
                            {"code", "MISSING_DATE_RANGE"},
                            {"message", "start and end query parameters are required (YYYY-MM-DD)"}
                        }}
                    }).dump());
                }

                auto summaries = forge::NutritionService::get_summary_range(ctx.user_id, start_date, end_date);

                json days = json::array();
                for (const auto& s : summaries) {
                    days.push_back(s.to_json());
                }

                return crow::response(200, json({
                    {"summaries", days},
                    {"count", static_cast<int>(summaries.size())}
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

        // ── Custom Foods ────────────────────────────────────────────

        // POST /api/nutrition/foods - Create custom food
        CROW_ROUTE(app, "/api/nutrition/foods").methods("POST"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                auto body = json::parse(req.body);

                forge::NutritionService::CreateCustomFoodRequest food_req;
                food_req.name = body.at("name").get<std::string>();
                food_req.serving_size = body.at("serving_size").get<double>();
                food_req.serving_unit = body.value("serving_unit", "g");
                food_req.calories = body.at("calories").get<double>();
                food_req.protein_g = body.at("protein_g").get<double>();
                food_req.carbs_g = body.at("carbs_g").get<double>();
                food_req.fat_g = body.at("fat_g").get<double>();

                if (body.contains("brand") && !body["brand"].is_null()) {
                    food_req.brand = body["brand"].get<std::string>();
                }
                if (body.contains("fiber_g") && !body["fiber_g"].is_null()) {
                    food_req.fiber_g = body["fiber_g"].get<double>();
                }
                if (body.contains("sugar_g") && !body["sugar_g"].is_null()) {
                    food_req.sugar_g = body["sugar_g"].get<double>();
                }
                if (body.contains("sodium_mg") && !body["sodium_mg"].is_null()) {
                    food_req.sodium_mg = body["sodium_mg"].get<double>();
                }

                auto food = forge::NutritionService::create_custom_food(ctx.user_id, food_req);

                return crow::response(201, json({
                    {"food", food.to_json()}
                }).dump());

            } catch (const std::invalid_argument& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", e.what()},
                        {"message", "Failed to create custom food"}
                    }}
                }).dump());
            } catch (const json::exception& e) {
                return crow::response(400, json({
                    {"error", {
                        {"code", "INVALID_REQUEST"},
                        {"message", std::string("Invalid request body: ") + e.what()}
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

        // GET /api/nutrition/foods - List user's custom foods
        CROW_ROUTE(app, "/api/nutrition/foods").methods("GET"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                auto foods = forge::NutritionService::get_custom_foods(ctx.user_id);

                json food_list = json::array();
                for (const auto& f : foods) {
                    food_list.push_back(f.to_json());
                }

                return crow::response(200, json({
                    {"foods", food_list},
                    {"count", static_cast<int>(foods.size())}
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

        // DELETE /api/nutrition/foods/<string> - Delete custom food
        CROW_ROUTE(app, "/api/nutrition/foods/<string>").methods("DELETE"_method)
        ([&app](const crow::request& req, const std::string& food_id) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                forge::NutritionService::delete_custom_food(ctx.user_id, food_id);

                return crow::response(200, json({
                    {"message", "Custom food deleted successfully"}
                }).dump());

            } catch (const std::invalid_argument& e) {
                std::string code = e.what();
                int status = (code == "FOOD_NOT_FOUND") ? 404 : 400;
                return crow::response(status, json({
                    {"error", {
                        {"code", code},
                        {"message", "Failed to delete custom food"}
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

        // ── Recent / Frequent Foods ─────────────────────────────────

        // GET /api/nutrition/recent - Get user's most frequently logged foods
        CROW_ROUTE(app, "/api/nutrition/recent").methods("GET"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                auto foods = forge::NutritionService::get_recent_foods(ctx.user_id);

                json food_list = json::array();
                for (const auto& f : foods) {
                    food_list.push_back(f.to_json());
                }

                return crow::response(200, json({
                    {"foods", food_list},
                    {"count", static_cast<int>(foods.size())}
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

        // ── USDA Food Search ────────────────────────────────────────

        // GET /api/nutrition/search?q=chicken+breast - Search USDA database
        CROW_ROUTE(app, "/api/nutrition/search").methods("GET"_method)
        ([&app](const crow::request& req) {
            auto& ctx = app.get_context<forge::AuthMiddleware>(req);
            try {
                std::string query = req.url_params.get("q") ? req.url_params.get("q") : "";
                if (query.empty()) {
                    return crow::response(400, json({
                        {"error", {
                            {"code", "MISSING_QUERY"},
                            {"message", "q query parameter is required"}
                        }}
                    }).dump());
                }

                auto foods = forge::NutritionService::search_usda(query);

                json food_list = json::array();
                for (const auto& f : foods) {
                    food_list.push_back(f.to_json());
                }

                return crow::response(200, json({
                    {"foods", food_list},
                    {"count", static_cast<int>(foods.size())},
                    {"query", query}
                }).dump());

            } catch (const std::runtime_error& e) {
                return crow::response(503, json({
                    {"error", {
                        {"code", "SEARCH_UNAVAILABLE"},
                        {"message", e.what()}
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
