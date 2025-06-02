#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

// Function to convert HSV to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    float hue = fract(gl_FragCoord.x * 0.005);
    vec3 rgb = hsv2rgb(vec3(hue, 1.0, 1.0));
    outColor = vec4(rgb, 1.0);
}