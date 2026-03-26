#include "AppWindow.h"

#include <big2.h>
#include <big2/default_quit_condition_app_extension.h>
#include <bgfx/bgfx.h>

// ---------------------------------------------------------------------------
// File-scope static used to pass the callback into the extension at
// construction time (big2 creates extensions via default-construct in
// AddExtension<T>()).
// ---------------------------------------------------------------------------
static std::shared_ptr<AppWindow::RenderCallback> s_pending_cb;

// ---------------------------------------------------------------------------
// App extension that forwards OnRender to the user-provided callback.
// ---------------------------------------------------------------------------
class RenderAppExtension final : public big2::AppExtensionBase {
public:
    RenderAppExtension() : cb_(s_pending_cb) {}

protected:
    void OnRender(big2::Window& window) override {
        AppExtensionBase::OnRender(window);
        if (cb_ && *cb_) {
            (*cb_)(window, app_->GetDeltaTime());
        }
    }

private:
    std::shared_ptr<AppWindow::RenderCallback> cb_;
};

// ---------------------------------------------------------------------------
// AppWindow implementation
// ---------------------------------------------------------------------------
AppWindow::AppWindow(const std::string& title, glm::ivec2 size)
    : title_(title), size_(size) {}

void AppWindow::SetRenderCallback(RenderCallback callback) {
    render_callback_ = std::move(callback);
}

void AppWindow::SetShutdownCallback(ShutdownCallback callback) {
    shutdown_callback_ = std::move(callback);
}

void AppWindow::SetClearColor(std::uint32_t rgba) {
    clear_color_ = rgba;
}

void AppWindow::Run() {
    // Force the Vulkan renderer.
    big2::App app(bgfx::RendererType::Vulkan);

    app.AddExtension<big2::DefaultQuitConditionAppExtension>();

    // Store the callback in the file-scope static so RenderAppExtension can
    // pick it up during construction inside AddExtension<>().
    s_pending_cb = std::make_shared<RenderCallback>(render_callback_);
    app.AddExtension<RenderAppExtension>();
    s_pending_cb.reset();

    big2::Window& window = app.AddWindow(title_, size_);
    window.SetClearColor(clear_color_);

    app.Run();

    // Let the application clean up GPU resources while bgfx is still alive
    // (the local `app` — and therefore bgfx — is destroyed at the end of
    // this scope).
    if (shutdown_callback_) {
        shutdown_callback_();
    }
}
