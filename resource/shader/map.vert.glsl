#version 330 core

layout (location = 0) in vec2 vPos;
layout (location = 1) in uint vColor;

flat out uint fColor;

void main()
{
    gl_Position = vec4(vPos, 0.0, 1.0);
    fColor = vColor;
}