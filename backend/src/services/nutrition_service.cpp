#include "../../include/nutrition_service.hpp"
#include "../../include/uuid.hpp"
#include <chrono>
#include <ctime>
#include <curl/curl.h>
#include <sstream>

namespace forge {

// Implement NutritionLog::to_json() (declared in models.hpp)
json NutritionLog::to_json() const {
    auto time_t_val = std::chrono::system_clock::to_time_t(logged_at);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&time_t_val));

    json j = {
        {"id", id},
        {"user_id", user_id},
        {"logged_at", std::string(buf)},
        {"food_name", food_name},
        {"serving_size", serving_size},
        {"serving_unit", serving_unit},
        {"quantity", quantity},
        {"calories", calories},
        {"protein_g", protein_g},
        {"carbs_g", carbs_g},
        {"fat_g", fat_g},
        {"is_custom", is_custom},
        {"source", source}
    };

    if (meal_type) j["meal_type"] = *meal_type;
    if (brand) j["brand"] = *brand;
    if (fiber_g) j["fiber_g"] = *fiber_g;
    if (sugar_g) j["sugar_g"] = *sugar_g;
    if (sodium_mg) j["sodium_mg"] = *sodium_mg;

    return j;
}

static size_t curl_write_callback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

static std::chrono::system_clock::time_point parse_timestamp(const std::string& ts) {
    std::tm tm{};
    std::istringstream ss(ts);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

static NutritionLog row_to_log(const pqxx::row& row) {
    NutritionLog log;
    log.id = row["id"].as<std::string>();
    log.user_id = row["user_id"].as<std::string>();
    log.logged_at = parse_timestamp(row["logged_at"].as<std::string>());
    if (!row["meal_type"].is_null()) log.meal_type = row["meal_type"].as<std::string>();
    log.food_name = row["food_name"].as<std::string>();
    if (!row["brand"].is_null()) log.brand = row["brand"].as<std::string>();
    log.serving_size = row["serving_size"].as<double>();
    log.serving_unit = row["serving_unit"].as<std::string>();
    log.quantity = row["quantity"].as<double>();
    log.calories = row["calories"].as<double>();
    log.protein_g = row["protein_g"].as<double>();
    log.carbs_g = row["carbs_g"].as<double>();
    log.fat_g = row["fat_g"].as<double>();
    if (!row["fiber_g"].is_null()) log.fiber_g = row["fiber_g"].as<double>();
    if (!row["sugar_g"].is_null()) log.sugar_g = row["sugar_g"].as<double>();
    if (!row["sodium_mg"].is_null()) log.sodium_mg = row["sodium_mg"].as<double>();
    log.is_custom = row["is_custom"].as<bool>();
    log.source = row["source"].as<std::string>();
    return log;
}

NutritionLog NutritionService::log_food(const std::string& user_id, const LogFoodRequest& req) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    std::string id = UUID::generate();

    // Build SQL with proper NULL handling
    std::string sql = "INSERT INTO nutrition_log (id, user_id, meal_type, food_name, brand, "
        "serving_size, serving_unit, quantity, calories, protein_g, carbs_g, fat_g, "
        "fiber_g, sugar_g, sodium_mg, is_custom, source) VALUES ("
        + txn.quote(id) + ", " + txn.quote(user_id) + ", "
        + (req.meal_type ? txn.quote(*req.meal_type) : "NULL") + ", "
        + txn.quote(req.food_name) + ", "
        + (req.brand ? txn.quote(*req.brand) : "NULL") + ", "
        + std::to_string(req.serving_size) + ", " + txn.quote(req.serving_unit) + ", "
        + std::to_string(req.quantity) + ", "
        + std::to_string(req.calories) + ", " + std::to_string(req.protein_g) + ", "
        + std::to_string(req.carbs_g) + ", " + std::to_string(req.fat_g) + ", "
        + (req.fiber_g ? std::to_string(*req.fiber_g) : "NULL") + ", "
        + (req.sugar_g ? std::to_string(*req.sugar_g) : "NULL") + ", "
        + (req.sodium_mg ? std::to_string(*req.sodium_mg) : "NULL") + ", "
        + (req.is_custom ? "true" : "false") + ", " + txn.quote(req.source)
        + ") RETURNING *";

    auto r = txn.exec(sql);
    txn.commit();
    return row_to_log(r[0]);
}

