#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace forge {

// Identified food item from LLM
struct FoodItem {
    std::string name;
    float portion_grams;
    float calories;
    float protein_g;
    float carbs_g;
    float fat_g;
};

// Tier 2 LLM refinement result
struct LLMFoodAnalysis {
    std::vector<FoodItem> items;
    float total_calories;
    float total_protein_g;
    float total_carbs_g;
    float total_fat_g;
    std::string confidence_note;
    int tier = 2;
    int64_t inference_time_ms;
};

/**
 * LLM client for food recognition refinement
 * Tier 2: Vision model (GPT-4V or Claude) for complex meals
 */
class LLMClient {
public:
    enum class Provider {
        ANTHROPIC,  // Claude (Sonnet)
        OPENAI      // GPT-4V
    };

    struct Config {
        Provider provider = Provider::ANTHROPIC;
        std::string api_key;
        std::string model = "claude-sonnet-4-5-20250929";  // or "gpt-4-turbo"

        int timeout_seconds = 10;
        int max_retries = 1;

        // Rate limiting
        int max_requests_per_day = 100;
    };

    explicit LLMClient(const Config& config);
    ~LLMClient();

    // Analyze food photo with LLM
    std::optional<LLMFoodAnalysis> analyze_food(
        const std::string& base64_image,
        const std::string& tier1_prediction,
        float tier1_confidence
    );

    // Check rate limit for user
    bool check_rate_limit(const std::string& user_id);

    // Increment usage counter
    void increment_usage(const std::string& user_id);

private:
    Config config_;

    // Build prompt for LLM
    std::string build_prompt(const std::string& tier1_prediction, float tier1_confidence);

    // HTTP POST to API
    std::optional<std::string> post_request(const std::string& url, const nlohmann::json& body);

    // Parse LLM response
    std::optional<LLMFoodAnalysis> parse_response(const std::string& response_json);

    // Rate limit tracking (in-memory for now, could be Redis later)
    std::map<std::string, std::pair<int, int64_t>> usage_tracker_;  // user_id -> (count, timestamp)
};

} // namespace forge
