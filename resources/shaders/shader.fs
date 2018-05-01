#version 330

in vec2 fragUV;

uniform sampler2D gColor;
uniform sampler2D gBloom;
uniform sampler2D debugTexture;
uniform sampler2D shadowMap;

uniform int pixelation;
uniform int nightVision;
uniform int debugMode;

out vec4 fragColor;

#define SHOULDER_STRENGTH 0.22
#define LINEAR_STRENGTH 0.30
#define LINEAR_ANGLE 0.10
#define TOE_STRENGTH 0.20
#define TOE_NUMERATOR 0.01
#define TOE_DENOMINATOR 0.30
#define LINEAR_WHITE_POINT_VALUE 11.2

vec4 filmic_tonemapping(vec4 x)
{
    return ((x*(SHOULDER_STRENGTH*x + LINEAR_ANGLE * LINEAR_STRENGTH) + TOE_STRENGTH * TOE_NUMERATOR) /
            (x*(SHOULDER_STRENGTH*x + LINEAR_STRENGTH) + TOE_STRENGTH * TOE_DENOMINATOR)) - TOE_NUMERATOR/TOE_DENOMINATOR;
}

vec3 dSphereMap(vec2 n) {
	vec4 t = vec4(n, 0.0, 0.0) * vec4(2.0, 2.0, 0.0, 0.0) + vec4(-1.0, -1.0, 1.0, -1.0);
	t.z = dot(t.xyz, -t.xyw);
	t.xy *= sqrt(t.z);
	return t.xyz * vec3(2.0) + vec3(0.0, 0.0, -1.0);
}

void main() {
	vec2 uv = fragUV;
	if (pixelation == 1) {
		uv = floor(fragUV * vec2(100.0)) / vec2(100.0);
	}

	fragColor = texture(gColor, uv);

	//fragColor = filmic_tonemapping(fragColor) / filmic_tonemapping(vec4(LINEAR_WHITE_POINT_VALUE));
    //fragColor.rgb = pow(fragColor.rgb, vec3(1.0/2.2));

	if (debugMode != 0) {
		fragColor = texture(debugTexture, uv);
		if (debugMode == 3) {
			fragColor = vec4(dSphereMap(texture(debugTexture, uv).xy), 1.0);
		}
		if (debugMode == 4) {
			fragColor = vec4(vec3(texture(debugTexture, uv).z), 1.0);
		}
	}
	//fragColor = vec4(vec3(texture(shadowMap, uv).r), 1.0);
}
