#version 330 core

out vec4 oFragColor;
in vec3 testColor;
in vec2 testTexCoord;

uniform sampler2D uTexture;
uniform sampler2D uPalette;

void main()
{
    vec4 index = texture(uTexture, testTexCoord);
    oFragColor = texture(uPalette, vec2(index.x, 0.0));
}
