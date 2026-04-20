#version 330 core

in vec4 vColor;
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform int useTexture;

void main()
{
    if (useTexture == 1)
    {
        // Font atlas is single-channel (red). Use it as the alpha mask
        // and apply the vertex color as the text color.
        FragColor = vec4(vColor.rgb, vColor.a * texture(uTexture, vTex).r);
    }
    else
    {
        FragColor = vColor;
    }
}
