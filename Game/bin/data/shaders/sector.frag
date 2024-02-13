#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform float k_time = 0.0;

void main()
{
    colour = vcolour;
}
