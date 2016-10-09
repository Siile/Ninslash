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
	float g = 0.7f;

	float StepY = 2.0f / screenheight;
	float SumGreen = (texture2D(texture, gl_TexCoord[0].st + vec2(0, +StepY)).g + texture2D(texture, gl_TexCoord[0].st).g)/2.0f;
	
	g = SumGreen * 0.7f;
	
	// get alpha
	float a = texture2D(texture, gl_TexCoord[0].st).g;
	if (a < 0.7f)
		a = 0.0f;
	else
		a = 1.0f;
	
	
	gl_FragColor = vec4(0, g * gl_Color.g, 0, a * gl_Color.w);
	
	//vec2 uv = (gl_FragCoord.xy + vec2(camerax, cameray)) / vec2(screenwidth, screenheight);
	/*
	vec2 uv = vec2(g, a+texture2D(texture, gl_TexCoord[0].st + vec2(0, +StepY*10)).g);
	vec2 pos = (uv.xy-0.5);
	vec2 cir = ((pos.xy*pos.xy+sin(uv.x*18.0+intensity)/25.0*sin(uv.y*7.0+intensity*1.5)/1.0)+uv.x*sin(intensity)/16.0+uv.y*sin(intensity*1.2)/16.0);
	float circles = (sqrt(abs(cir.x+cir.y*0.5)*25.0)*5.0);
	gl_FragColor = vec4(0,0.5+(abs(sin(circles*1.0-1.0)-sin(circles)))*0.1,0, a * gl_Color.w);
	*/
}