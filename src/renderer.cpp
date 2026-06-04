#include "renderer.h"

#include <stdio.h>
#include <glad/gl.h>

#include "log.h"
#include "math.h"
#include "memory.h"
#include "container.h"

#define VERT_SHADER_PATH "../src/vert.glsl"
#define FRAG_SHADER_PATH "../src/frag.glsl"

// NOTE(zgayford): while this renderer is currently strictly opengl,
// it was written in such a way where a new rendering backend could
// be swapped in with a managable amount of effort.

namespace renderer {
	typedef struct Renderer {
		window::Window* window;
		bool running = true;
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		GLint shaderProgram;
	} Renderer;

	typedef enum RenderCommandType : u32 {
		RENDER_COMMAND_QUAD,
		RENDER_COMMAND_TEXTURED_QUAD,
		RENDER_COMMAND_CIRCLE,
		RENDER_COMMAND_TEXT,
	} RenderCommandType;

	typedef struct RenderCommand {
		RenderCommandType type;
		union {
			struct {
				v2<f32> p[2];
				v3<f32> fill;
				v3<f32> stroke;
			} quad;
			struct {
				v2<f32> p[2];
				// TODO
			} texturedQuad;
			struct {
				v2<f32> p;
				f32 r;
				v3<f32> fill;
				v3<f32> stroke;
			} circle;
			struct {
				v2<f32> p;
				TextAnchorPoint anchor;
				v3<f32> fill;
				v3<f32> stroke;
			} text;
		};
	} RenderCommand;

	typedef struct RenderCommandBuffer {
		v3<f32> currentStroke;
		v3<f32> currentFill;
		u64 position;
		u64 capacity;
		RenderCommand commands[];
	} RenderCommandBuffer;

	typedef struct Vertex {
		GLfloat e[3];
		GLfloat fill[3];
		GLfloat stroke[3];
		GLfloat s[2];
	} Vertex;

	char* shader_source_load(allocator::Arena* arena, const char* filepath, OUT i32* size) {
		FILE* shaderFile = fopen(filepath, "rb");
		if (!shaderFile) {
			log_fatal("could not find shader at %s", filepath);
			return NULL;
		}
		if (fseek(shaderFile, 0, SEEK_END)) {
			log_fatal("could not seek end of shader source");
			return NULL;
		}
		u64 fileSize = ftell(shaderFile);
		rewind(shaderFile);
		char* shaderSource = (char*)allocator::arena_allocate(arena, fileSize + 1);
		if (!shaderSource) {
			log_fatal("failed to allocate shader source; out of memory");
			return NULL;
		}
		fread(shaderSource, 1, fileSize, shaderFile);
		*size = (i32)fileSize;
		return shaderSource;
	}

