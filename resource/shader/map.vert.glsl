#version 330 core

layout (location = 0) in vec2 vPos;
layout (location = 1) in uint vColor;

uniform sampler2D uPalette;

flat out vec4 fColor;

void main()
{
    gl_Position = vec4(vPos, 0.0, 1.0);
    fColor = texelFetch(uPalette, ivec2(vColor, 0), 0);
}
