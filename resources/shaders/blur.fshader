#version 330

const float offset[3] = float[](0.0, 0.00063846153846, 0.0052307692308);
const float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

in vec2 fragUV;

uniform sampler2D gLuminance;

uniform int blurType;

out vec4 finalColor;

vec4 horitzontalBlur() {
  vec4 color = vec4(0.0f);
  for(int i = 0; i < 3; i++) {
    color += texture(gLuminance, fragUV + vec2(offset[i], 0.0)) * weight[i];
    color += texture(gLuminance, fragUV - vec2(offset[i], 0.0)) * weight[i];
  }
  return color;
}

vec4 verticalBlur() {
  vec4 color = vec4(0.0f);
  for(int i = 0; i < 3; i++) {
    color += texture(gLuminance, fragUV + vec2(0.0, offset[i])) * weight[i];
    color += texture(gLuminance, fragUV - vec2(0.0, offset[i])) * weight[i];
  }
  return color;
}

void main() {
  finalColor = texture(gLuminance, fragUV);

   if (blurType == 0) {
    finalColor = horitzontalBlur();
   } else {
     finalColor = verticalBlur();
   }
}
