#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform float k_time = 0.0;

const float kStarSize = 0.001;
const float kStarBrightness = 0.5; // Value >= 0.0 and < 4.0
const vec3 kStarColor = vec3(1.0, 0.7, 0.0);

float getGlow(vec2 uv)
{
    // Gentle slope for the glow:
    // https://www.desmos.com/calculator/fjwv1ctp4h
    float pi = 3.14159;
    float x = min(length(uv), 1.);
    float l = cos(x*pi) / 2. + 0.5; 
    l = (l + (1.-x))/2.;
    return l;
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

    vec3 glowColor = vcolor.rgb * getGlow(centerUV); 
    vec3 intensityColor = vec3(1.) * getIntensity(centerUV);
    vec3 col = mix(glowColor, intensityColor, intensityColor.r );

    // Output to screen
    color = vec4(col, intensityColor.r);
}
