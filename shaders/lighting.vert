#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceTrMatrix;

void main()
{
    gl_Position = projection * view *  model * vec4(vPosition, 1.0f);
    FragPos = vec3(model * vec4(vPosition, 1.0f));
    Normal = mat3(transpose(inverse(model))) * vNormal;
    fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);

    TexCoords = vTexCoords;
}