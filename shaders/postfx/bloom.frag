#version 450

// Fragment shader inputs
layout(location = 0) in vec2 vTexCoord;

// Outputs
layout(location = 0) out vec4 fragColor;

// Bloom uniforms
layout(set = 0, binding = 0) uniform BloomUBO {
    float threshold;
    float intensity;
    int blurRadius;
    int blurPasses;
    float exposure;
    float gamma;
} bloomUBO;

// Texture samplers
layout(set = 0, binding = 1) uniform sampler2D inputTexture;
layout(set = 0, binding = 2) uniform sampler2D blurredTexture;

// Constants
const float PI = 3.14159265359;

// Gaussian blur weights
vec4 getGaussianWeights(float sigma) {
    vec4 weights = vec4(0.0);
    float sum = 0.0;
    
    // Calculate Gaussian weights
    for (int i = -2; i <= 2; i++) {
        float x = float(i);
        float weight = exp(-(x * x) / (2.0 * sigma * sigma));
        weights[i + 2] = weight;
        sum += weight;
    }
    
    // Normalize weights
    return weights / sum;
}

// Apply Gaussian blur
vec4 applyBlur(sampler2D tex, vec2 texCoord, vec2 texelSize, int radius) {
    vec4 result = vec4(0.0);
    vec4 weights = getGaussianWeights(2.0);
    
    // Horizontal blur
    for (int i = -radius; i <= radius; i++) {
        vec2 offset = vec2(float(i) * texelSize.x, 0.0);
        result += texture(tex, texCoord + offset) * weights[i + radius];
    }
    
    return result;
}

// Extract bright pixels
vec4 extractBrightPixels(vec4 color) {
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    if (brightness > bloomUBO.threshold) {
        return color * (brightness - bloomUBO.threshold) / brightness;
    }
    
    return vec4(0.0);
}

// Tone mapping
vec3 toneMap(vec3 color) {
    // ACES filmic tone mapping
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    
    color = ((color * (a * color + b)) / (color * (c * color + d) + e));
    
    return color;
}

// Gamma correction
vec3 gammaCorrect(vec3 color) {
    return pow(color, vec3(1.0 / bloomUBO.gamma));
}

void main() {
    // Sample input texture
    vec4 color = texture(inputTexture, vTexCoord);
    
    // Extract bright pixels
    vec4 brightPixels = extractBrightPixels(color);
    
    // Apply blur to bright pixels
    vec2 texelSize = 1.0 / vec2(textureSize(inputTexture, 0));
    vec4 blurredPixels = applyBlur(inputTexture, vTexCoord, texelSize, bloomUBO.blurRadius);
    
    // Apply multiple blur passes
    for (int i = 1; i < bloomUBO.blurPasses; i++) {
        blurredPixels = applyBlur(inputTexture, vTexCoord, texelSize, bloomUBO.blurRadius + i);
    }
    
    // Combine blurred bright pixels with original image
    vec3 finalColor = color.rgb + blurredPixels.rgb * bloomUBO.intensity;
    
    // Apply tone mapping
    finalColor = toneMap(finalColor);
    
    // Apply gamma correction
    finalColor = gammaCorrect(finalColor);
    
    // Apply exposure
    finalColor *= bloomUBO.exposure;
    
    // Output final color
    fragColor = vec4(finalColor, color.a);
}