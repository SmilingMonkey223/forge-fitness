#pragma once

#include "models.hpp"
#include "database.hpp"
#include "uuid.hpp"
#include "config.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>

namespace forge {

class NutritionService {
public:
    // ── Nutrition Log CRUD ──────────────────────────────────────────

    struct LogFoodRequest {
        std::string food_name;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
        double serving_size;
        std::string serving_unit;
        double quantity = 1.0;
        std::optional<std::string> meal_type;
        std::optional<std::string> logged_at;
        std::optional<std::string> brand;
        std::optional<double> fiber_g;
        std::optional<double> sugar_g;
        std::optional<double> sodium_mg;
        bool is_custom = false;
        std::string source = "manual";
    };

    struct UpdateFoodRequest {
        std::optional<std::string> food_name;
        std::optional<double> calories;
        std::optional<double> protein_g;
        std::optional<double> carbs_g;
        std::optional<double> fat_g;
        std::optional<double> serving_size;
        std::optional<std::string> serving_unit;
        std::optional<double> quantity;
        std::optional<std::string> meal_type;
        std::optional<std::string> logged_at;
        std::optional<std::string> brand;
        std::optional<double> fiber_g;
        std::optional<double> sugar_g;
        std::optional<double> sodium_mg;
    };

    static NutritionLog log_food(const std::string& user_id, const LogFoodRequest& req);
    static std::vector<NutritionLog> get_logs_for_date(const std::string& user_id, const std::string& date);
    static NutritionLog update_log(const std::string& user_id, const std::string& log_id, const UpdateFoodRequest& req);
    static void delete_log(const std::string& user_id, const std::string& log_id);

    // ── Daily Summary ───────────────────────────────────────────────

    struct DailySummary {
        std::string date;
        double total_calories;
        double total_protein_g;
        double total_carbs_g;
        double total_fat_g;
        int entry_count;

        json to_json() const {
            return {
                {"date", date},
                {"total_calories", total_calories},
                {"total_protein_g", total_protein_g},
                {"total_carbs_g", total_carbs_g},
                {"total_fat_g", total_fat_g},
                {"entry_count", entry_count}
            };
        }
    };

    static DailySummary get_daily_summary(const std::string& user_id, const std::string& date);
    static std::vector<DailySummary> get_summary_range(const std::string& user_id,
                                                        const std::string& start_date,
                                                        const std::string& end_date);

    // ── Custom Foods ────────────────────────────────────────────────

    struct CreateCustomFoodRequest {
        std::string name;
        double serving_size;
        std::string serving_unit;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
        std::optional<std::string> brand;
        std::optional<double> fiber_g;
        std::optional<double> sugar_g;
        std::optional<double> sodium_mg;
    };

    struct CustomFood {
        std::string id;
        std::string user_id;
        std::string name;
        std::optional<std::string> brand;
        double serving_size;
        std::string serving_unit;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
        std::optional<double> fiber_g;
        std::optional<double> sugar_g;
        std::optional<double> sodium_mg;

        json to_json() const {
            json j = {
                {"id", id},
                {"user_id", user_id},
                {"name", name},
                {"serving_size", serving_size},
                {"serving_unit", serving_unit},
                {"calories", calories},
                {"protein_g", protein_g},
                {"carbs_g", carbs_g},
                {"fat_g", fat_g}
            };
            if (brand) j["brand"] = *brand;
            if (fiber_g) j["fiber_g"] = *fiber_g;
            if (sugar_g) j["sugar_g"] = *sugar_g;
            if (sodium_mg) j["sodium_mg"] = *sodium_mg;
            return j;
        }
    };

    static CustomFood create_custom_food(const std::string& user_id, const CreateCustomFoodRequest& req);
    static std::vector<CustomFood> get_custom_foods(const std::string& user_id);
    static void delete_custom_food(const std::string& user_id, const std::string& food_id);

    // ── Recent / Frequent Foods ─────────────────────────────────────

    struct RecentFood {
        std::string food_name;
        std::optional<std::string> brand;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
        double serving_size;
        std::string serving_unit;
        int log_count;

        json to_json() const {
            json j = {
                {"food_name", food_name},
                {"calories", calories},
                {"protein_g", protein_g},
                {"carbs_g", carbs_g},
                {"fat_g", fat_g},
                {"serving_size", serving_size},
                {"serving_unit", serving_unit},
                {"log_count", log_count}
            };
            if (brand) j["brand"] = *brand;
            return j;
        }
    };

    static std::vector<RecentFood> get_recent_foods(const std::string& user_id, int limit = 20);

    // ── USDA Food Search ────────────────────────────────────────────

    struct USDAFood {
        int fdc_id;
        std::string name;
        std::optional<std::string> brand;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
        std::optional<double> fiber_g;
        std::optional<double> sugar_g;
        std::optional<double> sodium_mg;

        json to_json() const {
            json j = {
                {"fdc_id", fdc_id},
                {"name", name},
                {"calories", calories},
                {"protein_g", protein_g},
                {"carbs_g", carbs_g},
                {"fat_g", fat_g}
            };
            if (brand) j["brand"] = *brand;
            if (fiber_g) j["fiber_g"] = *fiber_g;
            if (sugar_g) j["sugar_g"] = *sugar_g;
            if (sodium_mg) j["sodium_mg"] = *sodium_mg;
            return j;
        }
    };

    static std::vector<USDAFood> search_usda(const std::string& query, int limit = 10);

private:
    // Validate meal_type
    static bool is_valid_meal_type(const std::string& meal_type);

    // Validate date format YYYY-MM-DD
    static bool is_valid_date(const std::string& date);

    // Parse USDA API response
    static std::vector<USDAFood> parse_usda_response(const std::string& response_body);

    // Check cache for USDA search results
    static std::vector<USDAFood> check_usda_cache(const std::string& query);

    // Store USDA results in cache
    static void cache_usda_results(const std::vector<USDAFood>& foods);

    // Perform HTTP GET request using libcurl
    static std::string http_get(const std::string& url);

    // URL-encode a string
    static std::string url_encode(const std::string& value);
};

} // namespace forge
