#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D k_sampler0;
uniform vec2 k_resolution;
uniform vec2 k_direction;
uniform float k_time;

void main()
{
	float radius = 2;
	float hstep = k_direction.x;
	float vstep = k_direction.y;
	float blur = radius / ( k_resolution.x * hstep + k_resolution.y * vstep );

	vec4 sum = texture2D( k_sampler0 , vec2( UV.x - 4.0 * blur * hstep , UV.y - 4.0 * blur * vstep ) ) * 0.0162162162;
	sum += texture2D( k_sampler0 , vec2( UV.x - 3.0 * blur * hstep , UV.y - 3.0 * blur * vstep ) ) * 0.0540540541;
	sum += texture2D( k_sampler0 , vec2( UV.x - 2.0 * blur * hstep , UV.y - 2.0 * blur * vstep ) ) * 0.1216216216;
	sum += texture2D( k_sampler0 , vec2( UV.x - 1.0 * blur * hstep , UV.y - 1.0 * blur * vstep ) ) * 0.1945945946;
	sum += texture2D( k_sampler0 , vec2( UV.x , UV.y ) ) * 0.2270270270;
	sum += texture2D( k_sampler0 , vec2( UV.x + 1.0 * blur * hstep , UV.y + 1.0 * blur * vstep ) ) * 0.1945945946;
	sum += texture2D( k_sampler0 , vec2( UV.x + 2.0 * blur * hstep , UV.y + 2.0 * blur * vstep ) ) * 0.1216216216;
	sum += texture2D( k_sampler0 , vec2( UV.x + 3.0 * blur * hstep , UV.y + 3.0 * blur * vstep ) ) * 0.0540540541;
	sum += texture2D( k_sampler0 , vec2( UV.x + 4.0 * blur * hstep , UV.y + 4.0 * blur * vstep ) ) * 0.0162162162;

	sum *= 1.25;

	color = vec4( sum.rgb, 1 );
}
