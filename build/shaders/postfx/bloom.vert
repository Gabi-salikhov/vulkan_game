#version 450

// Vertex input
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

// Outputs to fragment shader
layout(location = 0) out vec2 vTexCoord;

// Uniforms
layout(push_constant) uniform PushConstants {
    vec2 texelSize;
} pushConstants;

void main() {
    // Pass through texture coordinates
    vTexCoord = aTexCoord;
    
    // Calculate vertex position
    gl_Position = vec4(aPosition, 0.0, 1.0);
}   w