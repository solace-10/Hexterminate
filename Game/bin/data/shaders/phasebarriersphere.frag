#version 330 core

in vec2 UV;
in vec3 objPosition;

out vec3 colour;

uniform sampler2D k_sampler0;
uniform float k_time = 0.0;
uniform vec4 k_a = vec4( 0, 0, 0, 1 );
uniform vec4 k_clip = vec4( 0, 0, 0, 0 );
uniform vec4 k_clipForward = vec4( 1, 0, 0, 0 );
uniform int k_clipActive = 0;
uniform float k_active = 0.0;

float gNPixels = 0.5f;
float gThreshhold = 0.15f;

float getGray(vec4 c)
{
    return(dot(c.rgb, vec3(0.33333, 0.33333, 0.33333)));
}

vec3 edgeDetectPS(vec2 texCoord, float NPixels, float Threshhold)
{
    vec2 ox = vec2(NPixels/298.0,0.0);
    vec2 oy = vec2(0.0,NPixels/179.0);
    vec2 uv = texCoord;
    vec2 PP = uv - oy;
    vec4 CC = texture(k_sampler0,PP-ox); float g00 = getGray(CC);
    CC = texture(k_sampler0,PP);    float g01 = getGray(CC);
    CC = texture(k_sampler0,PP+ox); float g02 = getGray(CC);
    PP = uv;
    CC = texture(k_sampler0,PP-ox); float g10 = getGray(CC);
    CC = texture(k_sampler0,PP);    float g11 = getGray(CC);
    CC = texture(k_sampler0,PP+ox); float g12 = getGray(CC);
    PP = uv + oy;
    CC = texture(k_sampler0,PP-ox); float g20 = getGray(CC);
    CC = texture(k_sampler0,PP);    float g21 = getGray(CC);
    CC = texture(k_sampler0,PP+ox); float g22 = getGray(CC);
    float K00 = -1;
    float K01 = -2;
    float K02 = -1;
    float K10 = 0;
    float K11 = 0;
    float K12 = 0;
    float K20 = 1;
    float K21 = 2;
    float K22 = 1;
    float sx = 0;
    float sy = 0;
    sx += g00 * K00;
    sx += g01 * K01;
    sx += g02 * K02;
    sx += g10 * K10;
    sx += g11 * K11;
    sx += g12 * K12;
    sx += g20 * K20;
    sx += g21 * K21;
    sx += g22 * K22; 
    sy += g00 * K00;
    sy += g01 * K10;
    sy += g02 * K20;
    sy += g10 * K01;
    sy += g11 * K11;
    sy += g12 * K21;
    sy += g20 * K02;
    sy += g21 * K12;
    sy += g22 * K22; 
    float dist = sqrt(sx*sx+sy*sy);
    float result = 1;
    if (dist>Threshhold) { result = 0; }
    return vec3(result, result, result);
}

float rand( vec2 co ) 
{
	return fract( sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453 );
}

void main()
{
	vec4 objPosWorld = vec4(objPosition - k_clip.xyz, 1.0);
	if ( k_clipActive == 1 && dot( normalize( objPosWorld ), k_clipForward ) > 0 )
	{
		discard;
	}

	vec2 shiftedUV = UV;
	shiftedUV.x += k_time * 0.2;

	colour = texture( k_sampler0, shiftedUV ).rgb;

	vec3 edgeDetect = 1.0f - edgeDetectPS( shiftedUV, gNPixels, gThreshhold + sin( k_time ) * 0.1 );
	vec3 edgeDetectColourised = edgeDetect * vec3( 1.0, 0.3, 0.0f ) * k_active;

	colour = min( k_a.rgb + colour + edgeDetectColourised, 1.0 );
}
