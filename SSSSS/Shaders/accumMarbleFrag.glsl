#version 150 core

uniform sampler2D diffuseTex;

uniform sampler2D blurredTex1;
uniform sampler2D blurredTex2;
uniform sampler2D blurredTex3;
uniform sampler2D blurredTex4;
uniform sampler2D blurredTex5;

uniform bool useSSS;

// MARBLE
const vec3 gaussWeights1 = vec3(0.2, 0.2, 0.2); 
const vec3 gaussWeights2 = vec3(0.0544578254963, 0.12454890956, 0.217724878147);
const vec3 gaussWeights3 = vec3(0.243663230592, 0.243532369381, 0.18904245481);
const vec3 gaussWeights4 = vec3(0.310530428621, 0.315816663292, 0.374244725886);
const vec3 gaussWeights5 = vec3(0.391348515291, 0.316102057768, 0.218987941157);

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
		vec4 irrad5tap = texture(blurredTex5, IN.texCoord);
	
		diffuseLight += gaussWeights1 * irrad1tap.xyz;
		diffuseLight += gaussWeights2 * irrad2tap.xyz;
		diffuseLight += gaussWeights3 * irrad3tap.xyz;
		diffuseLight += gaussWeights4 * irrad4tap.xyz;
		diffuseLight += gaussWeights5 * irrad5tap.xyz;

		// Renormalize diffusion profiles to white
		//vec3 normConst = gaussWeights1 + gaussWeights2 + gaussWeights3 + gaussWeights4 + gaussWeights5;
		//diffuseLight /= normConst;

		fragColor = vec4(diffuseLight, 1.0);
	}
}