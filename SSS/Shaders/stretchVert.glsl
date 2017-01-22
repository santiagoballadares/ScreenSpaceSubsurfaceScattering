#version 150 core

uniform mat4 modelMatrix;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec3 worldPos;
} OUT;

// convert texture coordinates to NDC [-1, 1]
vec4 v2t(vec2 texCoord) {
	return vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, 0.0, 1.0);
}

void main(void) {
	OUT.worldPos = (modelMatrix * vec4(position, 1.0)).xyz;
	
	gl_Position = v2t(texCoord);
}