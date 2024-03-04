#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform float k_time = 0.0;
uniform sampler2D k_sampler0;

float rand( vec2 co ) 
{
	return fract( sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453 );
}

void main()
{
	float r = rand( vec2( UV.x + k_time, UV.y + k_time ) );
	color = vcolor * texture( k_sampler0, UV );
	color = color * 0.9 + color * r * 0.1;
}
