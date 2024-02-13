#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform float k_time = 0.0;
uniform sampler2D k_sampler0;

float rand( vec2 co ) 
{
	return fract( sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453 );
}

void main()
{
	float r = rand( vec2( UV.x + k_time, UV.y + k_time ) );
	colour = vcolour * texture( k_sampler0, UV );
	colour = colour * 0.9 + colour * r * 0.1;
}
