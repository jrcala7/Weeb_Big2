#include "AppWindow.h"

#include <big2.h>
#include <big2/default_quit_condition_app_extension.h>
#if BIG2_IMGUI_ENABLED
#include <big2/imgui/imgui.h>
#include <big2/imgui/imgui_context_wrapper.h>
#include <big2/bgfx/bgfx_utils.h>
#endif
#include <bgfx/bgfx.h>

// ---------------------------------------------------------------------------
// File-scope static used to pass the callback into the extension at
// construction time (big2 creates extensions via default-construct in
// AddExtension<T>()).
// ---------------------------------------------------------------------------
static std::shared_ptr<AppWindow::RenderCallback> s_pending_cb;

// ---------------------------------------------------------------------------
// ImGui extension that uses a SEPARATE bgfx view so it does not overwrite
// the 3D camera's view/projection matrices on the main window view.
// ---------------------------------------------------------------------------
#if BIG2_IMGUI_ENABLED
class ImGuiOverlayExtension final : public big2::AppExtensionBase {
protected:
    void OnWindowCreated(big2::Window& window) override {
        AppExtensionBase::OnWindowCreated(window);
        imgui_view_ = big2::ReserveViewId();

        context_.SetIsScoped(false);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        context_.Initialize(window, imgui_view_, false);
        ImGui::StyleColorsDark();
    }

    void OnWindowDestroyed(big2::Window& window) override {
        AppExtensionBase::OnWindowDestroyed(window);
        context_.Dispose();
        big2::FreeViewId(imgui_view_);
    }

    void OnUpdate(std::float_t dt) override {
        AppExtensionBase::OnUpdate(dt);
        for (big2::Window& window : app_->GetWindows()) {
            ImGui::SetCurrentContext(context_.GetContext());
            big2::GlfwEventQueue::UpdateImGuiEvents(window);
        }
    }

private:
    big2::ImGuiContextWrapper context_;
    bgfx::ViewId imgui_view_ = BGFX_INVALID_HANDLE;
};
#endif

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

#if BIG2_IMGUI_ENABLED
    app.AddExtension<ImGuiOverlayExtension>();
#endif

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
