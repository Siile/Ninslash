#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform int screenwidth;
uniform int screenheight;

void main (void)
{
	float StepX = 2.0f / screenwidth;
	float StepY = 2.0f / screenheight;
	
	vec4 c = (texture2D(texture, gl_TexCoord[0].st + vec2(-StepX, -StepY)) + texture2D(texture, gl_TexCoord[0].st + vec2(+StepX, +StepY))) / 2.0f;
	
	float a = clamp(c.r + c.g + c.b, 0.0f, 1.0f);
	
	c *= gl_Color;
	
	vec4 c2 = c;
	
	c.g -= (c2.r + c2.b) / 1.3f;
	c.r -= (c2.g + c2.b) / 1.3f;
	c.b -= (c2.g + c2.r) / 1.3f;
	
	c.r = clamp(c.r, 0.0f, 0.5f);
	c.g = clamp(c.g, 0.0f, 0.4f);
	c.b = clamp(c.b, 0.0f, 0.1f);
	
	a = step(0.3f, a);
	c.a = a * gl_Color.w;

	gl_FragColor = c;
}