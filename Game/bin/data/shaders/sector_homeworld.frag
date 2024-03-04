#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;

void main()
{
    color = texture( k_sampler0, UV );
}
