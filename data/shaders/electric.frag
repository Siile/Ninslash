#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

//float rand(vec2 co){
//    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
//}

highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main (void)
{
	float f = rand(vec2(0, gl_TexCoord[0].y)) * rand(vec2(0, gl_FragCoord.y+rnd));// * max(0.2f, 1.0f - cs*0.5);

	vec4 color = vec4(-f*0.5f, f*0.5f, f, 0);
	gl_FragColor = texture2D(texture, gl_TexCoord[0].st) * gl_Color + color*intensity;
}