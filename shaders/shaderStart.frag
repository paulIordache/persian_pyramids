#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 TexCoords;
in vec4 fragPosLightSpace; // Light-space position
in vec3 fToLight;
in mat4 viewMatrix;

out vec4 fColor;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct DirLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	vec3 lightColor;
};

struct PointLight
{
	vec3 position;
	vec3 wPosition;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 lightColor;
};

struct SpotLight
{
	vec3 position;
	vec3 wPosition;

	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 lightColor;
};

#define MAX 20
#define SPOT 3

// Lighting
uniform DirLight light;
uniform PointLight pointLights[MAX];
uniform SpotLight spotLights[SPOT];
uniform Material material;
float far_plane = 25.f;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D ambientTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;
//uniform samplerCube environmentMap;

uniform sampler2D shadowMap;
uniform samplerCube depthMap;
uniform vec3 cameraPos;
uniform float uTime;
vec3 cameraPosEye;

float shininess = 35.0f;

const float PI = 3.14159265359;

vec3 getNormalFromMap()
{
	vec3 tangentNormal = texture(normalTexture, TexCoords).xyz * 2.0 - 1.0;

	vec3 Q1  = dFdx(fPosEye.xyz);
	vec3 Q2  = dFdy(fPosEye.xyz);
	vec2 st1 = dFdx(TexCoords);
	vec2 st2 = dFdy(TexCoords);

	vec3 N   = normalize(fNormal);
	vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
	vec3 B  = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

float ShadowCalculation(vec3 fragPos, PointLight pointLight) {
	// get vector between fragment position and light position
	vec3 fragToLight = fragPos - pointLight.wPosition;
	// use the light to fragment vector to sample from the depth map
	float closestDepth = texture(depthMap, normalize(fragToLight)).r;

	// it is currently in linear range between [0,1]. Re-transform back to original value
	closestDepth *= far_plane;
	// now get current linear depth as the length between the fragment and ligh t position
	float bias = 0.05f;
	return(closestDepth + bias) > length(fragToLight) ? 0.f : 1.f;
}

float computeShadow(vec3 direction) {
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0) return 0.0;

	float shadow = 0.0;
	float bias = max(0.05 * (1.0 - dot(normalize(fNormal), direction)), 0.005);
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for (int x = -2; x <= 2; ++x) {
		for (int y = -2; y <= 2; ++y) {
			vec2 offset = vec2(x, y) * texelSize;
			float closestDepth = texture(shadowMap, normalizedCoords.xy + offset).r;
			shadow += (normalizedCoords.z - bias > closestDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 18.0; // 5x5 PCF
	return shadow;
}

vec3 CalcDirLight() {
	float metallic = texture(metallicTexture, TexCoords).r; // Metallic value
	float roughness = texture(roughnessTexture, TexCoords).r; // Roughness value
	float textureAmbient = texture(ambientTexture, TexCoords).r;
	vec3 textureDiffuse = texture(diffuseTexture, TexCoords).rgb;
	vec3 textureSpecular = texture(specularTexture, TexCoords).rgb;

	// Transform normal to eye space
	vec3 normalEye = normalize(fNormal);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 lightDirN = normalize(light.direction);
	vec3 reflectDir = reflect(-lightDirN, normalEye);

	// Specular shading
	float specPower = 1.0 / (roughness * roughness + 0.001); // Specular power based on roughness
	float spec = pow(max(dot(viewDirN, reflectDir), 0.0), specPower);

	vec3 fresnel = mix(vec3(0.04), textureDiffuse, metallic); // Fresnel interpolation
	vec3 F0 = fresnelSchlick(max(dot(normalEye, viewDirN), 0.0), fresnel);
	vec3 spe = light.specular * spec * light.lightColor * F0;

	vec3 diff = max(dot(normalEye, lightDirN), 0.0) * light.lightColor * vec3(0.7f, 0.7f, 0.7f);
	vec3 amb = light.ambient * light.lightColor * vec3(0.3f, 0.3f, 0.3f) * diff;

	float shadow = computeShadow(lightDirN);

	// Reflection calculation
//	vec3 reflectionVec = reflect(-viewDirN, normalEye);
//	vec3 envReflection = texture(environmentMap, reflectionVec).rgb;
//	vec3 reflectionColor = envReflection * mix(vec3(0.04), textureDiffuse, metallic);
//	reflectionColor *= 1.0 - roughness; // Diminish reflection with roughness

	// Combine results
	amb *= textureAmbient * textureDiffuse * (1.0 - metallic);
	diff *= textureDiffuse * (1.0 - metallic);
	spe *= textureSpecular;

	vec3 finalColor = amb + diff * (1.0 - shadow) + spe * (1.0 - shadow);

	return min(finalColor, vec3(1.0));
}

vec3 computeLightComponents(PointLight pointLight, int i)
{
	float metallic = texture(metallicTexture, TexCoords).r;
	float roughness = texture(roughnessTexture, TexCoords).r;
	float specPower = 1.0 / (roughness * roughness + 0.001); // Specular power based on roughness
	float textureAmbient = texture(ambientTexture, TexCoords).r;
	vec3 textureDiffuse = texture( diffuseTexture, TexCoords ).rgb;
	vec3 specularDiffuse = texture( specularTexture, TexCoords ).rgb;

	//transform normal
	vec3 tangentNormal = texture(normalTexture, TexCoords).rgb * 2.0 - 1.0;

	// Transform normal to eye space
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(pointLight.position - fPosEye.xyz);
	//lightDir *= inverseTrans;

	//compute view direction
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float dist = length(pointLight.position - fPosEye.xyz);
	float att = 1.0f / (pointLight.constant + pointLight.linear * dist + pointLight.quadratic * (dist * dist));

	//compute ambient light
	vec3 ambient = att * pointLight.ambient * vec3(0.3f, 0.3f, 0.3f) * pointLight.lightColor;

	//compute diffuse light
	vec3 diffuse = att * pointLight.diffuse * max(dot(normalEye, lightDirN), 0.0f) * vec3(0.5f, 0.5f, 0.5f) * pointLight.lightColor;

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), specPower);
	vec3 fresnel = mix(vec3(0.04), textureDiffuse, metallic); // Fresnel interpolation based on metallic value
	vec3 F0 = fresnelSchlick(max(dot(normalEye, viewDirN), 0.0), fresnel);

	vec3 specular = att * pointLight.specular * specCoeff * fresnel * pointLight.lightColor;

	float shadow;
	if (i == 0)
	shadow = ShadowCalculation(fToLight, pointLight);
	else shadow = 0;
	//float shadow = 0;

		ambient *= textureAmbient * textureDiffuse ;
		diffuse *= textureDiffuse ;
		specular *= specularDiffuse;

	return  min(ambient + (1 - shadow) * (diffuse + specular), 1.0f);
}
vec3 CalcSpotLight(SpotLight spotLight)
{
	float metallic = texture(metallicTexture, TexCoords).r; // Metallic value
	float roughness = texture(roughnessTexture, TexCoords).r; // Roughness value

	//transform normal
	vec3 normalEye = normalize(fNormal);

	vec3 lightDirN = normalize(spotLight.position - fPosEye.xyz);


	//compute view direction
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float dist = length(spotLight.position - fPosEye.xyz);
	float att = 1.0f / (spotLight.constant + spotLight.linear * dist + spotLight.quadratic * (dist * dist));

	//compute ambient light
	vec3 ambient = att * spotLight.ambient * spotLight.lightColor * vec3(0.3f, 0.3f, 0.3f);

	//compute diffuse light
	vec3 diffuse = att * spotLight.diffuse * max(dot(normalEye, lightDirN), 0.0f) * spotLight.lightColor * vec3(0.7f, 0.7f, 0.7f);

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specPower = 1.0 / (roughness * roughness + 0.001); // Specular power based on roughness

	float specCoeff = pow(max(dot(normalEye, reflection), 0.0f), specPower);
	vec3 fresnel = mix(vec3(0.04), vec3(1.0), metallic); // Fresnel interpolation based on metallic value

	vec3 specular = att * spotLight.specular * specCoeff * spotLight.lightColor * fresnel;

	float textureAmbient = texture(ambientTexture, TexCoords).r;
	vec3 textureDiffuse = texture( diffuseTexture, TexCoords ).rgb;
	vec3 specularDiffuse = texture( specularTexture, TexCoords ).rgb;

	vec3 spotLightDirEye = normalize(mat3(viewMatrix) * spotLight.direction);


	// Spotlight intensity
	float theta = dot( lightDirN, normalize( -spotLightDirEye) );
	float epsilon = spotLight.cutOff - spotLight.outerCutOff;
	float intensity = clamp( ( theta - spotLight.outerCutOff ) / epsilon, 0.0, 1.0 );

	ambient *= textureAmbient * textureDiffuse * (1 - metallic);
	diffuse *= textureDiffuse * (1 - metallic);
	specular *= specularDiffuse;

	ambient *= intensity;
	diffuse *= intensity;
	specular *= intensity;

	return min( ambient + diffuse + specular, 1.f );
}


// Add a noise function for smooth, pseudo-random patterns
float noise(vec2 uv) {
	return fract(sin(dot(uv.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Smooth noise function for better results
float smoothNoise(vec2 uv) {
	vec2 i = floor(uv);
	vec2 f = fract(uv);
	float a = noise(i);
	float b = noise(i + vec2(1.0, 0.0));
	float c = noise(i + vec2(0.0, 1.0));
	float d = noise(i + vec2(1.0, 1.0));
	vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// Fog computation with heatwave effect
float computeHeatwaveFog(vec3 fPosEye, float uTime) {
	float fogDensity = 0.002f;
	float fragmentDistance = length(fPosEye);

	// Add heatwave distortion using time and noise
	vec2 distortionUV = fPosEye.xy * 0.1 + vec2(uTime * 0.05, uTime * 0.03);
	float distortion = smoothNoise(distortionUV) * 2.0 - 1.0; // Range: [-1, 1]
	fragmentDistance += distortion * 10.0; // Amplify distortion for effect

	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {
	cameraPosEye = normalize(mat3(viewMatrix) * cameraPos);


	float fogFactor = computeHeatwaveFog(fPosEye.xyz, uTime);
	vec4 fogColor = vec4(.2f, .2f, .2f, .2f);
	vec4 v4color = vec4(CalcDirLight(), 1.f);

	for (int i = 0; i < MAX; i++) {
		v4color += vec4(computeLightComponents(pointLights[i], i), 1.f);
	}

	for (int i = 0; i < SPOT; i++) {
		v4color += vec4(CalcSpotLight(spotLights[i]), 1.f);
	}

	fColor = mix(fogColor, v4color, fogFactor);
}

