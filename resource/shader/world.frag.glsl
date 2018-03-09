#version 330 core

in vec2 fTexCoord;

uniform sampler2D uTexture;
uniform sampler2D uPalette;

out vec4 color;

void main()
{
    vec4 index = texture(uTexture, fTexCoord);
    color = texture(uPalette, vec2(index.x, 0.0));
}
