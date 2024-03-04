#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D k_sampler0;
uniform vec4 k_color = vec4( 1, 1, 1, 1 );

void main()
{
	color = k_color * texture( k_sampler0, UV );
}
