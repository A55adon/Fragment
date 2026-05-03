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
uniform vec3 shadowLightDirection;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    float normalToLight = max(dot(normalize(bNormal), normalize(shadowLightDirection)), 0.0);
    float bias = max(0.0005 * (1.0 - normalToLight), 0.00005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;
    return shadow;
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

    float shadow = 0.0;

    for (int i = 0; i < lightCount; i++)
    {
        vec3 lightDir = lights[i].position - bPos;
        float distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

        float diff = max(dot(normal, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        vec3 diffuse = diff * baseColor * lights[i].color;
        vec3 specular = 0.4 * spec * lights[i].color;

        float shadowFactor = 1.0;

        if (i == 0) {
            shadow = ShadowCalculation(FragPosLightSpace);
            shadowFactor = 1.0 - shadow;
        }

        lighting += (diffuse + specular) * lights[i].emission * attenuation * shadowFactor;
    }

    FragColor = vec4(lighting, 1.0);
}
