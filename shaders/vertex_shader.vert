#version 460

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TextureCoord;
layout(location = 1) out vec2 outTextureCoord;

layout(location = 0) out vec3 FragColor;

layout(push_constant) uniform constant {
    mat4 worldTransform;
    mat4 worldTextureTransform;
} ObjectPushConstant;

layout(set = 0, binding = 0, std140) uniform ubo {

    mat4 proj;
    mat4 view;
    mat4 projView;

} MainPassUBO;

void main() {

    gl_Position =  MainPassUBO.projView  * ObjectPushConstant.worldTransform  * vec4(Position, 1.0f);
    outTextureCoord = TextureCoord;
    FragColor = vec3(Position);
}
