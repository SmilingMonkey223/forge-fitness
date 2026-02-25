#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "database.hpp"
#include "models.hpp"

namespace forge {

struct DailySummary {
    std::string date;
    double total_calories;
    double total_protein_g;
    double total_carbs_g;
    double total_fat_g;
    int meal_count;

    nlohmann::json to_json() const {
        return {
            {"date", date},
            {"total_calories", total_calories},
            {"total_protein_g", total_protein_g},
            {"total_carbs_g", total_carbs_g},
            {"total_fat_g", total_fat_g},
            {"meal_count", meal_count}
        };
    }
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

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"id", id},
            {"name", name},
            {"serving_size", serving_size},
            {"serving_unit", serving_unit},
            {"calories", calories},
            {"protein_g", protein_g},
            {"carbs_g", carbs_g},
            {"fat_g", fat_g}
        };
        if (brand) j["brand"] = *brand;
        return j;
    }
};

struct RecentFood {
    std::string food_name;
    std::optional<std::string> brand;
    double serving_size;
    std::string serving_unit;
    double calories;
    double protein_g;
    double carbs_g;
    double fat_g;
    std::string last_logged;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"food_name", food_name},
            {"serving_size", serving_size},
            {"serving_unit", serving_unit},
            {"calories", calories},
            {"protein_g", protein_g},
            {"carbs_g", carbs_g},
            {"fat_g", fat_g},
            {"last_logged", last_logged}
        };
        if (brand) j["brand"] = *brand;
        return j;
    }
};

struct USDAFood {
    int fdc_id;
    std::string description;
    std::optional<std::string> brand_owner;
    std::optional<double> serving_size;
    std::optional<std::string> serving_unit;
    double calories;
    double protein_g;
    double carbs_g;
    double fat_g;

    nlohmann::json to_json() const {
        nlohmann::json j = {
            {"fdc_id", fdc_id},
            {"description", description},
            {"calories", calories},
            {"protein_g", protein_g},
            {"carbs_g", carbs_g},
            {"fat_g", fat_g}
        };
        if (brand_owner) j["brand_owner"] = *brand_owner;
        if (serving_size) j["serving_size"] = *serving_size;
        if (serving_unit) j["serving_unit"] = *serving_unit;
        return j;
    }
};

class NutritionService {
public:
    explicit NutritionService(Database& db) : db_(db) {}

    struct LogFoodRequest {
        std::optional<std::string> meal_type;
        std::string food_name;
        std::optional<std::string> brand;
        double serving_size = 100.0;
        std::string serving_unit = "g";
        double quantity = 1.0;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
        std::optional<double> fiber_g;
        std::optional<double> sugar_g;
        std::optional<double> sodium_mg;
        bool is_custom = false;
        std::string source = "manual";
    };

    struct UpdateFoodRequest {
        std::optional<std::string> meal_type;
        std::optional<std::string> food_name;
        std::optional<double> serving_size;
        std::optional<std::string> serving_unit;
        std::optional<double> quantity;
        std::optional<double> calories;
        std::optional<double> protein_g;
        std::optional<double> carbs_g;
        std::optional<double> fat_g;
    };

    struct CreateCustomFoodRequest {
        std::string name;
        std::optional<std::string> brand;
        double serving_size;
        std::string serving_unit;
        double calories;
        double protein_g;
        double carbs_g;
        double fat_g;
    };

    // CRUD
    NutritionLog log_food(const std::string& user_id, const LogFoodRequest& req);
    std::vector<NutritionLog> get_logs_for_date(const std::string& user_id, const std::string& date);
    NutritionLog update_log(const std::string& user_id, const std::string& log_id, const UpdateFoodRequest& req);
    bool delete_log(const std::string& user_id, const std::string& log_id);

    // Summaries
    DailySummary get_daily_summary(const std::string& user_id, const std::string& date);
    std::vector<DailySummary> get_summary_range(const std::string& user_id, const std::string& start, const std::string& end);

    // Custom foods
    CustomFood create_custom_food(const std::string& user_id, const CreateCustomFoodRequest& req);
    std::vector<CustomFood> get_custom_foods(const std::string& user_id);
    bool delete_custom_food(const std::string& user_id, const std::string& food_id);

    // Recent foods and search
    std::vector<RecentFood> get_recent_foods(const std::string& user_id, int limit = 20);
    std::vector<USDAFood> search_usda(const std::string& query, int limit = 25);

private:
    Database& db_;
};

} // namespace forge
