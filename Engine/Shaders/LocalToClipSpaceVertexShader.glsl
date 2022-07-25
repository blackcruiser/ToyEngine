#version 450

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform UniformBuffer
{ 
    mat4 mvp;
} ub;

void main() {
    gl_Position = ub.mvp * vec4(position, 1.0) ;
    fragColor = vec3(1.0, 0.0, 1.0);
}