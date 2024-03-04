#version 330 core

in vec2 UV;
in vec4 vColor;

out vec4 color;

uniform sampler2D k_sampler0;
void main()
{
	color = vColor * texture( k_sampler0, UV );
}
