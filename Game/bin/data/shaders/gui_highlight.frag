#version 330 core

out vec4 color;

uniform float k_time = 0.0;

void main()
{
	float c = step( 0.5, fract( ( gl_FragCoord.x + gl_FragCoord.y ) / 16.0 + k_time ) );
	color = vec4( c, c, c, 1.0 );
}
