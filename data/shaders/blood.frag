#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform int screenwidth;
uniform int screenheight;

void main (void)
{
	float a = texture2D(texture, gl_TexCoord[0].st).r+texture2D(texture, gl_TexCoord[0].st).g+texture2D(texture, gl_TexCoord[0].st).b;
	
	float StepX = 2.0f / screenwidth;
	float StepY = 2.0f / screenheight;
	
	vec4 c = texture2D(texture, gl_TexCoord[0].st + vec2(-StepX, -StepY)) + texture2D(texture, gl_TexCoord[0].st + vec2(+StepX, +StepY)) / 2.0f;
	
	c *= gl_Color;
	
	c.g -= (c.r + c.b) / 1.3f;
	c.r -= (c.g + c.b) / 1.3f;
	c.b -= (c.g + c.r) / 1.3f;
	
	c.r = clamp(c.r, 0.0f, 0.5f);
	c.g = clamp(c.g, 0.0f, 0.5f);
	c.b = clamp(c.b, 0.0f, 0.1f);
	
	a = step(0.7f, a);
	c.a = a * gl_Color.w;

	gl_FragColor = c;
}