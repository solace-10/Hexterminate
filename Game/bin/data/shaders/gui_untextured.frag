#version 330 core

out vec4 color;

uniform vec4 k_color = vec4( 1, 0, 0, 0.5 );

void main()
{
	color = k_color;
}
