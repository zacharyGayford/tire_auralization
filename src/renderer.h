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

	typedef allocator::Arena RenderCommandBuffer;

	Renderer* create(allocator::Arena* arena, RendererCreateInfo createInfo);
	void* main(void* data);

	RenderCommandBuffer* render_command_buffer_create(allocator::Arena* arena, u32 byteCapacity);
	void render_command_buffer_publish(Renderer* renderer, RenderCommandBuffer* commandBuffer);
	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> p[4]);
	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> bottomRight, v2<f32> topLeft, v2<f32> topRight);
	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> p[4]);
	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> bottomRight, v2<f32> topLeft, v2<f32> topRight);
	void render_command_buffer_circle_push(RenderCommandBuffer* buffer, f32 x, f32 y, f32 radius);
	void render_command_buffer_circle_push(RenderCommandBuffer* buffer, v2<f32> center, f32 radius);
	void render_command_buffer_text_push(RenderCommandBuffer* buffer, char* text, f32 x, f32 y, TextAnchorPoint anchor);
	void render_command_buffer_clear(RenderCommandBuffer* buffer, v3<f32> color);
}

#endif // RENDERER_H
