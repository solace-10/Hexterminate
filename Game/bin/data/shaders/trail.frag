#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform sampler2D k_sampler0;

void main()
{
	colour = vcolour * texture( k_sampler0, UV );
}
