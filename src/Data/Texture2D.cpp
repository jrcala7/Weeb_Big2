#include "Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture2D::~Texture2D() {
    Unload();
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : handle_(other.handle_)
    , width_(other.width_)
    , height_(other.height_)
    , channels_(other.channels_)
    , error_(std::move(other.error_)) {
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

    const bgfx::Memory* mem = bgfx::copy(pixels, width_ * height_ * channels_);
    stbi_image_free(pixels);

    handle_ = bgfx::createTexture2D(
        static_cast<uint16_t>(width_),
        static_cast<uint16_t>(height_),
        false,  // no mipmaps
        1,      // single layer
        bgfx::TextureFormat::RGBA8,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        mem
    );

    if (!bgfx::isValid(handle_)) {
        error_ = "bgfx: failed to create texture";
        width_ = 0;
        height_ = 0;
        channels_ = 0;
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
}
