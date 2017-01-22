#version 150 core

in float depth;
out vec4 fragColor;

void main(void) {
	fragColor = vec4(vec3(depth), 1.0);
}