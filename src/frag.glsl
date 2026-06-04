#version 410 core

in vec3 vFillColor;
in vec3 vStrokeColor;
in vec2 vTexCoord;

out vec4 fragColor;


bool float_compare(float a, float b) {
	const float EPSILON = 1e-3;
	return abs(a - b) < EPSILON;
}

void main() {
	float s = vTexCoord.s;
	float t = vTexCoord.t;
	const float strokeWidth = 0.01;

	if (
			s <= strokeWidth || 
			s >= 1 - strokeWidth ||
			t <= strokeWidth ||
			t >= 1 - strokeWidth)
		fragColor = vec4(vStrokeColor, 1.0f);
	else
		fragColor = vec4(vFillColor, 1.0f);
}
