#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

out vec2 UV;
out vec3 objPosition;
out vec3 normal;
out vec3 viewDir;

uniform mat4 k_worldViewProj;
uniform mat4 k_world;
uniform mat4 k_worldInverseTranspose;
uniform mat4 k_viewInverse;

void main()
{
	gl_Position = k_worldViewProj * vec4( vertexPosition, 1 );
	objPosition = vec4( k_world * vec4( vertexPosition, 1 ) ).xyz;
	UV = vertexUV;
	normal = vec4( k_worldInverseTranspose * vec4( normalize( vertexNormal ), 1 ) ).xyz;
	vec4 po = vec4( gl_Position.xyz , 1 );
	vec3 pw = vec4( k_worldInverseTranspose * po ).xyz;
	viewDir = normalize( vec3( k_viewInverse[0].w, k_viewInverse[1].w, k_viewInverse[2].w ) - pw );
}
