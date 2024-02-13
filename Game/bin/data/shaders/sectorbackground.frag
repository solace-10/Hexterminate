#version 330 core

in vec2 UV;

out vec3 colour;

uniform vec3 k_coreColour = vec3( 0.8, 0.65, 0.3 );
uniform vec3 k_coronaColour = vec3( 0.8, 0.35, 0.1 );
uniform vec2 k_offset = vec2( 0.0, 0.0 );
uniform float k_distance = 1.0;
uniform vec2 k_resolution = vec2( 512.0, 512.0 );

uniform float k_time = 0.0f;
uniform sampler2D k_backgroundSampler;
uniform sampler2D k_starSampler;
uniform vec2 k_starUvScale = vec2( 1.0, 1.0 );
uniform bool k_hasStar = true;

float snoise(vec3 uv, float res)	// by trisomie21
{
	const vec3 s = vec3( 1e0, 1e2, 1e4 );
	
	uv *= res;
	
	vec3 uv0 = floor( mod( uv, res ) ) * s;
	vec3 uv1 = floor( mod( uv + vec3( 1.0 ), res ) ) * s;
	
	vec3 f = fract( uv ); f = f * f * ( 3.0 - 2.0 * f );
	
	vec4 v = vec4( uv0.x + uv0.y + uv0.z, uv1.x + uv0.y + uv0.z,
		      	   uv0.x + uv1.y + uv0.z, uv1.x + uv1.y + uv0.z );
	
	vec4 r = fract( sin( v * 1e-3 ) * 1e5 );
	float r0 = mix( mix( r.x, r.y, f.x ), mix( r.z, r.w, f.x ), f.y );
	
	r = fract( sin( ( v + uv1.z - uv0.z ) * 1e-3 ) *1e5 );
	float r1 = mix( mix( r.x, r.y, f.x ), mix( r.z, r.w, f.x ), f.y );
	
	return mix( r0, r1, f.z ) * 2.0 - 1.0;
}

// Main sequence star effect based on https://www.shadertoy.com/view/4dXGR4
vec4 star( in vec2 uv )
{
	float brightness = 0.5;
	float radius = 0.24 + brightness * 0.2;
	float invRadius = 1.0 / radius;
	float time = k_time * 0.025;

	uv += k_offset;
	vec2 p = -0.5 + uv;
	float aspect = k_resolution.x / k_resolution.y;
	p.x *= aspect;
	p *= k_distance;

	float fade		= pow( length( 2.0 * p ), 0.5 );
	float fVal1		= 1.0 - fade;
	float fVal2		= 1.0 - fade;
	
	float angle		= atan( p.x, p.y )/6.2832;
	float dist		= length( p );
	vec3 coord		= vec3( angle, dist, time * 0.1 );
	
	float newTime1	= abs( snoise( coord + vec3( 0.0, -time * ( 0.35 + brightness * 0.001 ), time * 0.015 ), 15.0 ) );
	float newTime2	= abs( snoise( coord + vec3( 0.0, -time * ( 0.15 + brightness * 0.001 ), time * 0.015 ), 45.0 ) );	
	for( int i = 1; i <= 7; i++ )
	{
		float power = pow( 2.0, float(i + 1) );
		fVal1 += ( 0.5 / power ) * snoise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 10.0 ) * ( newTime1 + 1.0 ) ) );
		fVal2 += ( 0.5 / power ) * snoise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 25.0 ) * ( newTime2 + 1.0 ) ) );
	}
	
	float corona		= pow( fVal1 * max( 1.1 - fade, 0.0 ), 2.0 ) * 50.0;
	corona				+= pow( fVal2 * max( 1.1 - fade, 0.0 ), 2.0 ) * 50.0;
	corona				*= 1.2 - newTime1;
	vec3 sphereNormal 	= vec3( 0.0, 0.0, 1.0 );
	vec3 dir 			= vec3( 0.0 );
	vec3 starSphere		= vec3( 0.0 );
	
	vec2 sp = -1.0 + 2.0 * uv;
	sp.x *= aspect;
	sp *= k_distance;
	sp *= ( 2.0 - brightness );
  	float r = dot( sp, sp );
	float f = ( 1.0 - sqrt( abs( 1.0 - r ) ) ) / r + brightness * 0.5;
	if ( dist < radius )
	{
		corona *= pow( dist * invRadius, 24.0 );
  		vec2 newUv;
 		newUv.x = sp.x*f;
  		newUv.y = sp.y*f;
		newUv += vec2( time, 0.0 );
		
		vec3 texSample 	= texture( k_starSampler, newUv ).rgb;
		float uOff		= ( texSample.g * brightness * 4.5 + time );
		vec2 starUV		= newUv + vec2( uOff, 0.0 );
		starSphere		= texture( k_starSampler, starUV ).rgb;
	}
	
	float starGlow = min( max( 1.0 - dist * ( 1.0 - brightness ), 0.0 ), 1.0 );
	vec4 fragColour;
	fragColour.rgb = vec3( f * ( 0.75 + brightness * 0.3 ) * k_coreColour ) + starSphere + corona * k_coreColour + starGlow * k_coronaColour;
	fragColour.a = ( 0.299 * fragColour.g + 0.587 * fragColour.g + 0.114 * fragColour.b );
	return fragColour;
}

void main()
{
	if ( k_hasStar )
	{
		vec4 backgroundColour = texture( k_backgroundSampler, UV );
		vec4 starColour = star( UV * k_starUvScale );
		colour = mix( backgroundColour.rgb, starColour.rgb, starColour.a );
	}
	else
	{
		colour = texture( k_backgroundSampler, UV ).rgb;
	}
}
