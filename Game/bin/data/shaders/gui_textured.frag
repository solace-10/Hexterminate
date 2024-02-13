#version 330 core

in vec2 UV;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform vec4 k_colour = vec4( 1, 1, 1, 1 );

void main()
{
	colour = k_colour * texture( k_sampler0, UV );
}
