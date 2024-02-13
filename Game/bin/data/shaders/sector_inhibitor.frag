#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;

float rand( vec2 co ) 
{
	return fract( sin( dot( co.xy , vec2( 12.9898, 78.233 ) ) ) * 43758.5453 );
}

void main()
{
	float ns = rand( vec2( UV.x + k_time, UV.y + k_time ) ) * 0.25 + 0.25;
	vec4 backgroundColour = texture( k_sampler0, UV );
    colour = backgroundColour * vcolour * ns * 0.7;
}
