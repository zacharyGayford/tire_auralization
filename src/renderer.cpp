#include "renderer.h"

#include "log.h"

namespace renderer {
	typedef struct Renderer {
	} Renderer;

	Renderer* create(allocator::Arena* arena, RendererCreateInfo createInfo) {
		Renderer* renderer = (Renderer*)allocator::arena_allocate(arena, sizeof(Renderer));

		return renderer;
	}

	void* main(void* data) {
		Renderer* renderer = (Renderer*)data;
		return NULL;
	}
}
