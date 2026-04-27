#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;

// Use names that match exactly what UI.frag reads
out vec4 vColor;
out vec2 vTex;

void main()
{
    // Vertices are already in NDC space — drawUIObject computes and
    // bakes the NDC offset directly into the vertex positions.
    // Applying any camera matrix here would destroy that.
    gl_Position = vec4(aPos, 1.0);

    vColor = aColor;
    vTex = aTex;
}
