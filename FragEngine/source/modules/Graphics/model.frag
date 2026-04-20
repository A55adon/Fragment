#version 330 core
#define MAX_LIGHTS 16

struct Light {
    vec3 position;
    vec3 color;
    float emission;
};

out vec4 FragColor;

in vec4 bColor;
in vec3 bNormal;
in vec3 bPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace; // FIX: was output from vert shader but never declared here

uniform int useTexture;
uniform sampler2D uTexture;
uniform sampler2D shadowMap; // FIX: used in ShadowCalculation() but never declared

uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Outside the light frustum = no shadow
    if (projCoords.z > 1.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.005;
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

void main()
{
    vec3 baseColor;
    if (useTexture == 1) {
        vec3 texColor = texture(uTexture, TexCoord).rgb;
        // FIX: texColor was read outside its if-block while potentially uninitialized
        baseColor = (length(texColor) == 0.0) ? bColor.rgb : texColor;
    }
    else {
        baseColor = bColor.rgb;
    }

    vec3 normal = normalize(bNormal);
    vec3 viewDir = normalize(viewPos - bPos);

    vec3 lighting = vec3(0.0);

    float ambientStrength = 0.1;
    lighting += baseColor * ambientStrength;

    // FIX: ShadowCalculation() was defined but never called — shadows had no effect
    float shadow = ShadowCalculation(FragPosLightSpace);

    for (int i = 0; i < lightCount; i++)
    {
        vec3 lightDir = lights[i].position - bPos;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

        float diff = max(dot(normal, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        float specStrength = 0.4;

        vec3 diffuse = diff * baseColor * lights[i].color;
        vec3 specular = specStrength * spec * lights[i].color;

        // Only the first light casts shadows (matches your single shadow pass)
        float shadowFactor = (i == 0) ? (1.0 - shadow) : 1.0;

        lighting += (diffuse + specular) * lights[i].emission * attenuation * shadowFactor;
    }

    FragColor = vec4(lighting, 1.0);
}
