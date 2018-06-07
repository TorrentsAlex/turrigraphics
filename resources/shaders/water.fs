#version 330

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPosition;
in vec3 tangent;
in vec3 bitangent;
in vec4 clipSpace;

uniform sampler2D textureReflection;
uniform sampler2D textureRefraction;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;

uniform vec3 viewerPosition;
uniform vec3 lightPosition;
uniform vec3 lightColor;

uniform float moveFactor;
uniform float fresnelFactor;
uniform float waveStrenght;
uniform float shineDamper;
uniform float reflectivity;

out vec4 color;

void main() {

    vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;

    vec2 distortion1 = waveStrenght * (texture(dudvMap, vec2(fragUV.x + moveFactor/2.0, fragUV.y)).rg * 2.0 - 1.0);
    vec2 distortion2 = waveStrenght * (texture(dudvMap, vec2(fragUV.x, fragUV.y + moveFactor/2.0)).rg * 2.0 - 1.0);
    vec2 totalDistorion = distortion1 + distortion2;

    vec2 refractCoords = vec2(ndc.x, ndc.y) + totalDistorion;
    vec2 reflectCoords = vec2(ndc.x, -ndc.y) + totalDistorion;

    refractCoords = clamp(refractCoords, 0.001, 0.999);
    reflectCoords.x = clamp(reflectCoords.x, 0.001, 0.999);
    reflectCoords.y = clamp(reflectCoords.y, -0.999, -0.001);

    vec4 refr = texture(textureRefraction, refractCoords);
    vec4 refl = texture(textureReflection, reflectCoords);

    // Fresnel
    vec3 toCameraVector = normalize(viewerPosition - fragPosition.xyz);
    float refractiveFactor = dot(toCameraVector, vec3(0.0, 1.0, 0.0));
    refractiveFactor = pow(refractiveFactor, fresnelFactor);
    // Normal map
    vec3 normal = texture(normalMap, totalDistorion).xyz;
    normal = vec3(normal.r * 2.0 - 1.0, normal.b, normal.g *2.0 -1.0); 
    normal = normalize(normal);

    vec3 fromLightVector = normalize(fragPosition.xyz - lightPosition);

	vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
	float specular = max(dot(reflectedLight, toCameraVector), 0.0);
	specular = pow(specular, shineDamper);
	vec3 specularHighlights = lightColor * specular * reflectivity;

    color = mix(refl, refr, refractiveFactor);
    color = mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(specularHighlights, 0.0);
}
