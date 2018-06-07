#version 330

in vec3 vertexPosition;
in vec2 vertexUV;
in vec3 vertexNormal;

out vec2 fragUV;

void main() {
	gl_Position = vec4(vertexPosition, 1.0);

	fragUV = vertexUV;
}
