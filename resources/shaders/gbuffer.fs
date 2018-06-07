#version 330

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPosition;
in vec3 tangent;
in vec3 bitangent;

uniform sampler2D textureData;
uniform sampler2D materialMap;
uniform sampler2D normalMap;
uniform sampler2D roughness;

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
	// Calculate normal with fragment normal and normal normalMap
	vec3 normalTexture = normalize(texture(normalMap, fragUV).rgb * 2.0 - 1.0);
	vec3 atangent = normalize(tangent);
	vec3 abitangent = normalize(bitangent);
	mat3 tangentToWorld = mat3(atangent.x, abitangent.x, fragNormal.x,
							   atangent.y, abitangent.y, fragNormal.y,
							   atangent.z, abitangent.z, fragNormal.z);
	vec3 normal = normalTexture * tangentToWorld;

	gDiffuse = vec4(texture(textureData, fragUV).rgb, 1.0);
	gNormal = vec4(sphereMap(normal), depth, 1.0);
	gPosition = vec4(fragPosition, 1.0);
    
    float roug = texture(roughness, fragUV).r;
	float metal = texture(materialMap, fragUV).r;
    
    gMaterial = vec4(roug, metal, 1.0, 1.0);

}
