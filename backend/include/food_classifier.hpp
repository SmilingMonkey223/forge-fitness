#pragma once

#include "image_processor.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace forge {

// Single classification result
struct FoodPrediction {
    std::string food_class;
    float confidence;
};

// Tier 1 classification result
struct ClassificationResult {
    std::vector<FoodPrediction> predictions;  // Top-5 predictions
    int tier;                                  // Always 1 for ONNX
    int64_t inference_time_ms;
};

/**
 * Food classification using ONNX Runtime
 * Tier 1: On-server ML inference with MobileNetV3 or EfficientNet
 */
class FoodClassifier {
public:
    struct Config {
        std::string model_path;
        int num_threads = 4;
        bool enable_optimizations = true;

        // Model-specific config
        int input_size = 224;  // 224 for MobileNetV3, 300 for EfficientNet
        std::vector<std::string> class_labels;  // Food-101 or Food-2K labels

        // Performance targets
        int64_t max_inference_time_ms = 100;  // p95 target
    };

    explicit FoodClassifier(const Config& config);
    ~FoodClassifier();

    // Main inference method
    std::optional<ClassificationResult> classify(const ProcessedImage& image);

    // Load model (called in constructor, but exposed for reload)
    bool load_model(const std::string& model_path);

    // Check if model is loaded
    bool is_loaded() const;

private:
    class Impl;  // PIMPL pattern to hide ONNX Runtime headers
    std::unique_ptr<Impl> impl_;
    Config config_;
};

} // namespace forge