std::vector<NutritionLog> NutritionService::get_logs_for_date(
    const std::string& user_id, const std::string& date
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT * FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date "
        "ORDER BY logged_at DESC",
        user_id, date
    );

    txn.commit();

    std::vector<NutritionLog> logs;
    for (const auto& row : r) {
        logs.push_back(row_to_log(row));
    }
    return logs;
}

NutritionLog NutritionService::update_log(
    const std::string& user_id, const std::string& log_id, const UpdateFoodRequest& req
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Build dynamic update
    std::vector<std::string> updates;
    int param_idx = 3;

    std::string sql = "UPDATE nutrition_log SET ";
    std::vector<std::string> set_clauses;

    if (req.meal_type) set_clauses.push_back("meal_type = " + txn.quote(*req.meal_type));
    if (req.food_name) set_clauses.push_back("food_name = " + txn.quote(*req.food_name));
    if (req.serving_size) set_clauses.push_back("serving_size = " + std::to_string(*req.serving_size));
    if (req.serving_unit) set_clauses.push_back("serving_unit = " + txn.quote(*req.serving_unit));
    if (req.quantity) set_clauses.push_back("quantity = " + std::to_string(*req.quantity));
    if (req.calories) set_clauses.push_back("calories = " + std::to_string(*req.calories));
    if (req.protein_g) set_clauses.push_back("protein_g = " + std::to_string(*req.protein_g));
    if (req.carbs_g) set_clauses.push_back("carbs_g = " + std::to_string(*req.carbs_g));
    if (req.fat_g) set_clauses.push_back("fat_g = " + std::to_string(*req.fat_g));

    if (set_clauses.empty()) {
        // Nothing to update, just return current
        auto r = txn.exec_params(
            "SELECT * FROM nutrition_log WHERE id = $1 AND user_id = $2",
            log_id, user_id
        );
        txn.commit();
        if (r.empty()) throw std::invalid_argument("LOG_NOT_FOUND");
        return row_to_log(r[0]);
    }

    sql += set_clauses[0];
    for (size_t i = 1; i < set_clauses.size(); ++i) {
        sql += ", " + set_clauses[i];
    }
    sql += " WHERE id = " + txn.quote(log_id) + " AND user_id = " + txn.quote(user_id);
    sql += " RETURNING *";

    auto r = txn.exec(sql);
    txn.commit();

    if (r.empty()) throw std::invalid_argument("LOG_NOT_FOUND");
    return row_to_log(r[0]);
}

bool NutritionService::delete_log(const std::string& user_id, const std::string& log_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "DELETE FROM nutrition_log WHERE id = $1 AND user_id = $2 RETURNING id",
        log_id, user_id
    );

    txn.commit();
    return !r.empty();
}

DailySummary NutritionService::get_daily_summary(
    const std::string& user_id, const std::string& date
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT COALESCE(SUM(calories * quantity), 0) AS total_calories, "
        "       COALESCE(SUM(protein_g * quantity), 0) AS total_protein_g, "
        "       COALESCE(SUM(carbs_g * quantity), 0) AS total_carbs_g, "
        "       COALESCE(SUM(fat_g * quantity), 0) AS total_fat_g, "
        "       COUNT(*) AS meal_count "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date",
        user_id, date
    );

    txn.commit();

    DailySummary ds;
    ds.date = date;
    ds.total_calories = r[0]["total_calories"].as<double>(0);
    ds.total_protein_g = r[0]["total_protein_g"].as<double>(0);
    ds.total_carbs_g = r[0]["total_carbs_g"].as<double>(0);
    ds.total_fat_g = r[0]["total_fat_g"].as<double>(0);
    ds.meal_count = r[0]["meal_count"].as<int>(0);
    return ds;
}

std::vector<DailySummary> NutritionService::get_summary_range(
    const std::string& user_id, const std::string& start, const std::string& end
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT DATE(logged_at) AS date, "
        "       COALESCE(SUM(calories * quantity), 0) AS total_calories, "
        "       COALESCE(SUM(protein_g * quantity), 0) AS total_protein_g, "
        "       COALESCE(SUM(carbs_g * quantity), 0) AS total_carbs_g, "
        "       COALESCE(SUM(fat_g * quantity), 0) AS total_fat_g, "
        "       COUNT(*) AS meal_count "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) BETWEEN $2::date AND $3::date "
        "GROUP BY DATE(logged_at) ORDER BY DATE(logged_at)",
        user_id, start, end
    );

    txn.commit();

    std::vector<DailySummary> result;
    for (const auto& row : r) {
        DailySummary ds;
        ds.date = row["date"].as<std::string>().substr(0, 10);
        ds.total_calories = row["total_calories"].as<double>(0);
        ds.total_protein_g = row["total_protein_g"].as<double>(0);
        ds.total_carbs_g = row["total_carbs_g"].as<double>(0);
        ds.total_fat_g = row["total_fat_g"].as<double>(0);
        ds.meal_count = row["meal_count"].as<int>(0);
        result.push_back(std::move(ds));
    }
    return result;
}

