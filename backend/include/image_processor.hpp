#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace forge {

// Image preprocessing result
struct ProcessedImage {
    std::vector<float> tensor_data;  // Preprocessed tensor in NCHW format
    int channels;
    int height;
    int width;
    int batch_size;
};

// Image preprocessing statistics
struct PreprocessingStats {
    int64_t decode_time_ms;
    int64_t resize_time_ms;
    int64_t normalize_time_ms;
    int64_t total_time_ms;
    size_t original_size_bytes;
    size_t processed_size_bytes;
};

/**
 * Image preprocessing pipeline for food recognition
 * Handles: decode, EXIF rotation, resize, color conversion, normalization, tensor layout
 */
class ImageProcessor {
public:
    struct Config {
        int target_width = 224;
        int target_height = 224;

        // ImageNet normalization defaults
        std::vector<float> mean = {0.485f, 0.456f, 0.406f};
        std::vector<float> std = {0.229f, 0.224f, 0.225f};

        // Performance constraints
        size_t max_file_size = 10 * 1024 * 1024;  // 10MB
        int64_t max_processing_time_ms = 50;       // p95 target
    };

    explicit ImageProcessor(const Config& config = Config{});
    ~ImageProcessor();

    // Main preprocessing pipeline
    std::optional<ProcessedImage> process(
        const std::vector<uint8_t>& image_data,
        PreprocessingStats* stats = nullptr
    );

    // Individual steps (exposed for testing)
    std::optional<std::vector<uint8_t>> decode(const std::vector<uint8_t>& encoded_data);
    std::optional<std::vector<uint8_t>> apply_exif_rotation(const std::vector<uint8_t>& rgba_data, int width, int height);
    std::optional<std::vector<uint8_t>> resize(const std::vector<uint8_t>& rgba_data, int src_w, int src_h, int dst_w, int dst_h);
    std::vector<float> normalize_and_convert_to_tensor(const std::vector<uint8_t>& rgba_data, int width, int height);

private:
    Config config_;

    // Bilinear interpolation for resizing
    uint8_t sample_pixel(const std::vector<uint8_t>& data, int w, int h, int channels, float x, float y, int channel);
};

} // namespace forge
