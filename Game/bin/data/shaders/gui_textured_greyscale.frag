#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D k_sampler0;

void main()
{
	color = texture( k_sampler0, UV );
	float g = sqrt( dot( color, vec4( 0.299, 0.587, 0.114, 0.000 ) ) );
	color = vec4( g );
}
