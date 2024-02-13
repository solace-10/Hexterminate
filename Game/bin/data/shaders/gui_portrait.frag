#version 330 core

in vec2 UV;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;
uniform vec2 k_resolution = vec2( 1440, 900 );

float rand( vec2 co ) 
{
	return fract( sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453 );
}

void main()
{
	vec4 c0 = texture( k_sampler0, UV );
	
	vec2 size = vec2( 131.0, 160.0 );
	vec2 coord = UV * size;

	if ( mod( floor( coord.y ), 2.0 ) < 1.0 )
	{
		float luminance = dot( c0.xyz, vec3( 0.2126, 0.7152, 0.0722 ) );
		colour = vec4( luminance, luminance, luminance, 1.0 ) * c0;
	}
	else
	{
		float t = pow( ( ( ( 1.0 + sin( k_time ) * 0.5 )
			 * 0.8 + sin( k_time * cos( UV.y ) * 41415.92653 ) * .0125 )
			 * 1.5 + sin( k_time * 7.0 ) * 0.5 ), 5.0 );
	
		vec4 c1 = texture( k_sampler0, coord / ( size + vec2( t * 0.10, 0.0 ) ) );
		vec4 c2 = texture( k_sampler0, coord / ( size - vec2( t * 0.15, 0.0 ) ) );
		vec4 c3 = texture( k_sampler0, coord / ( size + vec2( t * 0.20, 0.0 ) ) );
	
		colour = vec4( c3.r, c2.g, c1.b, 1 );

		float r = rand( vec2( UV.x + k_time, UV.y + k_time ) );
		colour = colour * 0.8 + colour * r * 0.2;
	}
}
