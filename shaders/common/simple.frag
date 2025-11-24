#version 450

// Input from vertex shader
layout(location = 0) in vec3 vColor;

// Output color
layout(location = 0) out vec4 fragColor;

void main() {
    // Simply output the vertex color
    fragColor = vec4(vColor, 1.0);
}