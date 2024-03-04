#version 330 core

in vec2 UV;
in vec4 vcolor;

out vec4 color;

uniform sampler2D k_sampler0;

void main()
{
	color = vcolor * texture( k_sampler0, UV );
}
