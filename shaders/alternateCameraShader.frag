#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 TexCoords;
in vec4 fragPosLightSpace; // Light-space position
in vec3 fToLight;
in mat4 viewMatrix;

out vec4 fColor;

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

// Lighting
uniform DirLight light;
uniform PointLight pointLight[10];
uniform SpotLight spotLight;
float far_plane = 1000.f;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform samplerCube depthMap;
uniform vec3 cameraPos;
vec3 cameraPosEye;
//uniform vec3 lightDir;
//uniform vec3 lightColor;

// Shadow mapping parameters
//float ambientStrength = 0.2f;
//float specularStrength = 0.5f;
float shininess = 5.0f;
//
//vec3 ambient;
//vec3 diffuse;
//vec3 specular;

//float ShadowCalculation(vec3 fragPos)
//{
//	// get vector between fragment position and light position
//	vec3 fragToLight = fragPos - pointLight.wPosition;
//	// use the light to fragment vector to sample from the depth map
//	float closestDepth = texture(depthMap, normalize(fragToLight)).r;
//
//	// it is currently in linear range between [0,1]. Re-transform back to original value
//	closestDepth *= far_plane;
//	// now get current linear depth as the length between the fragment and ligh t position
//	float bias = 0.05f;
//	return(closestDepth + bias) > length(fragToLight) ? 0.f : 1.f;
//}

float computeShadow(DirLight light) {
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0) return 0.0;

	float shadow = 0.0;
	float bias = max(0.05 * (1.0 - dot(normalize(fNormal), light.direction)), 0.005);
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for (int x = -2; x <= 2; ++x) {
		for (int y = -2; y <= 2; ++y) {
			vec2 offset = vec2(x, y) * texelSize;
			float closestDepth = texture(shadowMap, normalizedCoords.xy + offset).r;
			shadow += (normalizedCoords.z - bias > closestDepth) ? 1.0 : 0.0;
		}
	}
	shadow /= 25.0; // 5x5 PCF
	return shadow;
}

vec3 CalcDirLight() {

	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(light.direction );
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	vec3 amb = light.ambient * light.lightColor;

	// Diffuse shading
	vec3 diff = max( dot( normalEye, lightDirN ), 0.0 ) * light.lightColor;

	// Specular shading
	vec3 reflectDir = reflect( -lightDirN, normalEye );
	float spec = pow( max( dot( viewDirN, reflectDir ), 0.0 ), shininess);
	vec3 spe = light.specular * spec * light.lightColor;

	float shadow = computeShadow(light);

	vec3 textureDiffuse = texture(diffuseTexture, TexCoords).rgb;
	vec3 textureSpecular = texture(specularTexture, TexCoords).rgb;

	// Combine results
	amb *= textureDiffuse;
	diff *= textureDiffuse * (1.f - shadow);
	spe *= textureSpecular * (1.f - shadow);

	return min((amb + (1.0f - shadow)*diff) + (1.0f - shadow)*spe, 1.0f);
}

vec3 computeLightComponents(PointLight pointLight)
{


	//transform normal
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
	vec3 ambient = att * pointLight.ambient * pointLight.lightColor;

	//compute diffuse light
	vec3 diffuse = att * pointLight.diffuse * max(dot(normalEye, lightDirN), 0.0f) * pointLight.lightColor;

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	vec3 specular = att * pointLight.specular * specCoeff * pointLight.lightColor;

		vec3 textureDiffuse = texture( diffuseTexture, TexCoords ).rgb;
		vec3 specularDiffuse = texture( specularTexture, TexCoords ).rgb;

	//float shadow = ShadowCalculation(fToLight);

		ambient *= textureDiffuse;
		diffuse *= textureDiffuse;
		specular *= specularDiffuse;

	return  min(ambient + (diffuse + specular), 1.0f);
}

vec3 CalcSpotLight()
{

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
	vec3 ambient = att * spotLight.ambient * spotLight.lightColor;

	//compute diffuse light
	vec3 diffuse = att * spotLight.diffuse * max(dot(normalEye, lightDirN), 0.0f) * spotLight.lightColor;

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	vec3 specular = att * spotLight.specular * specCoeff * spotLight.lightColor;

	vec3 textureDiffuse = texture( diffuseTexture, TexCoords ).rgb;
	vec3 specularDiffuse = texture( specularTexture, TexCoords ).rgb;

	vec3 spotLightDirEye = normalize(mat3(viewMatrix) * spotLight.direction);


	// Spotlight intensity
	float theta = dot( lightDirN, normalize( -spotLightDirEye) );
	float epsilon = spotLight.cutOff - spotLight.outerCutOff;
	float intensity = clamp( ( theta - spotLight.outerCutOff ) / epsilon, 0.0, 1.0 );

	ambient *= textureDiffuse;
	diffuse *= textureDiffuse;
	specular *= specularDiffuse;

	ambient *= intensity;
	diffuse *= intensity;
	specular *= intensity;

	return min( ambient + diffuse + specular, 1.f );
}

float computeFog() {
	float fogDensity = .001f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, .0f, 1.f);
}

void main() {
	cameraPosEye = normalize(mat3(viewMatrix) * cameraPos);


	float fogFactor = computeFog();
	vec4 fogColor = vec4(.2f, .2f, .2f, .2f);
	vec4 v4color = vec4(CalcDirLight() + CalcSpotLight(), 1.f);

	for (int i = 0; i < 10; i++) {
		v4color += vec4(computeLightComponents(pointLight[i]), 1.f);
	}

	fColor = mix(fogColor, v4color, fogFactor);
}

