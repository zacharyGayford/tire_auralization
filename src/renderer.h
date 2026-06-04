#ifndef RENDERER_H
#define RENDERER_H

#include "defines.h"
#include "allocator.h"
#include "window.h"

namespace renderer {
	typedef struct Renderer Renderer;

	typedef struct RendererCreateInfo {
		window::Window* window;
		u64 renderCommandBufferByteSize;
		u64 framesInFlight;
	} RendererCreateInfo;

	typedef enum TextAnchorPoint : u8 {
		TEXT_ANCHOR_POINT_TOP_LEFT,
		TEXT_ANCHOR_POINT_TOP_RIGHT,
		TEXT_ANCHOR_POINT_BOTTOM_LEFT,
		TEXT_ANCHOR_POINT_BOTTOM_RIGHT,
		TEXT_ANCHOR_POINT_CENTER_LEFT,
		TEXT_ANCHOR_POINT_CENTER_RIGHT,
		TEXT_ANCHOR_POINT_TOP_CENTER,
		TEXT_ANCHOR_POINT_BOTTOM_CENTER,
		TEXT_ANCHOR_POINT_CENTER
	} TextAnchorPoint;

	typedef struct RenderCommandBuffer RenderCommandBuffer;

	Renderer* create(allocator::Arena* arena, RendererCreateInfo createInfo);
	void destroy(Renderer** renderer);
	void render(Renderer* renderer, RenderCommandBuffer* commandBuffer);

	RenderCommandBuffer* render_command_buffer_create(allocator::Arena* arena, u64 commandCapacity);
	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> p[2]);
	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> topRight);
	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> p[2]);
	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> topRight);
	void render_command_buffer_circle_push(RenderCommandBuffer* buffer, v2<f32> center, f32 radius);
	void render_command_buffer_fill_set(RenderCommandBuffer* buffer, v3<f32> fill);
	void render_command_buffer_stroke_set(RenderCommandBuffer* buffer, v3<f32> stroke);
}

#endif // RENDERER_H
