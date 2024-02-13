#version 330 core

layout(location = 0) in vec3 vertexPosition;


uniform mat4 k_worldViewProj;

void main()
{
	gl_Position = k_worldViewProj * vec4( vertexPosition, 1 );
}
