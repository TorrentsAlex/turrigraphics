#version 330

const int maxlights = 2;
const float PI = 3.1415926536;

struct Light {
	int type; // 0 : Directional,  1 : Pointlight
	vec3 amb;
	vec3 pos;// Or direction in case of directional
};

in vec2 fragUV;

uniform vec3 viewerPosition;

uniform sampler2D gDiff;
uniform sampler2D gNorm;
uniform sampler2D gPos;
uniform sampler2D gSpec;
uniform sampler2D skybox;

uniform samplerCube cubemap;

// 0 color, 1 cubemap
uniform int cubemapActive;

uniform Light lights[maxlights];

out vec4 lightColor;
out vec4 luminance;

vec3 dSphereMap(vec2 n) {
	vec4 t = vec4(n, 0.0, 0.0) * vec4(2.0, 2.0, 0.0, 0.0) + vec4(-1.0, -1.0, 1.0, -1.0);
	t.z = dot(t.xyz, -t.xyw);
	t.xy *= sqrt(t.z);
	return t.xyz * vec3(2.0) + vec3(0.0, 0.0, -1.0);
}

vec4 loadCubemap(vec3 N, vec3 V) {
	vec3 dir = reflect(N, V);
	vec4 color = texture(cubemap, dir);
	color = pow(color, vec4(1.45));
	return color;
}

/* Schlick */
vec3 fresnel(float VdotH, vec3 c) {
	return c + (1.0 - c) * pow(1.0 - VdotH, 5.0);
}

float dBeckmann(float NdotH, float roughness) {
	float a2 = pow(roughness, 4);
	float nd = NdotH * NdotH;

	float d = nd * (a2 -1.0) + 1.0;
	d = PI * d * d;
	return a2 / d;
}

float gCookTorrance(float NdotV, float VdotH, float NdotH) {
	float v1 = 2.0 * NdotH * NdotV / VdotH;
	return min(1, v1);
}

vec4 calcColor() {
	vec4 color = vec4(0.0);

	// load textures
	vec3 normalT = dSphereMap(texture(gNorm, fragUV).rg);
	float depth = texture(gNorm, fragUV).b;
	//r: specular, g: roughness, b: metallic
	vec3 material = texture(gSpec, fragUV).rgb;
	vec3 worldPos = texture(gPos, fragUV).rgb;

	vec3 V = normalize(viewerPosition - worldPos);
	vec3 N = normalize(normalT);

	vec4 albedo = texture(gDiff, fragUV);
	float NdotV = max(0.0, dot(N, V));

	// light
	for (int i = 0; i < maxlights; i++) {
		Light l = lights[i];

		vec3 L = l.type == 0 ? normalize(-l.pos) : normalize(l.pos - worldPos);
		vec3 H = normalize(L + V);

		float VdotH = max(0.0, dot(V, H));
		float NdotH = max(0.0, dot(N, H));
		float NdotL = max(0.0, dot(N, L));

		vec3 diffuse = albedo.rgb * clamp(NdotL, 0.0, 1.0) * (1.0 - material.b);

		// ambient color + PBR
		vec3 nom = fresnel(VdotH, vec3(1.0)) *
							 dBeckmann(NdotH, material.g) *
							 gCookTorrance(NdotV, VdotH, NdotH);

		vec3 specular = ((1.0 - material.b) + material.b * albedo.rgb) * (nom / 4.0 * NdotV * NdotL);

		float dist = length(l.pos - worldPos);

		// Don't have attenuation if it's directional
		float attenuation = (l.type == 0) ?  1.0 : (5000.0 / (dist * dist));

		if (material.r > 0.7) {
			diffuse = vec3(0.1);
		}
		color.rgb += l.amb * (diffuse+specular) * attenuation;
	}
	//color.rgb /= maxlights;

	// Uncomment if you wanna see the cubemap implemented
	//color = loadCubemap(N, V);
	return vec4(color.rgb , 1.0);
}

void main() {

	lightColor = calcColor();

	if (dot(lightColor.rgb, vec3(0.2126, 0.7152, 0.0722)) > 0.8) {
		luminance = vec4(lightColor.rgb, 1.0);
	}
	// add skybox
	float depth = texture(gNorm, fragUV).b;
	if (depth == 0.0) {
		lightColor = texture(skybox, fragUV);
	}
}
