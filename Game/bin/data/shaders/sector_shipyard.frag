#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;

void main()
{
	vec4 backgroundColor = texture( k_sampler0, UV );
    color = clamp( backgroundColor * vcolor + vcolor * 0.2, 0, 1 );
}
