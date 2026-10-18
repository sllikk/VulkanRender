#version 460


layout(location = 0) in vec3 FragColor;
layout(location = 0) out vec4 outFragColor;
layout(location = 1) in vec2 textureCoord;
layout(set = 0, binding = 1) uniform sampler2D Texture;

void main() {

   outFragColor = texture(Texture, textureCoord);

}
