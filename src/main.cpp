#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "defines.h"
#include "renderer.h"
#include "window.h"
#include "allocator.h"
#include "thread.h"
#include "log.h"

using thread::Thread;

int main() {
	allocator::Arena* staticArena = allocator::arena_create(MEBIBYTES(10));

	window::WindowCreateInfo windowCreateInfo = {0};
	windowCreateInfo.dimensions = { 500, 500 };
	windowCreateInfo.title = "tire_auralization";
	window::Window* window = window::create(staticArena, windowCreateInfo);
	if (!window) {
		log_fatal("unable to create window");
		return 1;
	}

	renderer::RendererCreateInfo rendererCreateInfo = {0};
	rendererCreateInfo.window = window;
	renderer::Renderer* renderer = renderer::create(staticArena, rendererCreateInfo);
	if (!renderer) {
		log_fatal("unable to create renderer");
		return 2;
	}

	Thread* rendererThread = thread::create(staticArena, "renderer_thread", renderer::main);
	// Thread* audioThread    = thread::create(staticArena, "audio_thread", audio::main);

	bool running = true;
	while (running) {
		window::events_poll();
	}

	window::destroy_all(&window, 1);

}
