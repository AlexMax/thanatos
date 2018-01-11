#version 330 core

in float fColor;

out vec4 color;

uniform sampler2D uPalette;

void main()
{
    color = texture(uPalette, vec2(fColor, 0.0));
}
