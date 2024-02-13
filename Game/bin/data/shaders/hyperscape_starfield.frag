#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform float k_time = 0.0;

const float kStarSize = 0.1;
const float kStarBrightness = 0.5; // Value >= 0.0 and < 4.0

vec3 getStarColor()
{
    return mix( vec3( 0.6, 0.4, 0.0 ), vec3( 0.4, 0.3, 0.2 ), sin( k_time / 18.0 ) / 2.0 + 1.0 );
}

float getGlow(vec2 uv)
{
    float x = min(length(uv), 1.);
    return abs(pow(x - 1.0, 3.0));
}

float getIntensity(vec2 uv)
{
    float x = min(length(uv), 1.);   
    return pow(1. - (x - kStarSize), 4. - kStarBrightness);
}

void main()
{
    // Center the UVs in the middle of the surface so we are working
    // in -1 to 1, with 0 being the center of the star.
    vec2 centerUV = 2. * (UV - vec2(0.5));

    vec3 glowColor = getStarColor() * getGlow(centerUV); 
    vec3 intensityColor = vec3(1.) * getIntensity(centerUV);
    vec3 col = mix(glowColor, intensityColor, intensityColor.r);

    // Output to screen
    color = vec4(col, 1.0 - pow(vcolor.a, 20.0));
}
