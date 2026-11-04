#version 460


layout(location = 0) in vec3 FragColor;
layout(location = 0) out vec4 outFragColor;
layout(location = 1) in vec2 textureCoord;

void main() {

   outFragColor = vec4(0.5, 0.3, 1, 1);

}
