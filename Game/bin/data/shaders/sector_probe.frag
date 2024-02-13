#version 330 core

in vec2 UV;
in vec4 vcolour;

out vec4 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;

void main()
{
	vec4 backgroundColour = texture( k_sampler0, UV );
    colour = backgroundColour * vcolour;
}
