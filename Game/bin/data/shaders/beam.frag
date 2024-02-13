#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform float k_time = 0.0;
uniform sampler2D k_sampler0; // beam

void main()
{
	colour = texture( k_sampler0, UV - vec2(0, 3 * k_time) ) * vcolour;
}
