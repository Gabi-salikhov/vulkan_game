#version 450

// Vertex input
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

// PBR specific inputs
layout(location = 5) in vec4 aBoneWeights;
layout(location = 6) in ivec4 aBoneIndices;

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
    mat4 boneMatrices[128];
} transformUBO;

// Outputs to fragment shader
layout(location = 0) out vec3 vWorldPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec3 vTangent;
layout(location = 4) out vec3 vBitangent;
layout(location = 5) out vec3 vViewPosition;
layout(location = 6) out vec3 vWorldNormal;
layout(location = 7) out vec4 vClipPosition;

// Bone animation
vec4 getBonePosition() {
    vec4 position = vec4(0.0);
    for (int i = 0; i < 4; i++) {
        float weight = aBoneWeights[i];
        if (weight > 0.0) {
            int boneIndex = aBoneIndices[i];
            mat4 boneMatrix = transformUBO.boneMatrices[boneIndex];
            position += boneMatrix * vec4(aPosition, 1.0) * weight;
        }
    }
    return position;
}

vec3 getBoneNormal() {
    vec3 normal = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        float weight = aBoneWeights[i];
        if (weight > 0.0) {
            int boneIndex = aBoneIndices[i];
            mat4 boneMatrix = transformUBO.boneMatrices[boneIndex];
            normal += mat3(boneMatrix) * aNormal * weight;
        }
    }
    return normalize(normal);
}

void main() {
    // Apply bone animation
    vec4 animatedPosition = getBonePosition();
    vec3 animatedNormal = getBoneNormal();
    
    // Calculate world position
    vec4 worldPosition = transformUBO.model * animatedPosition;
    vWorldPosition = worldPosition.xyz;
    
    // Calculate view position
    vec4 viewPosition = cameraUBO.view * worldPosition;
    vViewPosition = viewPosition.xyz;
    
    // Calculate normal in world space
    vNormal = mat3(transformUBO.normalMatrix) * animatedNormal;
    vWorldNormal = normalize(mat3(transformUBO.model) * animatedNormal);
    
    // Pass through texture coordinates
    vTexCoord = aTexCoord;
    
    // Pass through tangent and bitangent
    vTangent = mat3(transformUBO.model) * aTangent;
    vBitangent = mat3(transformUBO.model) * aBitangent;
    
    // Calculate clip position
    vClipPosition = cameraUBO.viewProjection * worldPosition;
    gl_Position = vClipPosition;
}