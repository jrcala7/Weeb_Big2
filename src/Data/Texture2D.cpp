#include "Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>
#include <cmath>
#include <algorithm>

Texture2D::~Texture2D() {
    Unload();
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : handle_(other.handle_)
    , width_(other.width_)
    , height_(other.height_)
    , channels_(other.channels_)
    , error_(std::move(other.error_))
    , pixels_(std::move(other.pixels_)) {
    other.handle_ = BGFX_INVALID_HANDLE;
    other.width_ = 0;
    other.height_ = 0;
    other.channels_ = 0;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this != &other) {
        Unload();
        handle_   = other.handle_;
        width_    = other.width_;
        height_   = other.height_;
        channels_ = other.channels_;
        error_    = std::move(other.error_);
        pixels_   = std::move(other.pixels_);
        other.handle_ = BGFX_INVALID_HANDLE;
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
    }
    return *this;
}

bool Texture2D::Load(const std::string& path) {
    Unload();
    error_.clear();

    int w = 0, h = 0, c = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &c, 4);
    if (pixels == nullptr) {
        error_ = "stb_image: ";
        error_ += stbi_failure_reason();
        return false;
    }

    width_    = static_cast<std::uint32_t>(w);
    height_   = static_cast<std::uint32_t>(h);
    channels_ = 4; // We always request RGBA.

    // Store the pixel data for later use (e.g., for blur operations)
    std::uint32_t pixelCount = width_ * height_ * channels_;
    pixels_.resize(pixelCount);
    std::copy(pixels, pixels + pixelCount, pixels_.begin());

    const bgfx::Memory* mem = bgfx::copy(pixels, pixelCount);
    stbi_image_free(pixels);

    handle_ = bgfx::createTexture2D(
        static_cast<uint16_t>(width_),
        static_cast<uint16_t>(height_),
        false,  // no mipmaps
        1,      // single layer
        bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE,
        mem
    );

    if (!bgfx::isValid(handle_)) {
        error_ = "bgfx: failed to create texture";
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        pixels_.clear();
        return false;
    }

    return true;
}

void Texture2D::Unload() {
    if (bgfx::isValid(handle_)) {
        bgfx::destroy(handle_);
        handle_ = BGFX_INVALID_HANDLE;
    }
    width_ = 0;
    height_ = 0;
    channels_ = 0;
    pixels_.clear();
}

Texture2D Texture2D::ApplyGaussianBlur(int radius) const {
    Texture2D blurred;

    // Validate input
    if (!IsLoaded() || width_ == 0 || height_ == 0 || radius <= 0 || pixels_.empty()) {
        blurred.error_ = "Invalid texture or blur radius for Gaussian blur";
        return blurred;
    }

    // Use the stored pixel data
    const uint8_t* srcPixels = pixels_.data();
    std::uint32_t pixelCount = width_ * height_ * channels_;
    uint8_t* dstPixels = new uint8_t[pixelCount];

    // Compute Gaussian kernel
    int kernelSize = 2 * radius + 1;
    std::vector<float> kernel(kernelSize);
    float sigma = radius / 2.0f;
    float sum = 0.0f;

    for (int i = 0; i < kernelSize; ++i) {
        int x = i - radius;
        float val = std::exp(-(x * x) / (2.0f * sigma * sigma));
        kernel[i] = val;
        sum += val;
    }

    // Normalize kernel
    for (float& k : kernel) {
        k /= sum;
    }

    // Apply separable Gaussian blur (horizontal then vertical)
    // Horizontal pass
    std::vector<uint8_t> temp(pixelCount);
    for (std::uint32_t y = 0; y < height_; ++y) {
        for (std::uint32_t x = 0; x < width_; ++x) {
            for (std::uint32_t c = 0; c < channels_; ++c) {
                float blurValue = 0.0f;
                for (int i = -radius; i <= radius; ++i) {
                    int sampleX = static_cast<int>(x) + i;
                    // Clamp to edge
                    sampleX = std::max(0, std::min(static_cast<int>(width_ - 1), sampleX));
                    uint32_t srcIdx = (y * width_ + sampleX) * channels_ + c;
                    blurValue += srcPixels[srcIdx] * kernel[i + radius];
                }
                uint32_t dstIdx = (y * width_ + x) * channels_ + c;
                temp[dstIdx] = static_cast<uint8_t>(blurValue + 0.5f);
            }
        }
    }

    // Vertical pass
    for (std::uint32_t y = 0; y < height_; ++y) {
        for (std::uint32_t x = 0; x < width_; ++x) {
            for (std::uint32_t c = 0; c < channels_; ++c) {
                float blurValue = 0.0f;
                for (int i = -radius; i <= radius; ++i) {
                    int sampleY = static_cast<int>(y) + i;
                    // Clamp to edge
                    sampleY = std::max(0, std::min(static_cast<int>(height_ - 1), sampleY));
                    uint32_t srcIdx = (sampleY * width_ + x) * channels_ + c;
                    blurValue += temp[srcIdx] * kernel[i + radius];
                }
                uint32_t dstIdx = (y * width_ + x) * channels_ + c;
                dstPixels[dstIdx] = static_cast<uint8_t>(blurValue + 0.5f);
            }
        }
    }

    // Store the blurred pixel data first (before freeing the temporary buffer)
    blurred.pixels_.resize(pixelCount);
    std::copy(dstPixels, dstPixels + pixelCount, blurred.pixels_.begin());

    // Create a new bgfx texture from the blurred pixels
    const bgfx::Memory* blurredMem = bgfx::copy(dstPixels, pixelCount);
    delete[] dstPixels;

    bgfx::TextureHandle blurredHandle = bgfx::createTexture2D(
        static_cast<uint16_t>(width_),
        static_cast<uint16_t>(height_),
        false,  // no mipmaps
        1,      // single layer
        bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_NONE,
        blurredMem
    );

    if (!bgfx::isValid(blurredHandle)) {
        blurred.error_ = "bgfx: failed to create blurred texture";
        return blurred;
    }

    // Set the blurred texture properties
    blurred.handle_   = blurredHandle;
    blurred.width_    = width_;
    blurred.height_   = height_;
    blurred.channels_ = channels_;

    return blurred;
}

