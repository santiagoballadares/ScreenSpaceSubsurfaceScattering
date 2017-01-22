#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2DShadow shadowTex;

uniform vec3 lightPos;
uniform vec4 lightColour;
uniform float lightRadius;

uniform vec3 cameraPos;

float mix = 0.5;

in Vertex {
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
} IN;

out vec4 fragColor;

float fresnelReflectance(vec3 H, vec3 V, float F0) {
	float base = 1.0 - dot(V, H);
	float exponential = pow(base, 5.0);  
	return exponential + F0 * (1.0 - exponential);
}

void main(void) {
	vec4 diffuse = texture(diffuseTex, IN.texCoord);

	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	vec3 normal = normalize(TBN * (texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0));

	vec3 incident = normalize(lightPos - IN.worldPos);
	float lambert = max(0.0, dot(incident, normal));
	
	float shadow = 1.0;
	if(IN.shadowProj.w > 0.0) {
		shadow = textureProj(shadowTex, IN.shadowProj);
	}
	lambert *= shadow;

	//--
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);
	
	float fresnel = fresnelReflectance(halfDir, viewDir, 0.028);
	//--

	//fragColor = vec4(vec3(lambert), 1.0);
	//fragColor = vec4(lambert * diffuse.rgb, 1.0);
	fragColor = vec4(lambert * pow(diffuse.rgb, vec3(mix)), 1.0);
}
