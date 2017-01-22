#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform float depth;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec2 texCoord;
} OUT;

void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	vec4 pos = mvp * vec4(position, 1.0);

	OUT.texCoord = texCoord;

	gl_Position = pos;
}