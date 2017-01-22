#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2DShadow shadowMapTex;
uniform sampler2D linearShadowMapTex;
uniform sampler2D beckmannTex;

uniform sampler2D frontDepthTex;
uniform sampler2D backDepthTex;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform float lightRadius;

uniform bool useTransmittance;

// Average recommended value for specular term on skin
const float m = 0.3;

in Vertex {
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
	float depth;
	mat4 lightViewProj;
} IN;

out vec4 fragColor[2];

// Compute Schlick fresnel reflectance approximation:
float fresnel(vec3 halfVector, vec3 viewVector, float f0) {
    float base = 1.0 - dot(viewVector, halfVector);
    float exponential = pow(base, 5.0);
    return exponential + f0 * (1.0 - exponential);
}

// Compute Kelemen/Szirmay-Kalos specular with a beckmann texture:
float specularKSK(sampler2D beckmannTex, vec3 normal, vec3 lightVector, vec3 viewVector, float roughness) {
    vec3 halfVector = lightVector + viewVector;
    vec3 halfVectorN = normalize(halfVector);

    float NdotL = max(0.0, dot(normal, lightVector));
    float NdotH = max(0.0, dot(normal, halfVectorN));

    float ph = pow( texture(beckmannTex, vec2(NdotH, roughness)).r * 2.0, 10.0 );
    float f = fresnel(halfVectorN, viewVector, 0.028);
    float ksk = max(0.0, ph * f / dot(halfVector, halfVector));

    return NdotL * ksk;   
}

// SSS Transmittance Function:
vec3 transmittance(	float translucency,			// control the transmittance effect. Range: [0..1]
					float width,				// Width of the filter
					vec3 worldPosition,			// Position in world space
					vec3 worldNormal,			// Normal in world space
					vec3 light,					// Light vector: lightWorldPosition - worldPosition
					sampler2D shadowMapTex,		// Linear 0..1 shadow map
					mat4 lightViewProjection,	// Regular world to light space matrix
					float lightFarPlane			// Far plane distance used in the light projection matrix
					) {
	// Calculate the scale of the effect:
	float scale = 8.25 * (1.0 - translucency) / width;

	// Shrink the position inwards the surface to avoid artifacts:
	vec4 shrinkedPos = vec4(worldPosition - 0.005 * worldNormal, 1.0);

	// Calculate the thickness from the light point of view:
	vec4 shadowPosition = lightViewProjection * shrinkedPos;
	float d1 = texture(shadowMapTex, shadowPosition.xy / shadowPosition.w).r;	// 'd1' has a range of 0..1
	float d2 = shadowPosition.z;												// 'd2' has a range of 0..'lightFarPlane'
	d1 *= lightFarPlane;														// So we scale 'd1' accordingly:
	float d = scale * abs(d1 - d2);

	if (d < 0.5) {
		d = max(0.5, d);
	}

/*
	// --- two depth maps ---
	vec2 depthMapCoords = shadowPosition.xy / shadowPosition.w;
	//depthMapCoords   = depthMapCoords / 2.0 + 0.5;
	//depthMapCoords.y = 1.0 - depthMapCoords.y;

	float dist1 = texture(backDepthTex, depthMapCoords).r;
	float dist2 = texture(frontDepthTex, depthMapCoords).r;

	float diff = scale * abs(dist1 - dist2);

	d = diff;
	/// --- o ---
*/

	// With the thickness, calculate the color using the precalculated transmittance profile:
	float dd = -d * d;

	vec3 profile =  vec3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
					vec3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
					vec3(0.118, 0.198, 0.0)   * exp(dd / 0.187) +
					vec3(0.113, 0.007, 0.007) * exp(dd / 0.567) +
					vec3(0.358, 0.004, 0.0)   * exp(dd / 1.99) +
					vec3(0.078, 0.0,   0.0)   * exp(dd / 7.41);

    // Using the profile, approximate the transmitted lighting from the back of the object:
    return profile * clamp(0.0 + dot(light, -worldNormal), 0.0, 1.0);
}

void main(void) {
	// Initialize the output:
	vec4 colour = vec4(vec3(0.0), 1.0);

	// Build TBN Matrix for bump mapping:
	mat3 TBN = mat3(IN.tangent, IN.binormal, IN.normal);
	vec3 normal = normalize(TBN * (texture(bumpTex, IN.texCoord).rgb * 2.0 - 1.0));

	// Sample colour texture:
	vec4 albedo = texture(diffuseTex, IN.texCoord);
/*
	// --- gamma correction ---
	vec4 albedo = pow(texture(diffuseTex, IN.texCoord), vec4(2.0, 2.0, 2.0, 1.0));
*/

	// Calculate atttenuation:
	float dist = length(lightPos - IN.worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

	// Calculate some terms we will use later on:
	vec3 f1 = lightColour.rgb * atten;
	vec3 f2 = albedo.rgb * f1;

	// Calculate vectors for light shading:
	vec3 lightVector = normalize(lightPos - IN.worldPos);
	vec3 viewVector = normalize(cameraPos - IN.worldPos);

	// Calculate the diffuse and specular lighting:
	vec3 diffuse = vec3( clamp(dot(lightVector, normal), 0.0, 1.0) );
	float specular = specularKSK(beckmannTex, normal, lightVector, viewVector, m);

	// Calculate shadows:
	float shadow = 1.0;
	if (IN.shadowProj.w > 0.0) {
		shadow = textureProj(shadowMapTex, IN.shadowProj);
	}

	// Add the diffuse and specular components:
	//colour.rgb += shadow * (f2 * diffuse + f1 * specular);	// shadows are not working for the head mesh
	colour.rgb += (f2 * diffuse + f1 * specular);

	// Add the transmittance component:
	if (useTransmittance) {
		colour.rgb += f2 * transmittance(0.7, 0.03, IN.worldPos, IN.normal, lightVector, linearShadowMapTex, IN.lightViewProj, 10.0);
	}

	// Store the final colour:
	fragColor[0] = colour;

/*
	// --- gamma correction ---
	//fragColor[0] = vec4(pow(colour.rgb, vec3(1.0 / 2.2)), 1.0);
*/

	// Store the depth value:
	fragColor[1] = vec4(vec3(IN.depth), 1.0);
}