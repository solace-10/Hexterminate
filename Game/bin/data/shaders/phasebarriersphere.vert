#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;
out vec3 objPosition;

uniform mat4 k_worldViewProj;
uniform mat4 k_world;

void main()
{
	gl_Position = k_worldViewProj * vec4( vertexPosition, 1 );
	objPosition = vec4( k_world * vec4( vertexPosition, 1 ) ).xyz;
	UV = vertexUV;
}
