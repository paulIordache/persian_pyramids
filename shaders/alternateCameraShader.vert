#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fNormal;
out vec4 fPosEye;
out vec2 TexCoords;
out vec4 fragPosLightSpace; // Light-space position
out vec3 fToLight;
out mat4 viewMatrix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceTrMatrix; // Transformation to light space
uniform bool reverse_normals;


void main()
{
	// Compute eye space coordinates
	fPosEye = view * model * vec4(vPosition, 1.0f);
	fToLight = vec3(model * vec4(vPosition, 1.0f));

	if (reverse_normals)
		fNormal = normalize(normalMatrix * (-1) * vNormal);
	else
		fNormal = normalize(normalMatrix * vNormal);
	TexCoords = vTexCoords;
	viewMatrix = view;

	// Transform position into light space
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);

//	fragPosLightSpaceSpot = lightSpaceTrMatrixSpot * model * vec4(vPosition, 1.f);

	// Final vertex position
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
}