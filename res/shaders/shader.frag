#version 460

layout(location=0) in vec2 UV;

layout(binding=1) uniform sampler2D tex;

layout(location=0) out vec4 outColor;

void main() {
    outColor = texture(tex, UV);
}