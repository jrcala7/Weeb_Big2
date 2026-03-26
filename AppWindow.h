#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include <big2/app.h>
#include <big2/app_extension_base.h>
#include <big2/window.h>

/// @brief Wraps a big2::App to provide a single window with a bgfx Vulkan render loop.
class AppWindow final {
public:
    /// Callback invoked each frame for custom rendering.
    /// @param window  The big2 window being rendered.
    /// @param dt      Delta time in seconds since the last frame.
    using RenderCallback = std::function<void(big2::Window& window, float dt)>;

    /// @param title  Window title.
    /// @param size   Window dimensions in pixels.
    explicit AppWindow(const std::string& title, glm::ivec2 size = {800, 600});

    /// Set a per-frame render callback.
    void SetRenderCallback(RenderCallback callback);

    /// Set the clear color (RGBA, e.g. 0x443355FF).
    void SetClearColor(std::uint32_t rgba);

    /// Start the blocking render loop. Returns when the window is closed.
    void Run();

private:
    std::string title_;
    glm::ivec2  size_;
    std::uint32_t clear_color_ = 0x443355FF;
    RenderCallback render_callback_;
};
