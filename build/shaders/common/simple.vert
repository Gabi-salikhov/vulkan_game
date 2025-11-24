#version 450

// Vertex input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

// Output to fragment shader
layout(location = 0) out vec3 vColor;

void main() {
    // Pass color through to fragment shader
    vColor = aColor;
    
    // Set position
    gl_Position = vec4(aPosition, 1.0);
}