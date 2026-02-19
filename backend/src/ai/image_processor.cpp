#include "image_processor.hpp"
#include <cmath>
#include <chrono>
#include <algorithm>

// Use stb_image for image decoding
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "stb_image.h"

namespace forge {

ImageProcessor::ImageProcessor(const Config& config) : config_(config) {}

ImageProcessor::~ImageProcessor() = default;

std::optional<ProcessedImage> ImageProcessor::process(
    const std::vector<uint8_t>& image_data,
    PreprocessingStats* stats
) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // Check file size
    if (image_data.size() > config_.max_file_size) {
        return std::nullopt;
    }

    PreprocessingStats local_stats{};
    local_stats.original_size_bytes = image_data.size();

    // Step 1: Decode image
    auto decode_start = std::chrono::high_resolution_clock::now();
    auto decoded = decode(image_data);
    if (!decoded) {
        return std::nullopt;
    }
    auto decode_end = std::chrono::high_resolution_clock::now();
    local_stats.decode_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        decode_end - decode_start
    ).count();

    // Step 2: Resize to target dimensions
    auto resize_start = std::chrono::high_resolution_clock::now();
    int width, height, channels;
    stbi_info_from_memory(image_data.data(), image_data.size(), &width, &height, &channels);

    auto resized = resize(*decoded, width, height, config_.target_width, config_.target_height);
    if (!resized) {
        return std::nullopt;
    }
    auto resize_end = std::chrono::high_resolution_clock::now();
    local_stats.resize_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        resize_end - resize_start
    ).count();

    // Step 3: Normalize and convert to tensor
    auto normalize_start = std::chrono::high_resolution_clock::now();
    auto tensor_data = normalize_and_convert_to_tensor(
        *resized,
        config_.target_width,
        config_.target_height
    );
    auto normalize_end = std::chrono::high_resolution_clock::now();
    local_stats.normalize_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        normalize_end - normalize_start
    ).count();

    auto end_time = std::chrono::high_resolution_clock::now();
    local_stats.total_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    ).count();
    local_stats.processed_size_bytes = tensor_data.size() * sizeof(float);

    if (stats) {
        *stats = local_stats;
    }

    // Check performance budget
    if (local_stats.total_time_ms > config_.max_processing_time_ms) {
        // Log warning but don't fail
    }

    ProcessedImage result{};
    result.tensor_data = std::move(tensor_data);
    result.channels = 3;
    result.height = config_.target_height;
    result.width = config_.target_width;
    result.batch_size = 1;

    return result;
}

std::optional<std::vector<uint8_t>> ImageProcessor::decode(
    const std::vector<uint8_t>& encoded_data
) {
    int width, height, channels;
    unsigned char* data = stbi_load_from_memory(
        encoded_data.data(),
        encoded_data.size(),
        &width,
        &height,
        &channels,
        4  // Force RGBA
    );

    if (!data) {
        return std::nullopt;
    }

    size_t size = width * height * 4;
    std::vector<uint8_t> result(data, data + size);
    stbi_image_free(data);

    return result;
}

std::optional<std::vector<uint8_t>> ImageProcessor::apply_exif_rotation(
    const std::vector<uint8_t>& rgba_data,
    int width,
    int height
) {
    // TODO: Implement EXIF rotation based on EXIF orientation tag
    // For now, just return the data as-is
    return rgba_data;
}

std::optional<std::vector<uint8_t>> ImageProcessor::resize(
    const std::vector<uint8_t>& rgba_data,
    int src_w,
    int src_h,
    int dst_w,
    int dst_h
) {
    std::vector<uint8_t> result(dst_w * dst_h * 4);

    float x_ratio = static_cast<float>(src_w) / dst_w;
    float y_ratio = static_cast<float>(src_h) / dst_h;

    for (int y = 0; y < dst_h; ++y) {
        for (int x = 0; x < dst_w; ++x) {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;

            for (int c = 0; c < 4; ++c) {
                uint8_t pixel = sample_pixel(rgba_data, src_w, src_h, 4, src_x, src_y, c);
                result[(y * dst_w + x) * 4 + c] = pixel;
            }
        }
    }

    return result;
}

uint8_t ImageProcessor::sample_pixel(
    const std::vector<uint8_t>& data,
    int w,
    int h,
    int channels,
    float x,
    float y,
    int channel
) {
    // Bilinear interpolation
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int x1 = std::min(x0 + 1, w - 1);
    int y1 = std::min(y0 + 1, h - 1);

    x0 = std::max(0, std::min(x0, w - 1));
    y0 = std::max(0, std::min(y0, h - 1));

    float dx = x - x0;
    float dy = y - y0;

    auto get_pixel = [&](int px, int py) -> uint8_t {
        return data[(py * w + px) * channels + channel];
    };

    float p00 = get_pixel(x0, y0);
    float p10 = get_pixel(x1, y0);
    float p01 = get_pixel(x0, y1);
    float p11 = get_pixel(x1, y1);

    float p0 = p00 * (1 - dx) + p10 * dx;
    float p1 = p01 * (1 - dx) + p11 * dx;
    float p = p0 * (1 - dy) + p1 * dy;

    return static_cast<uint8_t>(std::clamp(p, 0.0f, 255.0f));
}

std::vector<float> ImageProcessor::normalize_and_convert_to_tensor(
    const std::vector<uint8_t>& rgba_data,
    int width,
    int height
) {
    // Convert RGBA to RGB and apply ImageNet normalization
    // Output format: NCHW (batch × channels × height × width)
    std::vector<float> tensor(1 * 3 * height * width);

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            int rgba_idx = (h * width + w) * 4;

            // RGB channels (skip A)
            for (int c = 0; c < 3; ++c) {
                // Convert to [0, 1]
                float pixel = rgba_data[rgba_idx + c] / 255.0f;

                // Apply ImageNet normalization
                pixel = (pixel - config_.mean[c]) / config_.std[c];

                // Store in NCHW format
                int tensor_idx = c * (height * width) + h * width + w;
                tensor[tensor_idx] = pixel;
            }
        }
    }

    return tensor;
}

} // namespace forge
