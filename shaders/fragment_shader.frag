#version 460


layout(location = 0) in vec3 FragColor;
vec4 outFragColor;

void main() {

   outFragColor = vec4(FragColor, 1.0f);

}
