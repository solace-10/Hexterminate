#version 330 core

in vec2 UV;

out vec4 color;

uniform float k_screenWidth = 1024.0f;
uniform float k_screenHeight = 768.0f;
uniform float k_time = 0.0;
uniform sampler2D k_sampler0;
uniform sampler2D lineSampler;

float gNPixels = 0.5f;
float gThreshhold = 0.2f;

float getGray(vec4 c)
{
    return(dot(c.rgb, vec3(0.33333, 0.33333, 0.33333)));
}

vec3 edgeDetectPS(vec2 texCoord, float NPixels, float Threshhold)
{
    vec2 ox = vec2(NPixels/k_screenWidth,0.0);
    vec2 oy = vec2(0.0,NPixels/k_screenHeight);
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

void main()
{
	vec3 lineSweep = texture( lineSampler, UV - vec2( 0.0f, k_time * 0.05f ) ).rgb;
	vec2 uvDistortion = vec2( lineSweep.r * lineSweep.r * 0.001f, 0.0f );

	color = vec4( texture( k_sampler0, UV + uvDistortion ).rgb, 1 );

	vec3 edgeDetect = 1.0f - edgeDetectPS( UV + uvDistortion, gNPixels, gThreshhold );
	vec3 edgeDetectColorised = edgeDetect * vec3(0.0f, 0.2f, 0.2f);
	color.rgb += edgeDetectColorised * lineSweep;
	color.rgb += lineSweep * vec3( 0.0f, 0.1f, 0.1f );
	color = clamp( color, 0, 1 );
}
