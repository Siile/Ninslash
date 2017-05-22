#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform float colorswap;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	float f = rand(vec2(int(gl_FragCoord.y/3), int(gl_FragCoord.x/3)+rnd))*rand(vec2(int(gl_FragCoord.y/2), int(gl_FragCoord.x/2)+rnd))*rand(vec2(int(gl_FragCoord.y/2), int(gl_FragCoord.x/2)+rnd));

	float c1 = texture2D(texture, gl_TexCoord[0].st).x + texture2D(texture, gl_TexCoord[0].st).y + texture2D(texture, gl_TexCoord[0].st).z;
	c1 *= 3;
	
	vec4 color = vec4((1.0f - min(c1, 1.0f)) * f, (1.0f - min(c1, 1.0f)) * f, 0, 0);
	
	float x = sin(gl_TexCoord[0].x*16.0f+intensity)*0.5f + sin(gl_TexCoord[0].y*12.0f+intensity)*0.5f;
	x = clamp(x, 0.0f, 0.75f);
	x *= rand(vec2(0, gl_TexCoord[0].y));
	x *= rnd;
	color += vec4(x, x, x*0.5f, 0);
	
	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	
	float r = c.r;
	float b = c.b;
	c.r = r*(1-colorswap) + b*colorswap;
	c.b = b*(1-colorswap) + r*colorswap;
	
	gl_FragColor = c * gl_Color + color*intensity;
}