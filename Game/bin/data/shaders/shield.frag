#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0;
uniform bool k_quantum = false;
uniform float k_shieldStrength = 1;	// Value between 0 and 1

uniform float k_quantumScale = 1.0f;
uniform float k_quantumGaps = 0.1f;
uniform float k_quantumIntensity = 8.0f;
uniform vec3 k_quantumColour = vec3( 1.0, 1.0, 1.0 );

const float pi = 3.14159265359;
const float triangleScale = 0.816497161855865; // ratio of edge length and height

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 getTriangleCoords(vec2 uv) 
{
    uv.y /= triangleScale;
    uv.x -= uv.y / 2.0;
    vec2 center = floor(uv);
    vec2 local = fract(uv);
    
    center.x += center.y / 2.0;
    center.y *= triangleScale;
    
    if (local.x + local.y > 1.0) 
	{
    	local.x -= 1.0 - local.y;
        local.y = 1.0 - local.y;
        center.y += 0.586;
        center.x += 1.0; 
    } else 
	{
        center.y += 0.287;
    	center.x += 0.5;
    }
    
    return vec4(center, local);
}

float getBackground(vec4 triangle) 
{
    float dst = min(triangle.z, min(triangle.w, 1.0 - triangle.z - triangle.w)) - 0.05;
	
    if ( dst < k_quantumGaps )
	{
        return 0.0;
    }

    return pow(0.5 + cos(-abs(triangle.x) * 0.4 + rand(triangle.xy) * 2.0 + k_time * 4.0), 2.0) * 0.08;    
}

float getColor(vec2 uv) 
{
    return getBackground(getTriangleCoords(uv * 6.0)) * k_quantumIntensity;
}

void main()
{
	if (k_quantum)
	{
		vec2 p = UV;
		p *= k_quantumScale;
		colour = vec4(k_quantumColour, min(getColor(p), 1.0f));
	}
	else
	{
		// Shield noise becomes stronger the weaker it is
		// Maximum shield strength is capped at 0.9 so we always get some noise getting through
		float ns = rand( vec2( UV.x + k_time, UV.y + k_time ) ) / 2 + 0.5;
		float shieldNoiseMultiplier = 1 - ns * ( 1 - min( k_shieldStrength, 0.9 ) ); 

		vec2 uv = vec2( UV.x, UV.y + sin( k_time ) * 0.1 );
		vec4 grid = texture( k_sampler0, uv ) + texture( k_sampler0, UV );

		colour = clamp( vcolour * shieldNoiseMultiplier + vec4( 0.2, 0.2, 0.2, 0 ), 0, 1 ) * grid;
	}
}
