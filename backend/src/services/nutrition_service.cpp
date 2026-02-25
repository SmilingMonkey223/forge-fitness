#include "../../include/nutrition_service.hpp"
#include <curl/curl.h>
#include <regex>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>

namespace forge {

// ── Helpers ─────────────────────────────────────────────────────────

bool NutritionService::is_valid_meal_type(const std::string& meal_type) {
    return meal_type == "breakfast" || meal_type == "lunch" ||
           meal_type == "dinner"   || meal_type == "snack";
}

bool NutritionService::is_valid_date(const std::string& date) {
    static const std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    return std::regex_match(date, pattern);
}

std::string NutritionService::url_encode(const std::string& value) {
    CURL* curl = curl_easy_init();
    if (!curl) return value;

    char* encoded = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

// libcurl write callback
static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string NutritionService::http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize libcurl");
    }

    std::string response_body;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP request failed: ") + curl_easy_strerror(res));
    }

    if (http_code != 200) {
        throw std::runtime_error("USDA API returned HTTP " + std::to_string(http_code));
    }

    return response_body;
}

// Helper: read optional double from pqxx field
static std::optional<double> opt_double(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return f.as<double>();
}

// Helper: read optional string from pqxx field
static std::optional<std::string> opt_string(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return std::string(f.c_str());
}

