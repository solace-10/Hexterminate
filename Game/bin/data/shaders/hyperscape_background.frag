#version 330 core

in vec2 UV;

out vec3 color;

uniform float k_time = 0.0;

uniform sampler2D k_sampler0;
uniform sampler2D k_sampler1;

void main()
{
    vec2 warpUV = 2. * UV;

    float d = length( warpUV );
    vec2 st = warpUV * 0.1 + 0.2 * vec2( cos( 0.007 * k_time + d ), sin( 0.0072 * k_time - d ) );

    vec3 warpedCol = texture( k_sampler1, st ).rgb;
    float w = max( warpedCol.r, 0.85);

    vec2 offset = 0.01 * cos( warpedCol.rg * 3.14159 );
    vec3 col = texture( k_sampler0, UV + offset ).rgb * vec3( 0.8, 0.8, 1.5 );
    col *= w * 1.2;

    color = mix(col, texture( k_sampler0, UV + offset ).rgb, 0.5);

    color *= pow( 16.0 * UV.x *UV.y * ( 1.0 - UV.x ) * ( 1.0 - UV.y ), 0.15 + abs(cos(k_time * 0.1)) * 0.35 );
}
