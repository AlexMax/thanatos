#version 330 core

out vec4 oFragColor;
in vec2 testTexCoord;

uniform sampler2D uTexture;

void main()
{
    oFragColor = texture(uTexture, testTexCoord);
}