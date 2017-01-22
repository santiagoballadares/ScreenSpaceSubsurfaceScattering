#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix;

in vec3 position;
in vec4 colour;
in vec2 texCoord;
in vec3 normal;
in vec3 tangent;

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
} OUT;

void main(void) {
	mat4 mvp = (projMatrix * viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	OUT.colour	   = colour;
	OUT.texCoord   = texCoord;
	OUT.normal     = normalize(normalMatrix * normalize(normal));
	OUT.tangent    = normalize(normalMatrix * normalize(tangent));
	OUT.binormal   = normalize(normalMatrix * normalize(cross(normal, tangent)));
	OUT.worldPos   = (modelMatrix * vec4(position, 1.0)).xyz;
	OUT.shadowProj = shadowMatrix * vec4(position + (normal * 1.5), 1.0);

	gl_Position    = mvp * vec4(position, 1.0);
}