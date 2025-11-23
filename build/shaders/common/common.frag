#version 450

// Fragment shader inputs
layout(location = 0) in vec3 vWorldPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;
layout(location = 5) in vec3 vViewPosition;

// Outputs
layout(location = 0) out vec4 fragColor;

// PBR material properties
layout(set = 1, binding = 0) uniform MaterialUBO {
    vec4 albedo;
    float metallic;
    float roughness;
    float ao;
    float emissive;
    int useAlbedoMap;
    int useNormalMap;
    int useMetallicRoughnessMap;
    int useAOMap;
    int useEmissiveMap;
} materialUBO;

// Texture samplers
layout(set = 1, binding = 1) uniform sampler2D albedoMap;
layout(set = 1, binding = 2) uniform sampler2D normalMap;
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;
layout(set = 1, binding = 5) uniform sampler2D emissiveMap;

// Lighting
layout(set = 2, binding = 0) uniform LightingUBO {
    vec3 ambientColor;
    float ambientIntensity;
    int lightCount;
    vec3 lightPositions[16];
    vec3 lightColors[16];
    vec3 lightDirections[16];
    float lightIntensities[16];
    int lightTypes[16];
} lightingUBO;

// Camera
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 cameraPosition;
} cameraUBO;

// Constants
const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

// Calculate normal from normal map
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, vTexCoord).xyz * 2.0 - 1.0;
    
    // Construct TBN matrix
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent - dot(vTangent, N) * N);
    vec3 B = normalize(vBitangent - dot(vBitangent, N) * N - dot(vBitangent, T) * T);
    
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

// Distribution GGX
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry function
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

// Geometry function for Smith
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel function
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// Calculate PBR lighting
vec3 calculatePBR(vec3 albedo, vec3 normal, vec3 viewDir, vec3 worldPos) {
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, materialUBO.metallic);
    
    vec3 Lo = vec3(0.0);
    
    // Iterate over lights
    for (int i = 0; i < lightingUBO.lightCount; i++) {
        vec3 lightPos = lightingUBO.lightPositions[i];
        vec3 lightColor = lightingUBO.lightColors[i];
        float lightIntensity = lightingUBO.lightIntensities[i];
        int lightType = lightingUBO.lightTypes[i];
        
        vec3 lightDir;
        float attenuation = 1.0;
        
        if (lightType == 0) { // Directional light
            lightDir = normalize(-lightingUBO.lightDirections[i]);
        } else if (lightType == 1) { // Point light
            lightDir = normalize(lightPos - worldPos);
            float distance = length(lightPos - worldPos);
            attenuation = 1.0 / (distance * distance);
        } else if (lightType == 2) { // Spot light
            lightDir = normalize(lightPos - worldPos);
            float distance = length(lightPos - worldPos);
            attenuation = 1.0 / (distance * distance);
        }
        
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float NDF = DistributionGGX(normal, halfwayDir, materialUBO.roughness);
        float G = GeometrySmith(normal, viewDir, lightDir, materialUBO.roughness);
        vec3 F = FresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - materialUBO.metallic;
        
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
        vec3 specular = numerator / denominator;
        
        float NdotL = max(dot(normal, lightDir), 0.0);
        Lo += (kD * albedo / PI + specular) * lightColor * lightIntensity * attenuation * NdotL;
    }
    
    // Ambient lighting
    vec3 ambient = lightingUBO.ambientColor * lightingUBO.ambientIntensity * albedo;
    
    vec3 color = ambient + Lo;
    
    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    
    return color;
}

void main() {
    // Get material properties
    vec3 albedo = materialUBO.albedo.rgb;
    if (materialUBO.useAlbedoMap == 1) {
        albedo = texture(albedoMap, vTexCoord).rgb;
    }
    
    float metallic = materialUBO.metallic;
    if (materialUBO.useMetallicRoughnessMap == 1) {
        metallic = texture(metallicRoughnessMap, vTexCoord).b;
    }
    
    float roughness = materialUBO.roughness;
    if (materialUBO.useMetallicRoughnessMap == 1) {
        roughness = texture(metallicRoughnessMap, vTexCoord).g;
    }
    
    float ao = materialUBO.ao;
    if (materialUBO.useAOMap == 1) {
        ao = texture(aoMap, vTexCoord).r;
    }
    
    float emissive = materialUBO.emissive;
    if (materialUBO.useEmissiveMap == 1) {
        emissive = texture(emissiveMap, vTexCoord).r;
    }
    
    // Get normal
    vec3 normal = normalize(vNormal);
    if (materialUBO.useNormalMap == 1) {
        normal = getNormalFromMap();
    }
    
    // Calculate view direction
    vec3 viewDir = normalize(cameraUBO.cameraPosition - vWorldPosition);
    
    // Calculate PBR lighting
    vec3 color = calculatePBR(albedo, normal, viewDir, vWorldPosition);
    
    // Add emissive
    color += vec3(emissive);
    
    // Output final color
    fragColor = vec4(color, materialUBO.albedo.a);
}