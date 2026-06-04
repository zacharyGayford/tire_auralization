#ifndef WINDOW_H
#define WINDOW_H

#include "defines.h"
#include "allocator.h"
#include "math.h"

namespace window {
	typedef struct Window Window;

	typedef struct WindowCreateInfo {
		v2<i32> dimensions;
		const char* title;
	} WindowCreateInfo;
	Window* create(allocator::Arena* arena, WindowCreateInfo createInfo);
	void destroy_all(Window** window, u32 numWindows);
	v2<i32> dimensions_get(Window* window);
	u32 opengl_init(Window* window);
	void buffers_swap(Window* window);
	void close(Window* window);
	bool should_close(Window* window);
	void events_poll();
}

#endif // WINDOW_H
