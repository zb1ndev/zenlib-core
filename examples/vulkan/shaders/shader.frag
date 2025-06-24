#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = vec4(fragColor, 1.0f);
    vec4 texture = vec4(color * texture(texSampler, fragTexCoord).rgba);
    outColor = texture;
}