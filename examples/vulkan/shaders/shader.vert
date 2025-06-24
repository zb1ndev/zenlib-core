#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = pc.model * vec4(inPosition.x, -inPosition.y, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}