#version 150 core

float scale = 1.0;

in Vertex {
	vec3 worldPos;
} IN;

out vec4 fragColor;

vec2 computeStretchMap(vec3 worldPos, float scale) {      
    vec3 derivu = dFdx(worldPos);
    vec3 derivv = dFdy(worldPos);
	
    float stretchU = scale / length( derivu );
    float stretchV = scale / length( derivv );
	
    return vec2( stretchU, stretchV ); // two component texture color
}

void main(void) {
	vec2 outColour = computeStretchMap(IN.worldPos, scale);
	fragColor = vec4(outColour.xy, 0.0, 1.0);
}