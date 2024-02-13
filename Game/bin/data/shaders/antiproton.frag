#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform float k_time = 0.0;
uniform float k_internalRadius = 0.1;
uniform float k_externalRadius = 1.0;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define SQ3 1.73205080757
#define I_R 80.0
#define F_R 600.0
#define SPEED 4.4

float random( in vec2 _st )
{
    return fract(sin(dot(_st.xy, vec2(12.9898,78.233))) * 43758.54531237);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise( in vec2 _st )
{
    vec2 i = floor( _st );
    vec2 f = fract( _st );

    // Four corners in 2D of a tile
    float a = random( i );
    float b = random( i + vec2( 1.0, 0.0 ) );
    float c = random( i + vec2( 0.0, 1.0 ) );
    float d = random( i + vec2( 1.0, 1.0 ) );

    vec2 u = f * f * ( 3.0 - 2.0 * f );

    return mix(a, b, u.x) + 
            (c - a)* u.y * (1. - u.x) + 
            (d - b) * u.x * u.y;
}

float noise (float _st) 
{ 
    return fract(abs(sin(_st)));
}

vec4 flare( float alpha, vec2 main, float seed, float dir )
{
	float amnt = 0.6+sin(seed)*8.0;
	float ang = atan(main.y, main.x);
	float t = k_time * SPEED * dir;
	float n = noise(vec2( (seed+ang*amnt+t*0.1) + cos(alpha*13.8+noise(t+ang+seed)*3.0)*0.2+seed/20.0,seed+t+ang));

	n *= pow(noise(vec2(seed * 194.0 + ang*amnt+t + cos( alpha*2.0*n+t*1.1+ang)*2.8,seed+t+ang)+alpha),2.0);
	n *= pow(noise(vec2(seed * 134.0 + ang*amnt+t + cos( alpha*2.2*n+t*1.1+ang)*1.1,seed+t+ang)+alpha),3.0);
	n *= pow(noise(vec2(seed * 123.0 + ang*amnt+t + cos( alpha * 2.3 * n + t * 1.1 + ang ) * 0.8 , seed + t + ang ) + alpha ), 4.0 );
	n *= pow( alpha ,2.6 );
	n *= (ang+PI)/2.0 * (TWO_PI - ang - PI); //fade out flares at pole.
	
	
	n += sqrt(alpha * alpha) * 0.26;
	return vec4(pow(n*2.1,2.0),n,n,n);
}

void main()
{
    vec2 uv = UV - 0.5;
	colour = vec4(0.0);
	float len = length(uv);
	float alpha = pow(clamp( k_externalRadius - len + k_internalRadius, 0.0, k_externalRadius )/ k_externalRadius ,6.0);
	colour += flare(alpha,uv,75.0,1.0);
	colour += flare(alpha,uv,35.0,1.0);
	colour += flare(alpha,uv,21.0,1.0);
	colour += flare(alpha,uv,1.2,1.0);
	colour.xyz = clamp(colour.xyz,0.0,1.0);
	if (alpha >= 0.99)
    {
		colour.xyz -= (alpha - 0.99) * 150.0 * vcolour.r;
	}

	//colour = vec4(UV.x, UV.y, 0.0f, 1.0f);
}