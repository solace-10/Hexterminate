#version 330 core

in vec2 UV;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform vec4 k_colour = vec4( 1.0, 1.0, 1.0, 1.0 );

void main()
{
	colour = texture( k_sampler0, UV ) * k_colour;
}
