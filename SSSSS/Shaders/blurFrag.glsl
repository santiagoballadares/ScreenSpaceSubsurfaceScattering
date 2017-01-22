#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D depthTex;

uniform vec2 pixelSize;
uniform vec2 dir;
uniform float gaussianWidth;
uniform float correction;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void) {
	// Gaussian weights for the six samples around the current pixel:
	// float w[7] = float[](0.006, 0.061, 0.242, 0.382, 0.242, 0.061, 0.006);	// -3 -2 -1 0 +1 +2 +3
	// -3 -2 -1 +1 +2 +3
	float w[6] = float[](  0.006,  0.0610,  0.2420, 0.2420, 0.0610, 0.006 );
	float o[6] = float[]( -1.000, -0.6667, -0.3333, 0.3333, 0.6667, 1.000 );

	// Fetch color and linear depth for current pixel:
	vec4 colourM = texture(diffuseTex, IN.texCoord);
	float depthM = texture(depthTex, IN.texCoord).r;

	// Accumulate center sample, multiplying it with its gaussian weight:
	vec4 colourBlurred = colourM;
	colourBlurred.rgb *= 0.382;

	// Calculate: step = sssStrength * gaussianWidth * pixelSize * dir
	vec2 step = gaussianWidth * pixelSize * dir;
	vec2 finalStep = colourM.a * step / depthM;

	// Accumulate the other samples:
	for (int i = 0; i < 6; ++i) {
		// Fetch color and depth for current sample:
		vec2 offset = IN.texCoord + o[i] * finalStep;
		vec3 colour = texture(diffuseTex, offset).rgb;
		float depth = texture(depthTex, offset).r;

		// If the difference in depth is huge, lerp color back to "colorM":
		float s = min(0.0125 * correction * abs(depthM - depth), 1.0);
		colour = mix(colour, colourM.rgb, s);

		// Accumulate:
		colourBlurred.rgb += w[i] * colour;
	}

	fragColor = colourBlurred;
}