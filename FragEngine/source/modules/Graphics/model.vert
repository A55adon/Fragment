#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;

out vec4 bColor;
out vec3 bNormal;
out vec3 bPos;
out vec2 TexCoord;

uniform vec3 faceNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // ← add this

out vec4 FragPosLightSpace;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    bColor = aColor;
    bNormal = normalize(mat3(transpose(inverse(model))) * faceNormal);
    bPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTex;

    FragPosLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0);
}