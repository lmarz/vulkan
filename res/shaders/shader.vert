#version 460

layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;

layout(binding=0) uniform UBO {
    mat4 projection;
    mat4 view;
    mat4 model;
    vec3 lightPos;
};

layout(location=0) out vec3 NORMAL;
layout(location=1) out vec2 UV;
layout(location=2) out vec3 lightVec;

void main() {
    NORMAL = mat3(transpose(inverse(model))) * normal;
    UV = uv;
    lightVec = lightPos - vec3(model * vec4(pos, 1.0));

    gl_Position = projection * view * model * vec4(pos, 1.0f);
}