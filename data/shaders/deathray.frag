#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	float f = rand(vec2(0, gl_FragCoord.x+rnd));

	vec4 color = vec4(1.0f, 1.0f-f*0.5f, 1.0f-f, texture2D(texture, gl_TexCoord[0].st).w);
	gl_FragColor = texture2D(texture, gl_TexCoord[0].st) * gl_Color + color*intensity*(1+rnd);
}