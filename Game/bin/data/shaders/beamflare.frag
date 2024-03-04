#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform float k_time = 0.0;
uniform sampler2D k_sampler0; // flare
uniform sampler2D k_sampler1; // mask

void main()
{
	vec4 flareMask = texture( k_sampler1, UV + vec2(0, 3 * k_time) );
	color = texture( k_sampler0, UV ) * flareMask * vcolor;
}
