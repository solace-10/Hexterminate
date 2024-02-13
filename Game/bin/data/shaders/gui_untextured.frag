#version 330 core

out vec4 colour;

uniform vec4 k_colour = vec4( 1, 0, 0, 0.5 );

void main()
{
	colour = k_colour;
}
