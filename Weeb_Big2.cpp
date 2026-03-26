// Weeb_Big2.cpp : Defines the entry point for the application.
//

#include "Weeb_Big2.h"

#include <bgfx/bgfx.h>
#include <gsl/gsl>

int main(std::int32_t, gsl::zstring[]) {
	AppWindow window("Weeb Big2", {1280, 720});
	window.SetClearColor(0x443355FF);

	window.SetRenderCallback([](big2::Window& win, float /*dt*/) {
		// Touch the view so bgfx clears it each frame.
		bgfx::touch(win.GetView());
	});

	window.Run();
	return 0;
}
