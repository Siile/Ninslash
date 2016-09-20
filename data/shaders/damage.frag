#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	//intensity = 1.0f;
	//float f = max(0, rand(vec2(gl_FragCoord.y, gl_FragCoord.x+rnd)) - rand(vec2(0, gl_FragCoord.x+rnd))*intensity);

	float csum = (texture2D(texture, gl_TexCoord[0].st).x + texture2D(texture, gl_TexCoord[0].st).y + texture2D(texture, gl_TexCoord[0].st).z);
	
	vec4 color = vec4(1, 0, 0, texture2D(texture, gl_TexCoord[0].st).w * gl_Color.w);
	vec4 c1 = texture2D(texture, gl_TexCoord[0].st) * gl_Color;

	gl_FragColor = c1 + color*intensity;
}