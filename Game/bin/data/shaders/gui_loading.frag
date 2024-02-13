#version 330 core

in vec2 UV;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;
uniform vec2 k_resolution = vec2( 1440, 900 );

const float pi = 3.14159265359;
const float triangleScale = 0.816497161855865; // ratio of edge length and height
const vec3 orange = vec3(0.0, 1.0, 1.0);

float rand( vec2 co ) 
{
	return fract( sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453 );
}

vec4 getTriangleCoords( vec2 uv )
{
	uv.y /= triangleScale;
	uv.x -= uv.y / 2.0;
	vec2 center = floor(uv);
	vec2 local = fract(uv);

	center.x += center.y / 2.0;
	center.y *= triangleScale;

	if (local.x + local.y > 1.0) {
		local.x -= 1.0 - local.y;
		local.y = 1.0 - local.y;
		center.y += 0.586;
		center.x += 1.0; 
	}
	else
	{
		center.y += 0.287;
		center.x += 0.5;
	}

	return vec4(center, local);
}

vec4 getLoader(vec4 triangle) 
{
	if (length(triangle.xy) > 1.6)
	{
		return vec4(0.0);
	}

	float angle = atan(triangle.x, triangle.y);
	float seed = rand(triangle.xy);
	float dst = min(triangle.z, min(triangle.w, 1.0 - triangle.z - triangle.w)) * 15.0;
	float glow = dst < pi ? pow(sin(dst), 1.5) : 0.0;

	return vec4(mix(orange, vec3(1.0), glow * 0.07), pow(0.5 + 0.5 * sin(angle + k_time * 6.0 + seed), 4.0));
}

vec3 getLine(vec2 uv)
{
	if (uv.y > 0.0 && uv.y < 0.005)
	{
		float intensity = min(abs(uv.x) * 1., 1.0);
		intensity = intensity * intensity * intensity;
		return vec3(0., intensity, intensity);
	}
	else
	{
		return vec3(0.);
	}
}

vec3 getColor(vec2 uv)
{
	uv *= 2.0 / k_resolution.y;
	vec4 loader = getLoader(getTriangleCoords(uv * 20.0));
	return getLine(uv) + loader.rgb * loader.a;
}

void main()
{
	vec2 fragCoord = UV * k_resolution;
	fragCoord = fragCoord - 0.5 * k_resolution;
	
	colour.rgb = 0.25 * (getColor(fragCoord)
				+ getColor(fragCoord + vec2(0.5, 0.0))
				+ getColor(fragCoord + vec2(0.5, 0.5))
				+ getColor(fragCoord + vec2(0.0, 0.5)));
	colour.a = 1.;
}
