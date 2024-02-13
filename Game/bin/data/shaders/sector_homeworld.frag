#version 330 core

in vec2 UV;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;

void main()
{
    colour = texture( k_sampler0, UV );
}
