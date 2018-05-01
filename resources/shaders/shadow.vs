#version 330

in vec3 vertPosition;
in vec2 vertUV;
in vec3 vertNormal;

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

void main() {
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(vertPosition, 1.0);
}