// Helper: build NutritionLog from a result row
static NutritionLog row_to_nutrition_log(const pqxx::row& row) {
    NutritionLog log;
    log.id = row["id"].c_str();
    log.user_id = row["user_id"].c_str();
    log.food_name = row["food_name"].c_str();
    log.meal_type = opt_string(row["meal_type"]);
    log.brand = opt_string(row["brand"]);
    log.serving_size = row["serving_size"].as<double>();
    log.serving_unit = row["serving_unit"].c_str();
    log.quantity = row["quantity"].as<double>();
    log.calories = row["calories"].as<double>();
    log.protein_g = row["protein_g"].as<double>();
    log.carbs_g = row["carbs_g"].as<double>();
    log.fat_g = row["fat_g"].as<double>();
    log.fiber_g = opt_double(row["fiber_g"]);
    log.sugar_g = opt_double(row["sugar_g"]);
    log.sodium_mg = opt_double(row["sodium_mg"]);
    log.is_custom = row["is_custom"].as<bool>();
    log.source = row["source"].c_str();

    // Parse logged_at timestamp
    std::string logged_at_str = row["logged_at"].c_str();
    std::tm tm = {};
    std::istringstream ss(logged_at_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    log.logged_at = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    return log;
}

// ── Nutrition Log CRUD ──────────────────────────────────────────────

NutritionLog NutritionService::log_food(const std::string& user_id, const LogFoodRequest& req) {
    // Validate meal_type if provided
    if (req.meal_type && !is_valid_meal_type(*req.meal_type)) {
        throw std::invalid_argument("INVALID_MEAL_TYPE");
    }

    if (req.food_name.empty()) {
        throw std::invalid_argument("FOOD_NAME_REQUIRED");
    }

    if (req.serving_size <= 0) {
        throw std::invalid_argument("INVALID_SERVING_SIZE");
    }

    if (req.calories < 0 || req.protein_g < 0 || req.carbs_g < 0 || req.fat_g < 0) {
        throw std::invalid_argument("INVALID_NUTRITION_VALUES");
    }

    std::string entry_id = UUID::generate();

    // Use provided logged_at or default to now
    std::string logged_at_value;
    if (req.logged_at && !req.logged_at->empty()) {
        logged_at_value = *req.logged_at;
    } else {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::gmtime(&time_t_now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        logged_at_value = oss.str();
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    // Build the query with optional fields
    std::string meal_type_param;
    if (req.meal_type) {
        meal_type_param = *req.meal_type;
    }

    auto result = txn.exec_params(
        "INSERT INTO nutrition_log "
        "(id, user_id, logged_at, meal_type, food_name, brand, "
        " serving_size, serving_unit, quantity, calories, protein_g, carbs_g, fat_g, "
        " fiber_g, sugar_g, sodium_mg, is_custom, source) "
        "VALUES ($1, $2, $3::timestamp, "
        "  CASE WHEN $4 = '' THEN NULL ELSE $4::varchar END, "
        "  $5, "
        "  CASE WHEN $6 = '' THEN NULL ELSE $6::varchar END, "
        "  $7, $8, $9, $10, $11, $12, $13, "
        "  CASE WHEN $14::text IS NULL THEN NULL ELSE $14::decimal END, "
        "  CASE WHEN $15::text IS NULL THEN NULL ELSE $15::decimal END, "
        "  CASE WHEN $16::text IS NULL THEN NULL ELSE $16::decimal END, "
        "  $17, $18) "
        "RETURNING id, user_id, logged_at, meal_type, food_name, brand, "
        "  serving_size, serving_unit, quantity, calories, protein_g, carbs_g, fat_g, "
        "  fiber_g, sugar_g, sodium_mg, is_custom, source",
        entry_id,
        user_id,
        logged_at_value,
        req.meal_type ? *req.meal_type : std::string(""),
        req.food_name,
        req.brand ? *req.brand : std::string(""),
        req.serving_size,
        req.serving_unit,
        req.quantity,
        req.calories,
        req.protein_g,
        req.carbs_g,
        req.fat_g,
        req.fiber_g ? std::optional<std::string>(std::to_string(*req.fiber_g)) : std::nullopt,
        req.sugar_g ? std::optional<std::string>(std::to_string(*req.sugar_g)) : std::nullopt,
        req.sodium_mg ? std::optional<std::string>(std::to_string(*req.sodium_mg)) : std::nullopt,
        req.is_custom,
        req.source
    );

    txn.commit();

    return row_to_nutrition_log(result[0]);
}

std::vector<NutritionLog> NutritionService::get_logs_for_date(const std::string& user_id, const std::string& date) {
    if (!is_valid_date(date)) {
        throw std::invalid_argument("INVALID_DATE_FORMAT");
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT id, user_id, logged_at, meal_type, food_name, brand, "
        "  serving_size, serving_unit, quantity, calories, protein_g, carbs_g, fat_g, "
        "  fiber_g, sugar_g, sodium_mg, is_custom, source "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date AND deleted_at IS NULL "
        "ORDER BY logged_at ASC",
        user_id, date
    );

    txn.commit();

    std::vector<NutritionLog> logs;
    logs.reserve(result.size());
    for (const auto& row : result) {
        logs.push_back(row_to_nutrition_log(row));
    }
    return logs;
}

NutritionLog NutritionService::update_log(const std::string& user_id, const std::string& log_id, const UpdateFoodRequest& req) {
    if (!UUID::is_valid(log_id)) {
        throw std::invalid_argument("INVALID_LOG_ID");
    }

    if (req.meal_type && !req.meal_type->empty() && !is_valid_meal_type(*req.meal_type)) {
        throw std::invalid_argument("INVALID_MEAL_TYPE");
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    // Verify ownership
    auto check = txn.exec_params(
        "SELECT id FROM nutrition_log WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        log_id, user_id
    );

    if (check.empty()) {
        throw std::invalid_argument("LOG_NOT_FOUND");
    }

    // Build dynamic UPDATE query
    std::vector<std::string> set_clauses;
    int param_idx = 3; // $1 = log_id, $2 = user_id

    // We'll build the SQL dynamically but still use parameterized queries
    // by constructing the full parameter list
    std::string sql = "UPDATE nutrition_log SET ";

    // For simplicity and safety, update all provided fields
    // We construct the query with conditional updates using COALESCE pattern
    auto result = txn.exec_params(
        "UPDATE nutrition_log SET "
        "  food_name = COALESCE($3, food_name), "
        "  calories = COALESCE($4, calories), "
        "  protein_g = COALESCE($5, protein_g), "
        "  carbs_g = COALESCE($6, carbs_g), "
        "  fat_g = COALESCE($7, fat_g), "
        "  serving_size = COALESCE($8, serving_size), "
        "  serving_unit = COALESCE($9, serving_unit), "
        "  quantity = COALESCE($10, quantity), "
        "  meal_type = CASE WHEN $11::text IS NOT NULL THEN $11::varchar ELSE meal_type END, "
        "  logged_at = CASE WHEN $12::text IS NOT NULL THEN $12::timestamp ELSE logged_at END, "
        "  brand = CASE WHEN $13::text IS NOT NULL THEN $13::varchar ELSE brand END, "
        "  fiber_g = CASE WHEN $14::text IS NOT NULL THEN $14::decimal ELSE fiber_g END, "
        "  sugar_g = CASE WHEN $15::text IS NOT NULL THEN $15::decimal ELSE sugar_g END, "
        "  sodium_mg = CASE WHEN $16::text IS NOT NULL THEN $16::decimal ELSE sodium_mg END, "
        "  updated_at = CURRENT_TIMESTAMP "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL "
        "RETURNING id, user_id, logged_at, meal_type, food_name, brand, "
        "  serving_size, serving_unit, quantity, calories, protein_g, carbs_g, fat_g, "
        "  fiber_g, sugar_g, sodium_mg, is_custom, source",
        log_id,
        user_id,
        req.food_name ? std::optional<std::string>(*req.food_name) : std::nullopt,
        req.calories ? std::optional<double>(*req.calories) : std::nullopt,
        req.protein_g ? std::optional<double>(*req.protein_g) : std::nullopt,
        req.carbs_g ? std::optional<double>(*req.carbs_g) : std::nullopt,
        req.fat_g ? std::optional<double>(*req.fat_g) : std::nullopt,
        req.serving_size ? std::optional<double>(*req.serving_size) : std::nullopt,
        req.serving_unit ? std::optional<std::string>(*req.serving_unit) : std::nullopt,
        req.quantity ? std::optional<double>(*req.quantity) : std::nullopt,
        req.meal_type ? std::optional<std::string>(*req.meal_type) : std::nullopt,
        req.logged_at ? std::optional<std::string>(*req.logged_at) : std::nullopt,
        req.brand ? std::optional<std::string>(*req.brand) : std::nullopt,
        req.fiber_g ? std::optional<std::string>(std::to_string(*req.fiber_g)) : std::nullopt,
        req.sugar_g ? std::optional<std::string>(std::to_string(*req.sugar_g)) : std::nullopt,
        req.sodium_mg ? std::optional<std::string>(std::to_string(*req.sodium_mg)) : std::nullopt
    );

    txn.commit();

    if (result.empty()) {
        throw std::invalid_argument("LOG_NOT_FOUND");
    }

    return row_to_nutrition_log(result[0]);
}

void NutritionService::delete_log(const std::string& user_id, const std::string& log_id) {
    if (!UUID::is_valid(log_id)) {
        throw std::invalid_argument("INVALID_LOG_ID");
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "UPDATE nutrition_log SET deleted_at = CURRENT_TIMESTAMP "
        "WHERE id = $1 AND user_id = $2 AND deleted_at IS NULL",
        log_id, user_id
    );

    txn.commit();

    if (result.affected_rows() == 0) {
        throw std::invalid_argument("LOG_NOT_FOUND");
    }
}

// ── Daily Summary ───────────────────────────────────────────────────

NutritionService::DailySummary NutritionService::get_daily_summary(const std::string& user_id, const std::string& date) {
    if (!is_valid_date(date)) {
        throw std::invalid_argument("INVALID_DATE_FORMAT");
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT "
        "  COALESCE(SUM(calories * quantity), 0) as total_calories, "
        "  COALESCE(SUM(protein_g * quantity), 0) as total_protein_g, "
        "  COALESCE(SUM(carbs_g * quantity), 0) as total_carbs_g, "
        "  COALESCE(SUM(fat_g * quantity), 0) as total_fat_g, "
        "  COUNT(*) as entry_count "
        "FROM nutrition_log "
        "WHERE user_id = $1 AND DATE(logged_at) = $2::date AND deleted_at IS NULL",
        user_id, date
    );

    txn.commit();

    DailySummary summary;
    summary.date = date;
    summary.total_calories = result[0]["total_calories"].as<double>();
    summary.total_protein_g = result[0]["total_protein_g"].as<double>();
    summary.total_carbs_g = result[0]["total_carbs_g"].as<double>();
    summary.total_fat_g = result[0]["total_fat_g"].as<double>();
    summary.entry_count = result[0]["entry_count"].as<int>();

    return summary;
}

std::vector<NutritionService::DailySummary> NutritionService::get_summary_range(
    const std::string& user_id,
    const std::string& start_date,
    const std::string& end_date)
{
    if (!is_valid_date(start_date) || !is_valid_date(end_date)) {
        throw std::invalid_argument("INVALID_DATE_FORMAT");
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT "
        "  DATE(logged_at) as log_date, "
        "  COALESCE(SUM(calories * quantity), 0) as total_calories, "
        "  COALESCE(SUM(protein_g * quantity), 0) as total_protein_g, "
        "  COALESCE(SUM(carbs_g * quantity), 0) as total_carbs_g, "
        "  COALESCE(SUM(fat_g * quantity), 0) as total_fat_g, "
        "  COUNT(*) as entry_count "
        "FROM nutrition_log "
        "WHERE user_id = $1 "
        "  AND DATE(logged_at) >= $2::date "
        "  AND DATE(logged_at) <= $3::date "
        "  AND deleted_at IS NULL "
        "GROUP BY DATE(logged_at) "
        "ORDER BY log_date ASC",
        user_id, start_date, end_date
    );

    txn.commit();

    std::vector<DailySummary> summaries;
    summaries.reserve(result.size());
    for (const auto& row : result) {
        DailySummary s;
        s.date = row["log_date"].c_str();
        s.total_calories = row["total_calories"].as<double>();
        s.total_protein_g = row["total_protein_g"].as<double>();
        s.total_carbs_g = row["total_carbs_g"].as<double>();
        s.total_fat_g = row["total_fat_g"].as<double>();
        s.entry_count = row["entry_count"].as<int>();
        summaries.push_back(s);
    }
    return summaries;
}

// ── Custom Foods ────────────────────────────────────────────────────

NutritionService::CustomFood NutritionService::create_custom_food(
    const std::string& user_id,
    const CreateCustomFoodRequest& req)
{
    if (req.name.empty()) {
        throw std::invalid_argument("FOOD_NAME_REQUIRED");
    }

    if (req.serving_size <= 0) {
        throw std::invalid_argument("INVALID_SERVING_SIZE");
    }

    if (req.calories < 0 || req.protein_g < 0 || req.carbs_g < 0 || req.fat_g < 0) {
        throw std::invalid_argument("INVALID_NUTRITION_VALUES");
    }

    std::string food_id = UUID::generate();

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    txn.exec_params(
        "INSERT INTO custom_foods "
        "(id, user_id, name, brand, serving_size, serving_unit, "
        " calories, protein_g, carbs_g, fat_g, fiber_g, sugar_g, sodium_mg) "
        "VALUES ($1, $2, $3, "
        "  CASE WHEN $4 = '' THEN NULL ELSE $4::varchar END, "
        "  $5, $6, $7, $8, $9, $10, "
        "  CASE WHEN $11::text IS NULL THEN NULL ELSE $11::decimal END, "
        "  CASE WHEN $12::text IS NULL THEN NULL ELSE $12::decimal END, "
        "  CASE WHEN $13::text IS NULL THEN NULL ELSE $13::decimal END)",
        food_id,
        user_id,
        req.name,
        req.brand ? *req.brand : std::string(""),
        req.serving_size,
        req.serving_unit,
        req.calories,
        req.protein_g,
        req.carbs_g,
        req.fat_g,
        req.fiber_g ? std::optional<std::string>(std::to_string(*req.fiber_g)) : std::nullopt,
        req.sugar_g ? std::optional<std::string>(std::to_string(*req.sugar_g)) : std::nullopt,
        req.sodium_mg ? std::optional<std::string>(std::to_string(*req.sodium_mg)) : std::nullopt
    );

    txn.commit();

    CustomFood food;
    food.id = food_id;
    food.user_id = user_id;
    food.name = req.name;
    food.brand = req.brand;
    food.serving_size = req.serving_size;
    food.serving_unit = req.serving_unit;
    food.calories = req.calories;
    food.protein_g = req.protein_g;
    food.carbs_g = req.carbs_g;
    food.fat_g = req.fat_g;
    food.fiber_g = req.fiber_g;
    food.sugar_g = req.sugar_g;
    food.sodium_mg = req.sodium_mg;

    return food;
}

std::vector<NutritionService::CustomFood> NutritionService::get_custom_foods(const std::string& user_id) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "SELECT id, user_id, name, brand, serving_size, serving_unit, "
        "  calories, protein_g, carbs_g, fat_g, fiber_g, sugar_g, sodium_mg "
        "FROM custom_foods "
        "WHERE user_id = $1 "
        "ORDER BY name ASC",
        user_id
    );

    txn.commit();

    std::vector<CustomFood> foods;
    foods.reserve(result.size());
    for (const auto& row : result) {
        CustomFood f;
        f.id = row["id"].c_str();
        f.user_id = row["user_id"].c_str();
        f.name = row["name"].c_str();
        f.brand = opt_string(row["brand"]);
        f.serving_size = row["serving_size"].as<double>();
        f.serving_unit = row["serving_unit"].c_str();
        f.calories = row["calories"].as<double>();
        f.protein_g = row["protein_g"].as<double>();
        f.carbs_g = row["carbs_g"].as<double>();
        f.fat_g = row["fat_g"].as<double>();
        f.fiber_g = opt_double(row["fiber_g"]);
        f.sugar_g = opt_double(row["sugar_g"]);
        f.sodium_mg = opt_double(row["sodium_mg"]);
        foods.push_back(f);
    }
    return foods;
}

void NutritionService::delete_custom_food(const std::string& user_id, const std::string& food_id) {
    if (!UUID::is_valid(food_id)) {
        throw std::invalid_argument("INVALID_FOOD_ID");
    }

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    auto result = txn.exec_params(
        "DELETE FROM custom_foods WHERE id = $1 AND user_id = $2",
        food_id, user_id
    );

    txn.commit();

    if (result.affected_rows() == 0) {
        throw std::invalid_argument("FOOD_NOT_FOUND");
    }
}

// ── Recent / Frequent Foods ─────────────────────────────────────────

std::vector<NutritionService::RecentFood> NutritionService::get_recent_foods(const std::string& user_id, int limit) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    // Get the most frequently logged unique foods, using the most recent entry for nutrition data
    auto result = txn.exec_params(
        "WITH food_stats AS ( "
        "  SELECT "
        "    food_name, "
        "    brand, "
        "    COUNT(*) as log_count, "
        "    MAX(logged_at) as last_logged "
        "  FROM nutrition_log "
        "  WHERE user_id = $1 AND deleted_at IS NULL "
        "  GROUP BY food_name, brand "
        "  ORDER BY log_count DESC, last_logged DESC "
        "  LIMIT $2 "
        ") "
        "SELECT DISTINCT ON (fs.food_name, fs.brand) "
        "  fs.food_name, fs.brand, fs.log_count, "
        "  nl.calories, nl.protein_g, nl.carbs_g, nl.fat_g, "
        "  nl.serving_size, nl.serving_unit "
        "FROM food_stats fs "
        "JOIN nutrition_log nl ON nl.food_name = fs.food_name "
        "  AND (nl.brand = fs.brand OR (nl.brand IS NULL AND fs.brand IS NULL)) "
        "  AND nl.user_id = $1 AND nl.deleted_at IS NULL "
        "ORDER BY fs.food_name, fs.brand, nl.logged_at DESC",
        user_id, limit
    );

    txn.commit();

    std::vector<RecentFood> foods;
    foods.reserve(result.size());
    for (const auto& row : result) {
        RecentFood f;
        f.food_name = row["food_name"].c_str();
        f.brand = opt_string(row["brand"]);
        f.calories = row["calories"].as<double>();
        f.protein_g = row["protein_g"].as<double>();
        f.carbs_g = row["carbs_g"].as<double>();
        f.fat_g = row["fat_g"].as<double>();
        f.serving_size = row["serving_size"].as<double>();
        f.serving_unit = row["serving_unit"].c_str();
        f.log_count = row["log_count"].as<int>();
        foods.push_back(f);
    }

    // Re-sort by frequency (DISTINCT ON may change ordering)
    std::sort(foods.begin(), foods.end(), [](const RecentFood& a, const RecentFood& b) {
        return a.log_count > b.log_count;
    });

    return foods;
}

// ── USDA Food Search ────────────────────────────────────────────────

std::vector<NutritionService::USDAFood> NutritionService::check_usda_cache(const std::string& query) {
    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    // Search cache using full-text search, only return non-expired entries
    auto result = txn.exec_params(
        "SELECT fdc_id, name, brand, calories, protein_g, carbs_g, fat_g, "
        "  fiber_g, sugar_g, sodium_mg "
        "FROM usda_food_cache "
        "WHERE to_tsvector('english', name) @@ plainto_tsquery('english', $1) "
        "  AND expires_at > CURRENT_TIMESTAMP "
        "ORDER BY ts_rank(to_tsvector('english', name), plainto_tsquery('english', $1)) DESC "
        "LIMIT 10",
        query
    );

    txn.commit();

    std::vector<USDAFood> foods;
    foods.reserve(result.size());
    for (const auto& row : result) {
        USDAFood f;
        f.fdc_id = row["fdc_id"].as<int>();
        f.name = row["name"].c_str();
        f.brand = opt_string(row["brand"]);
        f.calories = row["calories"].is_null() ? 0.0 : row["calories"].as<double>();
        f.protein_g = row["protein_g"].is_null() ? 0.0 : row["protein_g"].as<double>();
        f.carbs_g = row["carbs_g"].is_null() ? 0.0 : row["carbs_g"].as<double>();
        f.fat_g = row["fat_g"].is_null() ? 0.0 : row["fat_g"].as<double>();
        f.fiber_g = opt_double(row["fiber_g"]);
        f.sugar_g = opt_double(row["sugar_g"]);
        f.sodium_mg = opt_double(row["sodium_mg"]);
        foods.push_back(f);
    }
    return foods;
}

void NutritionService::cache_usda_results(const std::vector<USDAFood>& foods) {
    if (foods.empty()) return;

    auto conn = Database::instance().get_connection();
    pqxx::work txn(*conn);

    for (const auto& food : foods) {
        // Upsert: insert or update on conflict
        txn.exec_params(
            "INSERT INTO usda_food_cache "
            "(fdc_id, name, brand, calories, protein_g, carbs_g, fat_g, "
            " fiber_g, sugar_g, sodium_mg, expires_at) "
            "VALUES ($1, $2, "
            "  CASE WHEN $3 = '' THEN NULL ELSE $3::varchar END, "
            "  $4, $5, $6, $7, "
            "  CASE WHEN $8::text IS NULL THEN NULL ELSE $8::decimal END, "
            "  CASE WHEN $9::text IS NULL THEN NULL ELSE $9::decimal END, "
            "  CASE WHEN $10::text IS NULL THEN NULL ELSE $10::decimal END, "
            "  CURRENT_TIMESTAMP + INTERVAL '30 days') "
            "ON CONFLICT (fdc_id) DO UPDATE SET "
            "  name = EXCLUDED.name, "
            "  brand = EXCLUDED.brand, "
            "  calories = EXCLUDED.calories, "
            "  protein_g = EXCLUDED.protein_g, "
            "  carbs_g = EXCLUDED.carbs_g, "
            "  fat_g = EXCLUDED.fat_g, "
            "  fiber_g = EXCLUDED.fiber_g, "
            "  sugar_g = EXCLUDED.sugar_g, "
            "  sodium_mg = EXCLUDED.sodium_mg, "
            "  expires_at = CURRENT_TIMESTAMP + INTERVAL '30 days', "
            "  cached_at = CURRENT_TIMESTAMP",
            food.fdc_id,
            food.name,
            food.brand ? *food.brand : std::string(""),
            food.calories,
            food.protein_g,
            food.carbs_g,
            food.fat_g,
            food.fiber_g ? std::optional<std::string>(std::to_string(*food.fiber_g)) : std::nullopt,
            food.sugar_g ? std::optional<std::string>(std::to_string(*food.sugar_g)) : std::nullopt,
            food.sodium_mg ? std::optional<std::string>(std::to_string(*food.sodium_mg)) : std::nullopt
        );
    }

    txn.commit();
}

std::vector<NutritionService::USDAFood> NutritionService::parse_usda_response(const std::string& response_body) {
    std::vector<USDAFood> foods;

    try {
        auto data = json::parse(response_body);

        if (!data.contains("foods") || !data["foods"].is_array()) {
            return foods;
        }

        for (const auto& item : data["foods"]) {
            USDAFood food;
            food.fdc_id = item.value("fdcId", 0);
            food.name = item.value("description", "Unknown");

            if (item.contains("brandName") && !item["brandName"].is_null()) {
                food.brand = item["brandName"].get<std::string>();
            } else if (item.contains("brandOwner") && !item["brandOwner"].is_null()) {
                food.brand = item["brandOwner"].get<std::string>();
            }

            // Extract nutrients - values are per 100g
            food.calories = 0;
            food.protein_g = 0;
            food.carbs_g = 0;
            food.fat_g = 0;

            if (item.contains("foodNutrients") && item["foodNutrients"].is_array()) {
                for (const auto& nutrient : item["foodNutrients"]) {
                    int nutrient_id = nutrient.value("nutrientId", 0);
                    double value = nutrient.value("value", 0.0);

                    switch (nutrient_id) {
                        case 1008: // Energy (kcal)
                            food.calories = value;
                            break;
                        case 1003: // Protein
                            food.protein_g = value;
                            break;
                        case 1005: // Carbohydrate, by difference
                            food.carbs_g = value;
                            break;
                        case 1004: // Total lipid (fat)
                            food.fat_g = value;
                            break;
                        case 1079: // Fiber, total dietary
                            food.fiber_g = value;
                            break;
                        case 2000: // Sugars, total
                            food.sugar_g = value;
                            break;
                        case 1093: // Sodium, Na
                            food.sodium_mg = value;
                            break;
                    }
                }
            }

            foods.push_back(food);

            if (static_cast<int>(foods.size()) >= 10) break;
        }
    } catch (const json::exception& e) {
        std::cerr << "Failed to parse USDA response: " << e.what() << std::endl;
    }

    return foods;
}

std::vector<NutritionService::USDAFood> NutritionService::search_usda(const std::string& query, int limit) {
    if (query.empty()) {
        throw std::invalid_argument("EMPTY_SEARCH_QUERY");
    }

    // First check cache
    auto cached = check_usda_cache(query);
    if (!cached.empty()) {
        if (static_cast<int>(cached.size()) > limit) {
            cached.resize(limit);
        }
        return cached;
    }

    // Not in cache - call USDA API
    auto& config = Config::instance();
    std::string api_key = config.usda_api_key();

    if (api_key.empty()) {
        throw std::runtime_error("USDA_API_KEY not configured");
    }

    std::string encoded_query = url_encode(query);
    std::string url = "https://api.nal.usda.gov/fdc/v1/foods/search"
                      "?api_key=" + api_key +
                      "&query=" + encoded_query +
                      "&pageSize=" + std::to_string(limit) +
                      "&dataType=Foundation,SR%20Legacy,Branded";

    std::string response_body = http_get(url);
    auto foods = parse_usda_response(response_body);

    // Cache results for next time
    if (!foods.empty()) {
        try {
            cache_usda_results(foods);
        } catch (const std::exception& e) {
            // Caching failure is not critical; log and continue
            std::cerr << "Warning: failed to cache USDA results: " << e.what() << std::endl;
        }
    }

    return foods;
}

} // namespace forge
