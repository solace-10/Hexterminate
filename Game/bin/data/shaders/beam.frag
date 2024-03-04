#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform float k_time = 0.0;
uniform sampler2D k_sampler0; // beam

void main()
{
	color = texture( k_sampler0, UV - vec2(0, 3 * k_time) ) * vcolor;
}
