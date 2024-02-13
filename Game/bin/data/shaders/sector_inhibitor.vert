#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
// layout(location = 2) is for normals, which isn't used in this shader
layout(location = 3) in vec4 vertexColour;

out vec2 UV;
out vec4 vcolour;

uniform mat4 k_worldViewProj;

void main()
{
	gl_Position = k_worldViewProj * vec4( vertexPosition, 1 );
	UV = vertexUV;
	vcolour = vertexColour;
}
