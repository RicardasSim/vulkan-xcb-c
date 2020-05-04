#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec3 col;

layout(binding = 0) uniform UniformBufferObject {
mat4 model;
mat4 view;
mat4 proj;
} ubo;

void main() {

    gl_Position = ubo.proj * ubo.view * ubo.model * pos;

    fragColor = col;
}
