#version 150

in vec2 Texcoord;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
    outColor = vec4(Color, 1.0);
}