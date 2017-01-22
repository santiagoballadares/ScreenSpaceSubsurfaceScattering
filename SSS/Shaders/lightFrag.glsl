#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform float lightRadius;

in Vertex {
	vec4 colour;	
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

void main(void) {
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	
	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	vec3 normal = normalize(TBN * (texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0));

	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float dist = length(lightPos - IN.worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

	float lambert = max(0.0, dot(incident, normal));
	float rFactor = max(0.0, dot(halfDir, normal));
	float sFactor = pow(rFactor, 50.0);
	
	vec3 colour = (diffuse.rgb * lightColour.rgb);
	colour += (lightColour.rgb * sFactor) * 0.33;
	
	fragColor = vec4(colour * atten * lambert, diffuse.a);
	fragColor.rgb += (diffuse.rgb * lightColour.rgb) * 0.1;
}