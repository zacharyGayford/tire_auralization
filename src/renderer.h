#ifndef RENDERER_H
#define RENDERER_H

#include "defines.h"
#include "allocator.h"
#include "window.h"

namespace renderer {
	typedef struct Renderer Renderer;

	typedef struct RendererCreateInfo {
		window::Window* window;
	} RendererCreateInfo;

	Renderer* create(allocator::Arena* arena, RendererCreateInfo createInfo);
	void* main(void* data);
}

#endif // RENDERER_H
