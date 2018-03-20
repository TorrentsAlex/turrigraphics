#version 330

in vec2 fragUV;

uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D gOther;

out vec4 fragColor;

void main() {
	vec2 uv = fragUV;

  vec4 diff;
  vec4 norm;
  vec4 dept;
  vec4 otth;

	if (uv.x > 0.0 && uv.y > 0.0) {
		diff = texture(gAlbedo, uv);
	} else if (uv.x > 0.0 && uv.y < 0.0) {
		norm = texture(gNormal, uv);
	} else if (uv.x < 0.0 && uv.y > 0.0) {
		dept = texture(gDepth,  uv);
	} else {
		otth = texture(gOther,  uv);
	}
  fragColor = diff + norm + depth + otth;
}
/*
1 2
3 4

column
1 0
1 0

row
0 1
1 2

ofsetX
0.5 0.0
0.5 0.0

ofsetY
0.0 0.5
0.5 1.0
int column = index % 2; // This 2 is the number of rows
int row = index / 2;
atlasOffset.x = (float)column / 2.0f;
atlasOffset.y = (float)row / 2.0f;

*/
