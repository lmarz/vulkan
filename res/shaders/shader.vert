#version 460

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(binding=0) uniform UBO {
    mat4 projection;
    mat4 view;
    mat4 model;  
};

layout(location=0) out vec2 UV;

void main() {
    UV = uv;
    gl_Position = projection * view * model * vec4(pos, 1.0f);
}