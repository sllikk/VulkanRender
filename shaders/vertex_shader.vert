#version 460

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextureCoord;
layout(location = 1) out vec2 outTextureCoord;

layout(location = 0) out vec3 FragColor;

void main() {

    gl_Position =  vec4(Position, 1.0f);
    FragColor = vec3(Position);
}
