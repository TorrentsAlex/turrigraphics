#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexUV;
in vec3 vertexTangent;
in vec3 vertexBitangent;

out vec2 fragUV;
out vec3 fragNormal;
out vec3 fragPosition;
out vec3 tangent;
out vec3 bitangent;

out vec4 clipSpace;

	//MVP matrices
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform float moveFactor;
	//normal model matrix
uniform mat4 modelMatrix;
uniform mat3 modelMatrix3x3;
uniform mat3 modelNormalMatrix;

uniform vec2 textureScaleFactor;

const vec4 plane = vec4(0, -1, 0, 0);

void main() {
		//Move the vertex from the local coordinates to the projection coordinates thanks to the transformation matrices
	clipSpace = projectionMatrix * viewMatrix * modelMatrix * vec4( vertexPosition, 1.0);

	// Outputs for fragment
	fragUV 		 = vec2(vertexUV.x, 1.0-vertexUV.y) * textureScaleFactor;
	fragNormal   = modelNormalMatrix * vertexNormal;
	fragPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
	
	tangent = vertexTangent;
	bitangent = vertexBitangent;

	gl_Position = clipSpace;
}
