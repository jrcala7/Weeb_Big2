// Weeb_Big2.cpp : Defines the entry point for the application.
//

#include "Weeb_Big2.h"

#include <bgfx/bgfx.h>
#include <gsl/gsl>

#include "Data/Model3D.h"
#include "Data/Camera.h"
#include "Rendering/ModelRenderer.h"

int main(std::int32_t, gsl::zstring[]) {
	AppWindow window("Weeb Big2", {1280, 720});
	window.SetClearColor(0x443355FF);

	// Load the bunny model.
	Model3D bunny;
	if (!bunny.Load("resources/3D/bunny.obj")) {
		std::cerr << "Failed to load bunny: " << bunny.GetError() << std::endl;
		return 1;
	}
	bunny.SetScale({10.0f, 10.0f, 10.0f});

	// Place the camera so it faces the bunny at the origin.
	// Forward at yaw=0 is +Z, so the camera sits on -Z looking toward it.
	Camera camera(
		{0.0f, 0.8f, -2.5f},  // position  (slightly above center, behind on Z)
		{0.0f, 0.0f, 0.0f},   // rotation  (looking straight +Z)
		60.0f,                 // fov
		1280.0f / 720.0f       // aspect ratio matching the window
	);

	ModelRenderer renderer;

	window.SetRenderCallback([&](big2::Window& win, float /*dt*/) {
		// Lazy-init: ModelRenderer needs bgfx to be up, which happens inside Run().
		if (!renderer.IsInitialized()) {
			renderer.Init();
			renderer.SetColor({0.0f, 0.8f, 0.0f, 1.0f});              // green
			renderer.SetLightDirection({-0.5f, -1.0f, -0.5f});
		}

		// Clear both color and depth each frame so the depth test works.
		bgfx::setViewClear(win.GetView(),
			BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
			0x443355FF, 1.0f, 0);
		bgfx::touch(win.GetView());
		renderer.Render(win.GetView(), bunny, camera);
	});

	// Release GPU resources while bgfx is still alive (before Run() destroys it).
	window.SetShutdownCallback([&]() {
		renderer.Shutdown();
	});

	window.Run();
	return 0;
}
