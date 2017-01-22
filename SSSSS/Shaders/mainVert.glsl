#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix;
uniform mat4 shadowMatrix2;
uniform mat4 lightView;
uniform mat4 lightView2;

uniform float zNear;
uniform float zFar;

in vec3 position;
in vec4 colour;
in vec2 texCoord;
in vec3 normal;
in vec3 tangent;

out Vertex {
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
	float depth;
	mat4 lightViewProj;
} OUT;

void main(void) {
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;

	OUT.texCoord = texCoord;
	OUT.normal = normalize(normalMatrix * normalize(normal));
	OUT.tangent = normalize(normalMatrix * normalize(tangent));
	OUT.binormal = normalize(normalMatrix * normalize(cross(normal, tangent)));
	OUT.worldPos = (modelMatrix * vec4(position, 1.0)).xyz;
	OUT.shadowProj = shadowMatrix * vec4(position + (normal * 1.5), 1.0);

	// linear depth:
	vec4 viewPos = (viewMatrix * modelMatrix) * vec4(position, 1.0);
	OUT.depth = (-viewPos.z - zNear) / (zFar - zNear);

	// light view projection matrix:
	OUT.lightViewProj = lightView;

	gl_Position = mvp * vec4(position, 1.0);
}