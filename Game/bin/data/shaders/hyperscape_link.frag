#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform float k_time = 0.0;

void main()
{
    float verticalGradient = 1.0 - ( cos( UV.x * 3.145 *2. ) / 2. + 0.5 );
    float horizontalGradient = fract( 3. * UV.y - k_time * 1. );
    float c = horizontalGradient * verticalGradient;
    color = vec4( c, c, c, c );
}
