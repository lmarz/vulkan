#version 460

layout(location=0) in vec3 NORMAL;
layout(location=1) in vec2 UV;
layout(location=2) in vec3 lightVec;

layout(binding=1) uniform sampler2D tex;

layout(binding=2) readonly buffer Light {
    vec3 lightPos;
    vec3 lightColor;
};

layout(location=0) out vec4 outColor;

void main() {
    vec3 N = normalize(NORMAL);
    vec3 L = normalize(lightVec);

    vec3 ambient = 0.1 * lightColor;
    vec3 diffuse = max(dot(N, L), 0.0) * lightColor;

    outColor = vec4(ambient + diffuse, 1.0) * texture(tex, UV);
}