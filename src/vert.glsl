#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 fillColor;
layout(location = 2) in vec3 strokeColor;
layout(location = 3) in vec2 texCoord;

out vec3 vFillColor;
out vec3 vStrokeColor;
out vec2 vTexCoord;

void main() {
	gl_Position  = vec4(pos, 1.0f);
	vFillColor   = fillColor;
	vStrokeColor = strokeColor;
	vTexCoord   = texCoord;
}
