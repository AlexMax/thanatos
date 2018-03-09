#version 330 core

flat in uint fColor;

out vec4 color;

uniform sampler2D uPalette;

void main()
{
    color = texelFetch(uPalette, ivec2(fColor, 0), 0);
}
