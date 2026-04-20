#version 330 core

// No color output needed — OpenGL writes gl_FragDepth automatically.
// The framebuffer for the shadow pass has no color attachment (glDrawBuffer(GL_NONE)),
// so this shader intentionally produces no output.

void main()
{
}
