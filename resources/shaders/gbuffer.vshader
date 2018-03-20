#version 330

in vec3 vertexPosition;
in vec2 vertexUV;
in vec3 vertexNormal;

out vec2 fragUV;
out vec3 fragNormal;
out vec3 fragPosition;

	//MVP matrices
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

	//normal model matrix
uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;

uniform vec2 textureScaleFactor;

void main() {
		//Move the vertex from the local coordinates to the projection coordinates thanks to the transformation matrices
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);


	// Outputs for fragment
	fragUV 		 = vec2(vertexUV.x, 1.0-vertexUV.y) * textureScaleFactor;
	fragNormal   = modelNormalMatrix * vertexNormal;
	fragPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
}
