#version 150 core

uniform sampler2D diffuseTex;

uniform sampler2D blurredTex1;
uniform sampler2D blurredTex2;
uniform sampler2D blurredTex3;
uniform sampler2D blurredTex4;

uniform bool useSSS;

// SKIN
const vec3 gaussWeights1 = vec3(0.240516183695, 0.447403391891, 0.615796108321); 
const vec3 gaussWeights2 = vec3(0.115857499765, 0.366176401412, 0.343917471552);
const vec3 gaussWeights3 = vec3(0.183619017698, 0.186420206697, 0.0);
const vec3 gaussWeights4 = vec3(0.460007298842, 0.0, 0.0402864201267);


in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void) {
	if (!useSSS) {
		vec4 diffuse = texture(diffuseTex, IN.texCoord);
		fragColor = diffuse;
	}
	else {
		// Total diffuse
		vec3 diffuseLight = vec3(0.0);
	
		vec4 irrad1tap = texture(blurredTex1, IN.texCoord);
		vec4 irrad2tap = texture(blurredTex2, IN.texCoord);
		vec4 irrad3tap = texture(blurredTex3, IN.texCoord);
		vec4 irrad4tap = texture(blurredTex4, IN.texCoord);
	
		diffuseLight += gaussWeights1 * irrad1tap.xyz;
		diffuseLight += gaussWeights2 * irrad2tap.xyz;
		diffuseLight += gaussWeights3 * irrad3tap.xyz;
		diffuseLight += gaussWeights4 * irrad4tap.xyz;

		// Renormalize diffusion profiles to white
		//vec3 normConst = gaussWeights1 + gaussWeights2 + gaussWeights3 + gaussWeights4;
		//diffuseLight /= normConst;

		fragColor = vec4(diffuseLight, 1.0);
	}
}