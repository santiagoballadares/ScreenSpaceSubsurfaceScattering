#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform float zNear;
uniform float zFar;

in vec3 position;
out float depth;

void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	
	// linear depth
	vec4 viewPos = (viewMatrix * modelMatrix) * vec4(position, 1.0);
	depth = (-viewPos.z - zNear) / (zFar - zNear);

	gl_Position = mvp * vec4(position, 1.0);
}