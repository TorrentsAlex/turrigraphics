#version 330

out vec4 depth;

const float near = 0.1;
const float far = 2001.0;

float linearDepth(float depth) {
	float z = depth * 2.0 -1.0;
	return (2.0 * near * far) / (far + near - z *(far - near));
}

void main() {
    gl_FragDepth = gl_FragCoord.z;
   // depth = vec4(vec3(linearDepth(gl_FragCoord.z)), 1.0);
    depth = vec4(vec3(gl_FragCoord.z), 1.0);
}