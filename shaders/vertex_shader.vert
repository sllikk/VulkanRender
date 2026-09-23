#version 460

layout(location = 0) out vec3 FragColor;

vec2 positions[3] = vec2[](
    vec2(0.0f, -0.5f),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);


void main() {

    gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
    FragColor = vec3(1.0f, 1.0f, 1.0f);
}
