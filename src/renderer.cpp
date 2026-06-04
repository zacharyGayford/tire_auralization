#include "renderer.h"

#include "log.h"
#include "math.h"
#include "memory.h"
#include "container.h"

namespace renderer {
	typedef struct Renderer {
		window::Window* window;
		bool running = true;
		// NOTE(zgayford): this is overkill and a bit
		// inefficient but we're rolling with it for
		// now
		u64 framesInFlight;
		container::RingBuffer* commandBuffers;
	} Renderer;

	typedef enum RenderCommandType : u32 {
		RENDER_COMMAND_QUAD,
		RENDER_COMMAND_TEXTURED_QUAD,
		RENDER_COMMAND_CIRCLE,
		RENDER_COMMAND_TEXT,
		RENDER_COMMAND_CLEAR,
	} RenderCommandType;

	typedef struct RenderCommandHeader {
		RenderCommandType type;
		u64 size;
	} RenderCommandHeader;

	typedef struct RenderCommandQuad {
		v2<f32> p[4];
	} RenderCommandQuad;

	typedef struct RenderCommandTexturedQuad {
		v2<f32> p[4];
	} RenderCommandTexturedQuad;

	typedef struct RenderCommandCircle {
		v2<f32> p;
		f32 r;
	} RenderCommandCircle;

	typedef struct RenderCommandText {
		v2<f32> p;
		TextAnchorPoint anchor;
	} RenderCommandText;

	typedef struct RenderCommandClear {
		v3<f32> color;
	} RenderCommandClear;

	Renderer* create(allocator::Arena* arena, RendererCreateInfo createInfo) {
		Renderer* renderer = (Renderer*)allocator::arena_allocate(arena, sizeof(Renderer));
		if (renderer == NULL) {
			log_fatal("failed to create renderer, out of memory");
			return NULL;
		}

		renderer->window = createInfo.window;

		if (window::opengl_init(renderer->window)) {
			log_fatal("failed to initialize opengl");
			return NULL;
		}

		renderer->framesInFlight = createInfo.framesInFlight;
		renderer->commandBuffers = container::ring_buffer_create(arena, renderer->framesInFlight, createInfo.renderCommandBufferByteSize);

		return renderer;
	}

	void* main(void* data) {
		Renderer* renderer = (Renderer*)data;
		log_debug("starting render thread");

		RenderCommandBuffer* buffer;
		while (1) {
			log_info("tryin to pop");
			if (container::ring_buffer_try_pop(renderer->commandBuffers, buffer)) {
				log_info("stuff to render!");
			}
		}

		return NULL;
	}

	RenderCommandBuffer* render_command_buffer_create(allocator::Arena* arena, u32 byteCapacity) {
		return (RenderCommandBuffer*)allocator::arena_create(arena, byteCapacity + sizeof(RenderCommandType));
	}

	void render_command_buffer_publish(Renderer* renderer, RenderCommandBuffer* commandBuffer) {
		if (!container::ring_buffer_try_push(renderer->commandBuffers, commandBuffer))
			log_warn("renderer is overloaded; dropping frame");
	}

	void render_command_buffer_push(RenderCommandBuffer* buffer, RenderCommandHeader header, void* data) {
		RenderCommandHeader* commandStream = (RenderCommandHeader*)
				allocator::arena_allocate(buffer, sizeof(RenderCommandHeader) + header.size);
		if (commandStream == NULL) {
			log_error("render command buffer out of memory");
			return;
		}

		// write the header
		*(commandStream) = header;

		// write the data
		memcpy(commandStream + 1, data, header.size);
	}

	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> p[4]) {
		RenderCommandHeader header = (RenderCommandHeader) {
			.type = RENDER_COMMAND_QUAD,
			.size = sizeof(RenderCommandQuad)
		};
		RenderCommandQuad command = {0};
		memcpy(command.p, p, 4&sizeof(v2<f32>));
		render_command_buffer_push(buffer, header, &command);
	}

	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> bottomRight, v2<f32> topLeft, v2<f32> topRight) {
		RenderCommandHeader header = (RenderCommandHeader) {
			.type = RENDER_COMMAND_QUAD,
			.size = sizeof(RenderCommandQuad)
		};
		RenderCommandQuad command = {
			.p = { bottomLeft, bottomRight, topLeft, topRight },
		};
		render_command_buffer_push(buffer, header, &command);
	}

	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> p[4]) {
		unimplemented();
	}

	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> bottomRight, v2<f32> topLeft, v2<f32> topRight) {
		unimplemented();
	}

	void render_command_buffer_circle_push(RenderCommandBuffer* buffer, f32 x, f32 y, f32 radius) {
		RenderCommandHeader header = (RenderCommandHeader) {
			.type = RENDER_COMMAND_CIRCLE,
			.size = sizeof(RenderCommandCircle)
		};
		RenderCommandCircle command = {
			.p = { x, y },
			.r = radius,
		};
		render_command_buffer_push(buffer, header, &command);
	}

	void render_command_buffer_circle_push(RenderCommandBuffer* buffer, v2<f32> center, f32 radius) {
		RenderCommandHeader header = (RenderCommandHeader) {
			.type = RENDER_COMMAND_CIRCLE,
			.size = sizeof(RenderCommandCircle)
		};
		RenderCommandCircle command = {
			.p = center,
			.r = radius,
		};
		render_command_buffer_push(buffer, header, &command);
	}

	void render_command_buffer_text_push(RenderCommandBuffer* buffer, char* text, f32 x, f32 y, TextAnchorPoint anchor) {
		unimplemented();
	}
	
	void render_command_buffer_clear(RenderCommandBuffer* buffer, v3<f32> color) {
		unimplemented();
	}
}
