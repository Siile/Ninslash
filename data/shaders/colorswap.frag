#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform float colorswap;

void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	float r = c.r; float b = c.b; float g = c.g;

	//float v = min((c.r + c.g + c.b) / 2.0f - 0.5f, 1.0f);
	float v = max(1.0f - (c.r + c.g + c.b) / 1.5f, -2.0f);
	
	c.b += v * max(colorswap*1.2f, -2.0f);
	c.g += v * min(colorswap, 0.95f);
	c.r += v * min(colorswap, 0.95f);
	
	gl_FragColor = c * gl_Color;
}