#version 330

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPosition;

uniform sampler2D textureData;
uniform sampler2D materialMap;
uniform int haveMaterialMap;

uniform vec2 ab;
uniform vec3 viewerPosition;
uniform mat4 projectionMatrix;

out vec4 gDiffuse;
out vec4 gNormal;
out vec4 gPosition;
out vec4 gMaterial;

float near = 0.1;
float far = 201.0;

vec2 sphereMap(vec3 nor) {
	return normalize(nor.xy) * sqrt(-nor.z*0.5 + 0.5) * 0.5 + 0.5;
}

float linearDepth(float depth) {
	float z = depth * 2.0 -1.0;
	return (2.0 * near * far) / (far + near - z *(far - near));
}

void main() {

	float depth = linearDepth(gl_FragCoord.z)/far;
	
	gDiffuse = vec4(texture(textureData, fragUV).rgb, 1.0);
	gNormal = vec4(sphereMap(fragNormal), depth, 1.0);
	gPosition = vec4(fragPosition, 1.0);

	if (haveMaterialMap == 1) {
		gMaterial = vec4(texture(materialMap, fragUV).rgb, 1.0);
	} else {
		gMaterial = vec4(vec3(0.0), 1.0);
	}
}
