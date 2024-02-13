#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;

uniform mat4 k_worldViewProj;
uniform vec2 k_parallax;

void main()
{
	float parallaxAmount = -32.0;
	gl_Position = k_worldViewProj * vec4( vertexPosition + vec3( k_parallax * parallaxAmount, 0.0 ), 1.0 );
	UV = vertexUV;
}
