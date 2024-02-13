#version 330 core

in vec2 UV;

out vec4 colour;

uniform float k_time = 0;
uniform sampler2D k_sampler0;

void main()
{
	vec4 sampledTexture = texture( k_sampler0, UV );

	float rotationSpeed = sampledTexture.g * 0.5;

	float angle = rotationSpeed * k_time;
	mat2 rotMat = mat2( cos( angle ), -sin( angle ),
					    sin( angle ),  cos( angle ) );

	vec2 newUV = UV - vec2( 0.5, 0.5 );
	newUV = newUV * rotMat + vec2( 0.5, 0.5 );
	sampledTexture = texture( k_sampler0, newUV );

	colour = vec4( sampledTexture.r, sampledTexture.r, sampledTexture.r, sampledTexture.a );
}
