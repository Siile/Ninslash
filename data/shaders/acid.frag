#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

uniform int screenwidth;
uniform int screenheight;
uniform int camerax;
uniform int cameray;

void main (void)
{
	float g = 0.5f;

	float StepY = 2.0f / screenheight;
	float SumGreen = (texture2D(texture, gl_TexCoord[0].st + vec2(0, +StepY)).g + texture2D(texture, gl_TexCoord[0].st).g)/2.0f;
	
	g = SumGreen * 0.7f;
	
	// get alpha
	float a = texture2D(texture, gl_TexCoord[0].st).g;
	a = step(0.7f, a);
	
	float AR = (float(screenheight) / screenwidth)*2.0f;
	float S = screenwidth / 2 * AR;
	
	// works
	//g += sin((gl_FragCoord.y-cameray*AR)*0.2f)*0.2f;
	
	//g += sin((gl_FragCoord.x+camerax*AR)*0.112793f)*0.2f * sin((gl_FragCoord.y-cameray*AR)*0.07325f)*0.2f;
	
	//g += cos((gl_FragCoord.y+cameray)*0.05f)*0.2f;
	
	gl_FragColor = vec4(0, g * gl_Color.g, 0, a * gl_Color.w);
}