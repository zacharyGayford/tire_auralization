#include "renderer.h"

#include "log.h"

namespace renderer {
	typedef struct Renderer {
		window::Window* window;
	} Renderer;

	Renderer* create(allocator::Arena* arena, RendererCreateInfo createInfo) {
		Renderer* renderer = (Renderer*)allocator::arena_allocate(arena, sizeof(Renderer));
		renderer->window = createInfo.window;

		return renderer;
	}

	void* main(void* data) {
		Renderer* renderer = (Renderer*)data;
		log_debug("Hello from the renderer thread!");
		return NULL;
	}
}
