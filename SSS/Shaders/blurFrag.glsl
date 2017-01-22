#version 150 core

uniform sampler2D diffuseTex;
uniform sampler2D stretchTex;
uniform vec2 pixelSize;
uniform bool xAxis;
uniform bool useStretch;

// GaussWidth should be the standard deviation.  
float GaussWidth = 1.0f;
// Gaussian curve - standard deviation of 1.0
const float weights[7] = float[](0.006, 0.061, 0.242, 0.382, 0.242, 0.061, 0.006);

in Vertex {
	vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void) {
	if (useStretch) {

		vec4 stretch = texture(stretchTex, IN.texCoord);
		vec4 sum = vec4(0.0);

		if (xAxis) {
			float netFilterWidth = pixelSize.x * GaussWidth * stretch.x;
			vec2 coords = IN.texCoord - vec2(netFilterWidth * 3.0, 0.0);

			for (int i = 0; i < 7; ++i) {
				vec4 tap = texture(diffuseTex, coords);
				sum += weights[i] * tap;
				coords += vec2(netFilterWidth, 0.0);
			}
		}
		else {
			float netFilterWidth = pixelSize.y * GaussWidth * stretch.y;
			vec2 coords = IN.texCoord - vec2(0.0, netFilterWidth * 3.0);

			for (int i = 0; i < 7; ++i) {
				vec4 tap = texture(diffuseTex, coords);
				sum += weights[i] * tap;
				coords += vec2(0.0, netFilterWidth);
			}
		}

		fragColor = sum;
		
	}
	else {

		vec2 values[7];
		if (xAxis) {
			values = vec2[](vec2(-pixelSize.x * 3.0, 0.0),
							vec2(-pixelSize.x * 2.0, 0.0),
							vec2(-pixelSize.x * 1.0, 0.0),
							vec2( 				0.0, 0.0),
							vec2( pixelSize.x * 1.0, 0.0),
							vec2( pixelSize.x * 2.0, 0.0),
							vec2( pixelSize.x * 3.0, 0.0));
		}
		else {
			values = vec2[](vec2(0.0, -pixelSize.y * 3.0),
							vec2(0.0, -pixelSize.y * 2.0),
							vec2(0.0, -pixelSize.y * 1.0),
							vec2(0.0, 				 0.0),
							vec2(0.0,  pixelSize.y * 1.0),
							vec2(0.0,  pixelSize.y * 2.0),
							vec2(0.0,  pixelSize.y * 3.0));
		}

		for (int i = 0; i < 7; ++i) {
			vec4 tmp = texture(diffuseTex, IN.texCoord + values[i]);
			fragColor += tmp * weights[i];
		}

	}
}