CustomFood NutritionService::create_custom_food(
    const std::string& user_id, const CreateCustomFoodRequest& req
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    std::string id = UUID::generate();

    if (req.brand) {
        txn.exec_params(
            "INSERT INTO custom_foods (id, user_id, name, brand, serving_size, serving_unit, "
            "  calories, protein_g, carbs_g, fat_g) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)",
            id, user_id, req.name, *req.brand, req.serving_size, req.serving_unit,
            req.calories, req.protein_g, req.carbs_g, req.fat_g
        );
    } else {
        txn.exec_params(
            "INSERT INTO custom_foods (id, user_id, name, serving_size, serving_unit, "
            "  calories, protein_g, carbs_g, fat_g) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)",
            id, user_id, req.name, req.serving_size, req.serving_unit,
            req.calories, req.protein_g, req.carbs_g, req.fat_g
        );
    }

    txn.commit();

    CustomFood cf;
    cf.id = id;
    cf.user_id = user_id;
    cf.name = req.name;
    cf.brand = req.brand;
    cf.serving_size = req.serving_size;
    cf.serving_unit = req.serving_unit;
    cf.calories = req.calories;
    cf.protein_g = req.protein_g;
    cf.carbs_g = req.carbs_g;
    cf.fat_g = req.fat_g;
    return cf;
}

std::vector<CustomFood> NutritionService::get_custom_foods(const std::string& user_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT * FROM custom_foods WHERE user_id = $1 ORDER BY name",
        user_id
    );

    txn.commit();

    std::vector<CustomFood> result;
    for (const auto& row : r) {
        CustomFood cf;
        cf.id = row["id"].as<std::string>();
        cf.user_id = row["user_id"].as<std::string>();
        cf.name = row["name"].as<std::string>();
        if (!row["brand"].is_null()) cf.brand = row["brand"].as<std::string>();
        cf.serving_size = row["serving_size"].as<double>();
        cf.serving_unit = row["serving_unit"].as<std::string>();
        cf.calories = row["calories"].as<double>();
        cf.protein_g = row["protein_g"].as<double>();
        cf.carbs_g = row["carbs_g"].as<double>();
        cf.fat_g = row["fat_g"].as<double>();
        result.push_back(std::move(cf));
    }
    return result;
}

bool NutritionService::delete_custom_food(const std::string& user_id, const std::string& food_id) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "DELETE FROM custom_foods WHERE id = $1 AND user_id = $2 RETURNING id",
        food_id, user_id
    );

    txn.commit();
    return !r.empty();
}

std::vector<RecentFood> NutritionService::get_recent_foods(
    const std::string& user_id, int limit
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    auto r = txn.exec_params(
        "SELECT food_name, brand, serving_size, serving_unit, "
        "       calories, protein_g, carbs_g, fat_g, "
        "       MAX(logged_at) AS last_logged "
        "FROM nutrition_log "
        "WHERE user_id = $1 "
        "GROUP BY food_name, brand, serving_size, serving_unit, "
        "         calories, protein_g, carbs_g, fat_g "
        "ORDER BY last_logged DESC LIMIT $2",
        user_id, limit
    );

    txn.commit();

    std::vector<RecentFood> result;
    for (const auto& row : r) {
        RecentFood rf;
        rf.food_name = row["food_name"].as<std::string>();
        if (!row["brand"].is_null()) rf.brand = row["brand"].as<std::string>();
        rf.serving_size = row["serving_size"].as<double>();
        rf.serving_unit = row["serving_unit"].as<std::string>();
        rf.calories = row["calories"].as<double>();
        rf.protein_g = row["protein_g"].as<double>();
        rf.carbs_g = row["carbs_g"].as<double>();
        rf.fat_g = row["fat_g"].as<double>();
        rf.last_logged = row["last_logged"].as<std::string>();
        result.push_back(std::move(rf));
    }
    return result;
}

