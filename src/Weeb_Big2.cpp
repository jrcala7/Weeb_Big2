// Weeb_Big2.cpp : Defines the entry point for the application.
//

#include "Weeb_Big2.h"

#include <bgfx/bgfx.h>
#include <gsl/gsl>

#if BIG2_IMGUI_ENABLED
#include <big2/imgui/imgui.h>
#include <big2/macros.h>
#endif

#include "Data/Model3D.h"
#include "Data/Camera.h"
#include "Data/Light.h"
#include "Rendering/ModelRenderer.h"
#include "UI/CameraDebugUI.h"
#include "UI/LightDebugUI.h"
#include "UI/ModelDebugUI.h"

/// Load and configure the test bunny model.
/// @param[out] model  The Model3D instance to populate.
/// @return true on success, false on failure (error available via model.GetError()).
static bool RenderTestRabbit(Model3D& model) {
	if (!model.Load("resources/3D/bunny.obj")) {
		return false;
	}
	model.SetScale({10.0f, 10.0f, 10.0f});
	model.SetBaseColor({0.0f, 0.8f, 0.0f, 1.0f});    // green
	model.SetShadowColor({0.0f, 0.15f, 0.0f, 1.0f}); // dark green shadow
	model.SetRotation({ 0.0f, 185.0f, 0.0f });
	return true;
}

/// Load the DJ sword model and replace its texture with partenza.jpg.
/// @param[out] model  The Model3D instance to populate.
/// @return true on success, false on failure (error available via model.GetError()).
static bool LoadDjSword(Model3D& model) {
	if (!model.Load("resources/3D/djSword.obj")) {
		return false;
	}
	if (!model.ReplaceAllTextures("resources/3D/partenza.jpg")) {
		return false;
	}
	model.SetScale({ 0.02f, 0.02f, 0.02f });
	model.SetPosition({0.0f, 0.7f, 0.0f});
	return true;
}

/// Load the Sponza palace model.
/// @param[out] model  The Model3D instance to populate.
/// @return true on success, false on failure (error available via model.GetError()).
static bool LoadSponza(Model3D& model) {
	if (!model.Load("resources/3D/sponza/Sponza.gltf")) {
		return false;
	}
	model.SetScale({1.0f, 1.0f, 1.0f});
	return true;
}

int main(std::int32_t, gsl::zstring[]) {
	AppWindow window("Weeb Big2", {1280, 720});
	window.SetClearColor(0x443355FF);

	// Initialize bgfx early so that model/texture loading can use the GPU.
	window.Init();

	// 0 = bunny, 1 = sword, 2 = sponza
	constexpr int kModelChoice = 2;

	Model3D model;
	if (kModelChoice == 1) {
		if (!LoadDjSword(model)) {
			std::cerr << "Failed to load sword: " << model.GetError() << std::endl;
			return 1;
		}
	} else if (kModelChoice == 2) {
		if (!LoadSponza(model)) {
			std::cerr << "Failed to load sponza: " << model.GetError() << std::endl;
			return 1;
		}
	} else {
		if (!RenderTestRabbit(model)) {
			std::cerr << "Failed to load bunny: " << model.GetError() << std::endl;
			return 1;
		}
	}

	// Place the camera so it faces the bunny at the origin.
	// Forward at yaw=0 is +Z, so the camera sits on -Z looking toward it.
	Camera camera(
		{0.0f, 0.8f, -2.5f},  // position  (slightly above center, behind on Z)
		{0.0f, 0.0f, 0.0f},   // rotation  (looking straight +Z)
		60.0f,                 // fov
		1280.0f / 720.0f       // aspect ratio matching the window
	);

	DirectionalLight light({-0.5f, -1.0f, -0.5f});

	ModelRenderer renderer;

	window.SetRenderCallback([&](big2::Window& win, float /*dt*/) {
		// Lazy-init: ModelRenderer needs bgfx to be up, which happens inside Run().
		if (!renderer.IsInitialized()) {
			renderer.Init();
		}

		// Clear both color and depth each frame so the depth test works.
		bgfx::setViewClear(win.GetView(),
			BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
			0x443355FF, 1.0f, 0);
		bgfx::touch(win.GetView());
		renderer.Render(win.GetView(), model, camera, light);

#if BIG2_IMGUI_ENABLED
		BIG2_SCOPE_VAR(big2::ImGuiFrameScoped) {
			DrawCameraDebugUI(camera);
			DrawLightDebugUI(light);
			DrawModelDebugUI(model);
		}
#endif
	});

	// Release GPU resources while bgfx is still alive (before Run() destroys it).
	window.SetShutdownCallback([&]() {
		renderer.Shutdown();
		model.Unload();  // Release GPU textures while bgfx is still alive.
	});

	window.Run();
	return 0;
}
