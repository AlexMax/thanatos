#version 330 core

in vec2 fTexCoord;

uniform sampler2D uTexture;

out vec4 color;

void main()
{
    color = texture(uTexture, fTexCoord);
}