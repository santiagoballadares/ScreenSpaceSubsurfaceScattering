#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;

void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;

	gl_Position = mvp * vec4(position, 1.0);
}