	i32 shader_build_errors_check(GLuint shader, const char* filepath, bool program) {
		i32 exitCode = 0;
		const u64 errorMessageLength = 1024;
		char errorMessage[errorMessageLength];
		if (program) {
			glGetProgramiv(shader, GL_LINK_STATUS, &exitCode);
			if (!exitCode) {
				glGetProgramInfoLog(shader, errorMessageLength, NULL, errorMessage);
				log_fatal("shader compilation error:\n%s", errorMessage);
			}
		} else {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &exitCode);
			if (!exitCode) {
				glGetShaderInfoLog(shader, errorMessageLength, NULL, errorMessage);
				log_fatal("shader compilation error in (%s):\n%s", filepath, errorMessage);
			}
		}
		return !exitCode;
	}

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

		glGenVertexArrays(1, &renderer->vao);
		glGenBuffers(1, &renderer->vbo);
		glGenBuffers(1, &renderer->ebo);

		glBindVertexArray(renderer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);

		// NOTE(zgayford): VBO layout
		// vertex | faceColor | strokeColor | uvCoord
		// x y z | r g b | r g b | u v
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void*)(6*sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11*sizeof(GLfloat), (void*)(9*sizeof(GLfloat)));
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		char* vertShaderSource;
		i32 vertShaderSize;
		if ((vertShaderSource = shader_source_load(arena, VERT_SHADER_PATH, &vertShaderSize))) {
			log_debug("vertex shader found");
			log_debug("vertex_shader:\n%s", vertShaderSource);
			glShaderSource(vertShader, 1, &vertShaderSource, &vertShaderSize);
			glCompileShader(vertShader);
			if (shader_build_errors_check(vertShader, VERT_SHADER_PATH, false))
				return NULL;
		} else
			return NULL;

		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		char* fragShaderSource;
		i32 fragShaderSize;
		if ((fragShaderSource = shader_source_load(arena, FRAG_SHADER_PATH, &fragShaderSize))) {
			log_debug("fragment shader found");
			log_debug("fragment_shader:\n%s", fragShaderSource);
			glShaderSource(fragShader, 1, &fragShaderSource, &fragShaderSize);
			glCompileShader(fragShader);
			if (shader_build_errors_check(fragShader, FRAG_SHADER_PATH, false))
				return NULL;
		} else
			return NULL;

		renderer->shaderProgram = glCreateProgram();
		glAttachShader(renderer->shaderProgram, vertShader);
		glAttachShader(renderer->shaderProgram, fragShader);
		glLinkProgram(renderer->shaderProgram);
		if (shader_build_errors_check(renderer->shaderProgram, NULL, true))
			return NULL;

		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		return renderer;
	}

	void destroy(Renderer** renderer) {
		Renderer* r = *renderer;
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDeleteVertexArrays(1, &r->vao);
		glDeleteBuffers(1, &r->vbo);
		glDeleteBuffers(1, &r->ebo);
		*renderer = NULL;
	}

	void render(Renderer* renderer, RenderCommandBuffer* commandBuffer) {
		v2<i32> dimensions = window::dimensions_get(renderer->window);

		// NOTE(zgayford): hardcoded for now, I would eventually like
		// to have a locking system on the arena or separate the arena
		// into a single threaded and multithreaded version, then create
		// a function like allocator::arena_allocate_grow(arena, ...)
		// that grows the last allocation by a set amount. This would 
		// require keeping track of individual allocations (or maybe just 
		// the latests allocation) which may be helpful for debugging.
		const u64 vertexCountMax = 1024;
		u64 vertexCount = 0;
		Vertex vertices[vertexCountMax];

		const u64 indexCountMax = 2048;
		u64 indexCount = 0;
		GLuint indices[indexCountMax];

		RenderCommand* command;
		for (u64 i = 0; i < commandBuffer->position; i++) {
			command = &commandBuffer->commands[i];
			switch (command->type) {
				case RENDER_COMMAND_QUAD:{
					// log_debug("drawing quad");
					if (vertexCount + 4 >= vertexCountMax) goto render_out_of_memory_error;
					if (indexCount + 6 >= indexCountMax) goto render_out_of_memory_error;

					GLfloat x1 = command->quad.p[0].x;
					GLfloat y1 = command->quad.p[0].y;
					GLfloat x2 = command->quad.p[1].x;
					GLfloat y2 = command->quad.p[1].y;

					Vertex v = {0};
					v.e[0] = x1;
					v.e[1] = y1;
					v.e[2] = 0.0f;
					v.s[0] = 0.0f;
					v.s[1] = 0.0f;
					memcpy(v.fill, &command->quad.fill, sizeof(v3<f32>));
					memcpy(v.stroke, &command->quad.stroke, sizeof(v3<f32>));
					vertices[vertexCount + 0] = v;

					v.e[0] = x2;
					v.e[1] = y1;
					v.s[0] = 1.0f;
					v.s[1] = 0.0f;
					vertices[vertexCount + 1] = v;

					v.e[0] = x1;
					v.e[1] = y2;
					v.s[0] = 0.0f;
					v.s[1] = 1.0f;
					vertices[vertexCount + 2] = v;

					v.e[0] = x2;
					v.e[1] = y2;
					v.s[0] = 1.0f;
					v.s[1] = 1.0f;
					vertices[vertexCount + 3] = v;

					indices[indexCount + 0] = vertexCount + 0;
					indices[indexCount + 1] = vertexCount + 1;
					indices[indexCount + 2] = vertexCount + 2;
					indices[indexCount + 3] = vertexCount + 2;
					indices[indexCount + 4] = vertexCount + 1;
					indices[indexCount + 5] = vertexCount + 3;

					vertexCount += 4;
					indexCount += 6;
					break;
				}
				case RENDER_COMMAND_TEXTURED_QUAD:
					unimplemented();
					break;
				case RENDER_COMMAND_CIRCLE:
					unimplemented();
					break;
				case RENDER_COMMAND_TEXT:
					unimplemented();
					break;
			}
		}

		if (false) {
		render_out_of_memory_error:
			log_error("render ran out of memory; incomplete draw");
		}

		glViewport(0, 0, dimensions.width, dimensions.height);
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(renderer->shaderProgram);
		glBindVertexArray(renderer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
		glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(Vertex), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(GLuint), indices, GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(0);
		glUseProgram(0);

		window::buffers_swap(renderer->window);

		return;
	}

	RenderCommandBuffer* render_command_buffer_create(allocator::Arena* arena, u64 commandCapacity) {
		RenderCommandBuffer* buffer = (RenderCommandBuffer*)allocator::arena_allocate(arena,
				sizeof(RenderCommandBuffer) + sizeof(RenderCommand)*commandCapacity);
		buffer->position = 0;
		buffer->capacity = commandCapacity;
		buffer->currentFill   = { 1.0f, 1.0f, 1.0f };
		buffer->currentStroke = { 0.0f, 0.0f, 0.0f };
		return buffer;
	}

	// NOTE(zgayford): admittedly this function is poorly named, but it is only used internally
	RenderCommand* render_command_buffer_command_get(RenderCommandBuffer* commandBuffer) {
		if (commandBuffer->position + 1 >= commandBuffer->capacity) {
			log_error("render command buffer out of memory; command dropped");
			return NULL;
		}
		
		return &commandBuffer->commands[commandBuffer->position++];
	}

	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> p[2]) {
		RenderCommand* command = render_command_buffer_command_get(buffer);
		if (command) {
			command->type = RENDER_COMMAND_QUAD;
			memcpy(command->quad.p, p, 2*sizeof(v2<f32>));
			command->quad.fill = buffer->currentFill;
			command->quad.stroke = buffer->currentStroke;
		}
	}

	void render_command_buffer_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> topRight) {
		RenderCommand* command = render_command_buffer_command_get(buffer);
		if (command) {
			command->type = RENDER_COMMAND_QUAD;
			command->quad = {
				.p = { bottomLeft, topRight },
				.fill = buffer->currentFill,
				.stroke = buffer->currentStroke,
			};
		}
	}

	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> p[2]) {
		unimplemented();
	}

	void render_command_buffer_textured_quad_push(RenderCommandBuffer* buffer, v2<f32> bottomLeft, v2<f32> topRight) {
		unimplemented();
	}

	void render_command_buffer_circle_push(RenderCommandBuffer* buffer, v2<f32> center, f32 radius) {
		RenderCommand* command = render_command_buffer_command_get(buffer);
		if (command) {
			command->type = RENDER_COMMAND_CIRCLE;
			command->circle = {
				.p = center,
				.r = radius,
				.fill = buffer->currentFill,
				.stroke = buffer->currentStroke,
			};
		}
	}

	void render_command_buffer_fill_set(RenderCommandBuffer* buffer, v3<f32> fill) {
		buffer->currentFill = fill;
	}

	void render_command_buffer_stroke_set(RenderCommandBuffer* buffer, v3<f32> stroke) {
		buffer->currentStroke = stroke;
	}
}