std::vector<USDAFood> NutritionService::search_usda(
    const std::string& query, int limit
) {
    auto conn = db_.get_connection();
    pqxx::work txn(*conn);

    // Check cache first (7-day cache)
    auto cached = txn.exec_params(
        "SELECT fdc_id, description, brand_owner, serving_size, serving_unit, "
        "       calories, protein_g, carbs_g, fat_g "
        "FROM usda_food_cache "
        "WHERE LOWER(description) LIKE '%' || LOWER($1) || '%' "
        "  AND cached_at > NOW() - INTERVAL '7 days' "
        "LIMIT $2",
        query, limit
    );

    if (!cached.empty()) {
        std::vector<USDAFood> result;
        for (const auto& row : cached) {
            USDAFood f;
            f.fdc_id = row["fdc_id"].as<int>();
            f.description = row["description"].as<std::string>();
            if (!row["brand_owner"].is_null()) f.brand_owner = row["brand_owner"].as<std::string>();
            if (!row["serving_size"].is_null()) f.serving_size = row["serving_size"].as<double>();
            if (!row["serving_unit"].is_null()) f.serving_unit = row["serving_unit"].as<std::string>();
            f.calories = row["calories"].as<double>();
            f.protein_g = row["protein_g"].as<double>();
            f.carbs_g = row["carbs_g"].as<double>();
            f.fat_g = row["fat_g"].as<double>();
            result.push_back(std::move(f));
        }
        txn.commit();
        return result;
    }

    txn.commit();

    // Call USDA FoodData Central API
    std::string api_key = "DEMO_KEY";
    if (const char* env_key = std::getenv("USDA_API_KEY")) {
        api_key = env_key;
    }

    CURL* curl = curl_easy_init();
    if (!curl) return {};

    char* encoded_query = curl_easy_escape(curl, query.c_str(), static_cast<int>(query.length()));
    std::string url = "https://api.nal.usda.gov/fdc/v1/foods/search?api_key=" + api_key +
        "&query=" + std::string(encoded_query) + "&pageSize=" + std::to_string(limit);
    curl_free(encoded_query);

    std::string response_body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return {};

    std::vector<USDAFood> result;
    try {
        auto j = nlohmann::json::parse(response_body);
        if (!j.contains("foods")) return {};

        auto conn2 = db_.get_connection();
        pqxx::work txn2(*conn2);

        for (const auto& food : j["foods"]) {
            USDAFood f;
            f.fdc_id = food["fdcId"].get<int>();
            f.description = food.value("description", "Unknown");
            if (food.contains("brandOwner") && !food["brandOwner"].is_null())
                f.brand_owner = food["brandOwner"].get<std::string>();
            if (food.contains("servingSize") && !food["servingSize"].is_null())
                f.serving_size = food["servingSize"].get<double>();
            if (food.contains("servingSizeUnit") && !food["servingSizeUnit"].is_null())
                f.serving_unit = food["servingSizeUnit"].get<std::string>();

            // Extract nutrients
            f.calories = 0; f.protein_g = 0; f.carbs_g = 0; f.fat_g = 0;
            if (food.contains("foodNutrients")) {
                for (const auto& n : food["foodNutrients"]) {
                    int id = n.value("nutrientId", 0);
                    double val = n.value("value", 0.0);
                    if (id == 1008) f.calories = val;       // Energy (kcal)
                    else if (id == 1003) f.protein_g = val;  // Protein
                    else if (id == 1005) f.carbs_g = val;    // Carbs
                    else if (id == 1004) f.fat_g = val;      // Fat
                }
            }

            // Cache the result
            txn2.exec_params(
                "INSERT INTO usda_food_cache (fdc_id, description, brand_owner, serving_size, "
                "  serving_unit, calories, protein_g, carbs_g, fat_g) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) "
                "ON CONFLICT (fdc_id) DO UPDATE SET "
                "  description = EXCLUDED.description, cached_at = NOW()",
                f.fdc_id, f.description,
                f.brand_owner ? *f.brand_owner : std::string(""),
                f.serving_size.value_or(100.0),
                f.serving_unit.value_or("g"),
                f.calories, f.protein_g, f.carbs_g, f.fat_g
            );

            result.push_back(std::move(f));
        }

        txn2.commit();
    } catch (...) {
        // If parsing fails, return empty
    }

    return result;
}

} // namespace forge
