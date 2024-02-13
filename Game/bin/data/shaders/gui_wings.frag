#version 330 core

in vec2 UV;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;
uniform vec4 k_colour = vec4( 1.0, 1.0, 1.0, 1.0 );

void main()
{
	vec2 distortion = vec2( 0.0, cos( k_time ) * 0.01 );
	vec2 extendedUV = UV * 0.9 + 0.05 + distortion * 2.0;
    vec4 baseColour = texture( k_sampler0, UV + distortion );
	vec4 outlineColour = texture( k_sampler0, extendedUV ) * k_colour;
	colour = mix( outlineColour, baseColour, baseColour.a );
	colour.a = min( k_colour.a, max( baseColour.a, outlineColour.a ) );
}
