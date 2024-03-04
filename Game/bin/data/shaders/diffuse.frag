#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D k_sampler0;

void main()
{
	color = texture( k_sampler0, UV ).rgb;
}
