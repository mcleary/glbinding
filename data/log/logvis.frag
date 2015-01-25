#version 150

in vec3 Color;

out vec4 outColor;

void main()
{
    float total = floor(gl_FragCoord.y);
    bool isEven = mod(total,30.0)<15.0;
    vec4 col1 = vec4(Color, 1.0);
    vec4 col2 = vec4(0.0, 0.0, 0.0, 1.0);
    outColor = (isEven)? col1:col2;
}