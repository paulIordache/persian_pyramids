
#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace; // Light-space position

out vec4 fColor;

// Lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Shadow mapping parameters
float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 32.0f;

vec3 ambient;
vec3 diffuse;
vec3 specular;

float computeShadow() {
	// Perform perspective divide to get normalized device coordinates
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0, 1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	// Check if fragment is outside shadow map
	if (normalizedCoords.z > 1.0)
	return 0.0;

	// Retrieve depth from shadow map
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get current fragment depth
	float currentDepth = normalizedCoords.z;

	// Apply bias to prevent shadow acne
	float bias = max(0.05 * (1.0 - dot(normalize(fNormal), lightDir)), 0.005);

	// Check if fragment is in shadow
	return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void computeLightComponents() {
	vec3 cameraPosEye = vec3(0.0f); // In eye coordinates, the viewer is at the origin
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	// Ambient light
	ambient = ambientStrength * lightColor;

	// Diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	// Specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

void main() {
	computeLightComponents();

	// Shadow computation
	float shadow = computeShadow();

	// Apply textures and compute final color
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); // Base orange color
	vec3 textureDiffuse = texture(diffuseTexture, fTexCoords).rgb;
	vec3 textureSpecular = texture(specularTexture, fTexCoords).rgb;

	ambient *= textureDiffuse;
	diffuse *= textureDiffuse * (1.0 - shadow);
	specular *= textureSpecular * (1.0 - shadow);

	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	fColor = vec4(color, 1.0f);
}