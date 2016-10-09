#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform int screenwidth;
uniform int screenheight;

void main (void)
{
	//vec4 color = vec4(1.0f, 1.0f-f*0.5f, 1.0f-f, texture2D(texture, gl_TexCoord[0].st).w);
	float a = texture2D(texture, gl_TexCoord[0].st).r;
	
	float r = 0.7f;
	
	float StepX = 2.0f / screenwidth;
	float StepY = 2.0f / screenheight;
	
	float SumRed = (texture2D(texture, gl_TexCoord[0].st + vec2(-StepX, -StepY)).r + texture2D(texture, gl_TexCoord[0].st + vec2(+StepX, +StepY)).r) / 2.0f;
	
	r = SumRed * 0.7f;
	
	
	//r -= (1.0f-texture2D(texture, gl_TexCoord[0].st + vec2(-0.001f, -0.001f)).r)*0.4f;
	
	
	
	if (a < 0.7f)
		a = 0.0f;
	else
		a = 1.0f;
	
	vec4 color = vec4(r * gl_Color.r, 0, 0, a * gl_Color.w);
	gl_FragColor = color;
}