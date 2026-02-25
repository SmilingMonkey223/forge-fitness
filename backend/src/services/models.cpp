#include "../../include/models.hpp"
#include <iomanip>
#include <sstream>

namespace forge {

// Note: Workout::to_json() is implemented in workout_service.cpp

json NutritionLog::to_json() const {
    json j = {
        {"id", id},
        {"user_id", user_id},
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

    // Serialize logged_at as ISO 8601 string
    auto logged_time_t = std::chrono::system_clock::to_time_t(logged_at);
    std::tm tm_buf{};
    gmtime_r(&logged_time_t, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    j["logged_at"] = oss.str();

    return j;
}

} // namespace forge
