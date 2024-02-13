#version 330 core

in vec2 UV;

out vec3 colour;

uniform sampler2D k_sampler0;

void main()
{
	colour = texture( k_sampler0, UV ).rgb;
}
