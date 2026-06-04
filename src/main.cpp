#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "defines.h"
#include "renderer.h"
#include "window.h"
#include "allocator.h"
#include "thread.h"
#include "log.h"
#include "audio.h"
#include "container.h"

using thread::Thread;

typedef struct Temp {
	u8 a;
	u32 b;
	u64 c;
} Temp;

int main() {
	log::init(log::LEVEL_DEBUG, "out.log");
	allocator::Arena* staticArena = allocator::arena_create(MEBIBYTES(16));
	allocator::Arena* volatileArena = allocator::arena_create(MEBIBYTES(16));

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
	rendererCreateInfo.renderCommandBufferByteSize = KIBIBYTES(16);
	rendererCreateInfo.framesInFlight = 3;
	renderer::Renderer* renderer = renderer::create(staticArena, rendererCreateInfo);
	if (!renderer) {
		log_fatal("unable to create renderer");
		return 2;
	}

	Thread* audioThread = thread::create(staticArena, "audio_thread", audio::main);

	thread::run(audioThread, NULL);

	renderer::RenderCommandBuffer* buffer = renderer::render_command_buffer_create(volatileArena, rendererCreateInfo.renderCommandBufferByteSize);
	renderer::render_command_buffer_fill_set(buffer, {1.0f, 1.0f, 1.0f});
	renderer::render_command_buffer_quad_push(buffer, {-0.5, -0.5}, {0.5, 0.5});

	log_debug("hello from the main thread");
	bool running = true;
	while (!window::should_close(window)) {
		window::events_poll();
		renderer::render(renderer, buffer);
	}

	window::destroy_all(&window, 1);
}
