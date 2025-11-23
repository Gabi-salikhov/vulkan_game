#version 450

// Vertex input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

// Uniforms
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 cameraPosition;
} cameraUBO;

layout(set = 0, binding = 1) uniform TransformUBO {
    mat4 model;
    mat4 normalMatrix;
} transformUBO;

// Outputs to fragment shader
layout(location = 0) out vec3 vWorldPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec3 vTangent;
layout(location = 4) out vec3 vBitangent;
layout(location = 5) out vec3 vViewPosition;

void main() {
    // Calculate world position
    vec4 worldPosition = transformUBO.model * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    
    // Calculate view position
    vec4 viewPosition = cameraUBO.view * worldPosition;
    vViewPosition = viewPosition.xyz;
    
    // Calculate normal in world space
    vNormal = mat3(transformUBO.normalMatrix) * aNormal;
    
    // Pass through texture coordinates
    vTexCoord = aTexCoord;
    
    // Pass through tangent and bitangent
    vTangent = mat3(transformUBO.model) * aTangent;
    vBitangent = mat3(transformUBO.model) * aBitangent;
    
    // Calculate clip position
    gl_Position = cameraUBO.viewProjection * worldPosition;
}