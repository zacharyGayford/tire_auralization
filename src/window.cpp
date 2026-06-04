#include "window.h"

#include <atomic>
#include <memory.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "defines.h"
#include "log.h"
#include "allocator.h"
#include "math.h"

namespace window {
	static std::atomic<bool> glfwInitialized = false;

	typedef struct Window {
		GLFWwindow* handle;
		v2<i32> dimensions;
	} Window;

	internal void glfw_framebuffer_resize_callback(GLFWwindow* w, int width, int height) {
		Window* window = (Window*)glfwGetWindowUserPointer(w);
		window->dimensions = (v2<i32>) { .width = width, .height = height };
	}

	internal void glfw_key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(w, GLFW_TRUE);
	}

	internal void* glfw_allocate(size_t size, void* user) {
		log_debug("glfw_allocate(size: %d)", size);
		void* newAlloc = allocator::arena_allocate((allocator::Arena*)user, size);
		if (newAlloc == NULL) {
			log_fatal("glfw allocation ran out of memory");
			return NULL;
		}
		return newAlloc;
	}

	internal void* glfw_reallocate(void* block, size_t size, void* user) {
		log_debug("glfw_reallocate(block: %p, size: %d)", block, size);
		unimplemented();
		return NULL;
	}

	internal void glfw_deallocate(void* block, void* user) {
		log_debug("glfw_deallocate(block: %p)", block);
	}

	Window* create(allocator::Arena* arena, WindowCreateInfo createInfo) {
		if (!glfwInitialized) {
			if (!glfwInit()) {
				log_fatal("failed to initialized glfw");
				return NULL;
			}
			glfwInitialized = true;
		}

		GLFWallocator glfwAllocator = {
			.allocate   = glfw_allocate,
			.reallocate = glfw_reallocate,
			.deallocate = glfw_deallocate,
			.user       = arena
		};
		glfwInitAllocator(&glfwAllocator);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		Window* window = (Window*)allocator::arena_allocate(arena, sizeof(Window));
		window->dimensions = createInfo.dimensions;
		window->handle = glfwCreateWindow(createInfo.dimensions.width, createInfo.dimensions.height, createInfo.title, NULL, NULL);
		if (!window->handle) {
			log_fatal("failed to open window");
			return NULL;
		}

		glfwSetWindowUserPointer(window->handle, window);
		glfwSetKeyCallback(window->handle, glfw_key_callback);
		glfwSetFramebufferSizeCallback(window->handle, glfw_framebuffer_resize_callback);

		return window;
	}

	void destroy_all(Window** window, u32 numWindows) {
		for (u32 i = 0; i < numWindows; i++) {
			glfwDestroyWindow((*window)->handle);
		}
		glfwTerminate();
	}

	v2<i32> dimensions_get(Window* window) {
		return window->dimensions;
	}

	u32 opengl_init(Window* window) {
		glfwMakeContextCurrent(window->handle);
		if (!gladLoadGL(glfwGetProcAddress)) {
			log_fatal("failed to load opengl functoins");
			return 1;
		}
		return 0;
	}

	void buffers_swap(Window* window) {
		glfwSwapBuffers(window->handle);
	}

	void close(Window* window) {
		glfwWindowShouldClose(window->handle);
	}

	bool should_close(Window* window) {
		return glfwWindowShouldClose(window->handle);
	}

	void events_poll() {
		glfwPollEvents();
	}
}
