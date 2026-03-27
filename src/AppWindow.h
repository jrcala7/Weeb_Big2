#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
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

    /// Callback invoked after the render loop ends but before bgfx is shut down.
    using ShutdownCallback = std::function<void()>;

    /// @param title  Window title.
    /// @param size   Window dimensions in pixels.
    explicit AppWindow(const std::string& title, glm::ivec2 size = {800, 600});

    /// Set a per-frame render callback.
    void SetRenderCallback(RenderCallback callback);

    /// Set a callback that runs after the render loop exits but while bgfx is
    /// still alive.  Use this to destroy GPU resources (programs, uniforms, etc.).
    void SetShutdownCallback(ShutdownCallback callback);

    /// Set the clear color (RGBA, e.g. 0x443355FF).
    void SetClearColor(std::uint32_t rgba);

    /// Initialize bgfx and the underlying windowing system.
    /// Call this before any bgfx-dependent work (e.g. loading textures).
    /// If not called explicitly, Run() will call it automatically.
    void Init();

    /// Start the blocking render loop. Returns when the window is closed.
    void Run();

private:
    std::string title_;
    glm::ivec2  size_;
    std::uint32_t clear_color_ = 0x443355FF;
    RenderCallback render_callback_;
    ShutdownCallback shutdown_callback_;
    std::unique_ptr<big2::App> app_;
};
