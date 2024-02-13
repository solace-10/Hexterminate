#version 330 core

in vec2 UV;
in vec3 normal;
in vec3 viewDir;

out vec4 colour;

uniform sampler2D k_sampler0; // diffuse
uniform sampler2D k_sampler1; // specular
uniform sampler2D k_sampler2; // paint maps
uniform vec4 k_a = vec4( 0, 0, 0, 1 );
uniform vec4 k_e = vec4( 1, 1, 1, 1 );
uniform float k_time = 0;

// Primary and secondary paints used by the paint map
uniform vec4 k_primaryPaint = vec4( 0, 0, 0.75, 1 );
uniform vec4 k_secondaryPaint = vec4( 0, 0, 0, 1 );

// Overlay formula taken from http://www.deepskycolors.com/archive/2010/04/21/formulas-for-Photoshop-blending-modes.html
float Overlay( float target, float blend, float alpha )
{
	float result;
	if ( target > 0.5 )
		result = 1 - ( 1 - 2 * ( target - 0.5 ) ) * ( 1 - blend );
	else
		result = 2 * target * blend;
	return mix( target, result, alpha );
}

void main()
{
	colour = vec4( 0, 0, 0, 1 );
	vec3 lightDir = normalize( vec3( 1, 0.25, 1 ) );

	// Diffuse component
	float diffuseLight = max( dot( lightDir, normal ), 0);
	vec4 diffuse = texture( k_sampler0, UV ) * diffuseLight;
	diffuse = vec4( diffuse.x, diffuse.y, diffuse.z, 1 );

	// Specular component
	float specularIntensity = clamp( 1 - sqrt( viewDir.x * viewDir.x + viewDir.y * viewDir.y ), 0, 1 );
	specularIntensity *= clamp( dot( vec3( 0, 0, 1 ), normal ), 0, 1 );
	float specular = texture(k_sampler1, UV).r * pow( specularIntensity, 10 );

	vec4 paintMask = texture( k_sampler2, UV );
	vec4 paintPrimary = paintMask.r * k_primaryPaint;
	vec4 paintSecondary = paintMask.g * k_secondaryPaint;
	vec4 paintedDiffuse = vec4( Overlay( diffuse.r, paintPrimary.r, paintMask.r * paintMask.a ), Overlay( diffuse.g, paintPrimary.g, paintMask.r * paintMask.a ), Overlay( diffuse.b, paintPrimary.b, paintMask.r * paintMask.a ), 1 );
	paintedDiffuse = vec4( Overlay( paintedDiffuse.r, paintSecondary.r, paintMask.g * paintMask.a ), Overlay( paintedDiffuse.g, paintSecondary.g, paintMask.g * paintMask.a ), Overlay( paintedDiffuse.b, paintSecondary.b, paintMask.g * paintMask.a ), 1 );

	// The emissive mask is in the k_sampler2's blue channel.
	vec4 emissive = texture( k_sampler2 , UV).b * k_e;

	colour = clamp( k_a + specular + paintedDiffuse + emissive, 0, 1 );
	colour.a = 1;
}
