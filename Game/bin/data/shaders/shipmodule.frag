#version 330 core

in vec2 UV;
in vec3 objPosition;
in vec3 normal;
in vec3 viewDir;

out vec4 colour;

uniform sampler2D k_sampler0; // diffuse
uniform sampler2D k_sampler1; // specular
uniform sampler2D k_sampler2; // paint maps
uniform sampler2D k_sampler3; // damage map
uniform float k_repairEdgeAlpha = 0;
uniform float k_repairEdgeOffset = 0;
uniform vec4 k_a = vec4( 0, 0, 0, 1 );
uniform vec4 k_e = vec4( 1, 1, 1, 1 );
uniform float k_time = 0;

// Primary and secondary paints used by the paint map
uniform vec4 k_primaryPaint = vec4( 0, 0, 0.75, 1 );
uniform vec4 k_secondaryPaint = vec4( 0, 0, 0, 1 );

// Damage overlay control - damage becomes more visible as health goes down
uniform float k_health = 1;

// Clip plane used by the hyperspace jump sequence
uniform vec4 k_clip = vec4( 0, 0, 0, 0 );
uniform vec4 k_clipForward = vec4( 1, 0, 0, 0 );
uniform int k_clipActive = 0;

// Ikeda effect colour, used by armour modules. Alpha controls the pattern's intensity.
uniform vec4 k_overlayColour = vec4( 0, 0, 0, 0 );

uniform int k_empActive = 0;

float random (float x) 
{
    return fract(sin(x)*1e4);
}

float random(vec2 st) 
{ 
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}

float pattern(vec2 st, vec2 v, float t) 
{
    vec2 p = floor(st+v);
    return step(t, random(100.+p*.000001)+random(p.x)*0.5 );
}

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

vec4 ikeda( vec2 uv, float intensity )
{
    vec2 st = uv;

    vec2 grid = vec2( 20, 20 );
    st *= grid;
    
    vec2 ipos = floor( st );  // integer
    vec2 fpos = fract( st );  // fraction
    
    vec2 vel = vec2( k_time * max( grid.x,grid.y ) ); // time
    vel *= vec2( -1, 0 ) * random( 1 +ipos.y ); // direction

    // Assign a random value base on the integer coord
    vec2 offset = vec2( 0.1,0 );

    vec3 color = vec3( 0 );
	float t = 1 - intensity;
    color.r = pattern( st+offset, vel, 0.5 + t );
    color.g = pattern( st, vel, 0.5 + t);
    color.b = pattern( st-offset, vel, 0.5 + t );

    // Margins
    color *= step( 0.2, fpos.y );

    return vec4( color, 1 );
}

void main()
{
	colour = vec4( 0, 0, 0, 1 );
	vec4 objPosWorld = vec4(objPosition - k_clip.xyz, 1.0);
	vec3 lightDir = normalize( vec3( 1, 0.25, 1 ) );

	if ( k_clipActive == 1 && dot( normalize( objPosWorld ), k_clipForward ) > 0 )
	{
		discard;
	}

	// Diffuse component
	float diffuseLight = max( dot( lightDir, normal ), 0);
	vec4 diffuse = texture( k_sampler0, UV ) * diffuseLight;
	diffuse = vec4( diffuse.x, diffuse.y, diffuse.z, 1 );

	// Specular component
	float specularIntensity = clamp( 1 - sqrt( viewDir.x * viewDir.x + viewDir.y * viewDir.y ), 0, 1 );
	specularIntensity *= clamp( dot( vec3( 0, 0, 1 ), normal ), 0, 1 );
	float specular = texture(k_sampler1, UV).r * pow( specularIntensity, 10 );

	// Repair edge
	float lum = (diffuse.r + diffuse.g + diffuse.b) / 3;
	float repairEdge = 0.0;
	if (lum > k_repairEdgeOffset && lum < k_repairEdgeOffset + 0.05)
		repairEdge = 1.0;
	vec4 repairEdgeColour = vec4(0.373, 0.441, 0.306, 1) * k_repairEdgeAlpha * repairEdge;

	vec4 paintMask = texture( k_sampler2, UV );
	vec4 paintPrimary = paintMask.r * k_primaryPaint;
	vec4 paintSecondary = paintMask.g * k_secondaryPaint;
	vec4 paintedDiffuse = vec4( Overlay( diffuse.r, paintPrimary.r, paintMask.r * paintMask.a ), Overlay( diffuse.g, paintPrimary.g, paintMask.r * paintMask.a ), Overlay( diffuse.b, paintPrimary.b, paintMask.r * paintMask.a ), 1 );
	paintedDiffuse = vec4( Overlay( paintedDiffuse.r, paintSecondary.r, paintMask.g * paintMask.a ), Overlay( paintedDiffuse.g, paintSecondary.g, paintMask.g * paintMask.a ), Overlay( paintedDiffuse.b, paintSecondary.b, paintMask.g * paintMask.a ), 1 );

	// The emissive mask is in the k_sampler2's blue channel.
	vec4 emissive = texture( k_sampler2 , UV).b * k_e;

	//
	vec4 ikedaOverlay = vec4( k_overlayColour.rgb, 1 ) * ikeda( UV, k_overlayColour.a );

	colour = clamp( k_a + specular + paintedDiffuse + repairEdgeColour + emissive + ikedaOverlay, 0, 1 );

	// Damage component
	vec4 damageMap = texture( k_sampler3, UV );
	if ( damageMap.b >= k_health ) // The damage cutoff layer is in the blue channel
	{
		float glowFactor = abs(cos(k_time * 0.5)) * 0.5 + 0.5;
		colour *= damageMap.r;
		colour = clamp( colour + ( 1 - damageMap.r ) * damageMap.g * vec4( 1, 0.4, 0, 1 ) * glowFactor, 0, 1 );
	}

	if ( k_empActive == 1 )
	{
		float c = random( vec2( UV.x + k_time, UV.y + k_time ) ) * 0.25;
		colour.g = clamp( colour.g + c, 0, 1 );
		colour.b = clamp( colour.b + c, 0, 1 );
	}

	colour.a = 1;
}
