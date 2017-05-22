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
	float f = rand(vec2(0, gl_FragCoord.x+rnd));

	vec4 color = vec4(1.0f, 1.0f-f*0.5f, 1.0f-f, texture2D(texture, gl_TexCoord[0].st).w);


	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	float r = c.r;
	float b = c.b;
	c.r = r*(1-colorswap) + b*colorswap;
	c.b = b*(1-colorswap) + r*colorswap;
	
	gl_FragColor = c * gl_Color + color*intensity*(1+rnd);
}