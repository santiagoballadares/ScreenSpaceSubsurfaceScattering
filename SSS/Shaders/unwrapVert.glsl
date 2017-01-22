#version 150 core

uniform mat4 modelMatrix;
uniform mat4 shadowMatrix;

in vec3 position;
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
} OUT;

// convert texture coordinates to NDC [-1, 1]
vec4 v2t(vec2 texCoord) {
	return vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, 0.0, 1.0);
}

void main(void) {
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	OUT.texCoord = texCoord;
	OUT.normal = normalize(normalMatrix * normalize(normal));
	OUT.tangent = normalize(normalMatrix * normalize(tangent));
	OUT.binormal = normalize(normalMatrix * normalize(cross(normal, tangent)));
	OUT.worldPos = (modelMatrix * vec4(position, 1.0)).xyz;
	OUT.shadowProj = shadowMatrix * vec4(position + (normal * 1.5), 1.0);

	gl_Position	= v2t(texCoord);
}
