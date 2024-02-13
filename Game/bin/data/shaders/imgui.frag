#version 330 core

in vec2 UV;
in vec4 vColour;

out vec4 colour;

uniform sampler2D k_sampler0;
void main()
{
	colour = vColour * texture( k_sampler0, UV );
}
