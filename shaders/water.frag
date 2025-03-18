#version 330 core

in vec2 fragTexCoord;
in vec3 fragPosition;

out vec4 fragColor;

// Uniforms for reflection and refraction
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D distortionMap; // Distortion texture
uniform float moveFactor;        // Movement factor for distortion animation

// Uniforms for light and water properties
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition; // Camera position

uniform float distortionStrength = 0.02; // Strength of distortion effect
uniform float specularStrength = 0.5;    // Strength of specular reflection

const float shininess = 32.0;

// Function to calculate specular lighting
vec3 calculateSpecular(vec3 lightDir, vec3 normal, vec3 viewDir) {
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    return lightColor * spec * specularStrength;
}

void main() {
    // Distort texture coordinates
    vec2 distortion = texture(distortionMap, fragTexCoord + vec2(moveFactor)).rg * 2.0 - 1.0;
    distortion *= distortionStrength;
    vec2 distortedTexCoord = fragTexCoord + distortion;

    // Sample reflection and refraction textures
    vec4 reflectionColor = texture(reflectionTexture, distortedTexCoord);
    vec4 refractionColor = texture(refractionTexture, distortedTexCoord);

    // Blend reflection and refraction based on view angle
    vec3 normal = vec3(0.0, 1.0, 0.0); // Approximate normal for water surface
    vec3 viewDir = normalize(viewPosition - fragPosition);
    float fresnelFactor = pow(1.0 - dot(viewDir, normal), 3.0);

    vec3 lightDir = normalize(lightPosition - fragPosition);
    vec3 specular = calculateSpecular(lightDir, normal, viewDir);

    vec4 finalColor = mix(refractionColor, reflectionColor, fresnelFactor);
    fragColor = vec4(finalColor.rgb + specular, 1.0);
}
