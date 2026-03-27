#pragma once

#include <cstdint>
#include <string>
#include <bgfx/bgfx.h>

/// @brief Holds a 2D texture loaded from an image file.
///        Manages both CPU pixel data (via stb_image) and a bgfx GPU handle.
class Texture2D final {
public:
    Texture2D() = default;
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&&) noexcept;
    Texture2D& operator=(Texture2D&&) noexcept;

    /// Load an image from the given file path and upload it to the GPU.
    /// Supported formats: PNG, JPG, BMP, TGA, etc. (anything stb_image can read).
    /// @param path  Filesystem path to the image file.
    /// @return true on success, false on failure (call GetError() for details).
    bool Load(const std::string& path);

    /// Release all resources (CPU + GPU).
    void Unload();

    /// @return true if the texture was loaded successfully.
    [[nodiscard]] bool IsLoaded() const { return bgfx::isValid(handle_); }

    /// @return A human-readable error string from the last failed Load().
    [[nodiscard]] const std::string& GetError() const { return error_; }

    // -- Accessors ------------------------------------------------------------

    /// @return Texture width in pixels.
    [[nodiscard]] std::uint32_t GetWidth() const { return width_; }

    /// @return Texture height in pixels.
    [[nodiscard]] std::uint32_t GetHeight() const { return height_; }

    /// @return Number of color channels (e.g. 4 for RGBA).
    [[nodiscard]] std::uint32_t GetChannels() const { return channels_; }

    /// @return The bgfx texture handle. Only valid when IsLoaded() is true.
    [[nodiscard]] bgfx::TextureHandle GetHandle() const { return handle_; }

private:
    bgfx::TextureHandle handle_ = BGFX_INVALID_HANDLE;
    std::uint32_t width_    = 0;
    std::uint32_t height_   = 0;
    std::uint32_t channels_ = 0;
    std::string error_;
};
