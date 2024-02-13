#version 330 core

in vec2 UV;

out vec4 colour;

uniform float k_time = 0;

float rand( vec2 co ) 
{
	return fract( sin( dot( co.xy , vec2( 12.9898, 78.233 ) ) ) * 43758.5453 );
}

void main()
{
	float ns = rand( vec2( UV.x + k_time, UV.y + k_time ) ) / 2 + 0.1;
	colour = vec4( ns, 0.0, 0.0, 1.0 );
}
