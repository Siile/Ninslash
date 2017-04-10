#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

//float rand(vec2 co){
//    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
//}

float rand(vec2 co)
{
    float a = 12.9898f;
    float b = 78.233f;
    float c = 43758.5453f;
    float dt= dot(co.xy ,vec2(a,b));
    float sn= mod(dt,3.14f);
    return fract(sin(sn) * c);
}

void main (void)
{
	float f = rand(vec2(0, gl_TexCoord[0].y/4)) * rand(vec2(0, gl_FragCoord.y/4+rnd));// * max(0.2f, 1.0f - cs*0.5);

	vec4 color = vec4(-f*0.5f, f*0.5f, f, 0);
	
	float x = sin(gl_TexCoord[0].x/16.0f+intensity)*0.5f + sin(gl_TexCoord[0].y/12.0f+intensity)*0.5f;
	x = clamp(x, 0.0f, 0.75f);
	x *= rand(vec2(0, gl_TexCoord[0].y));
	x *= rnd;
	color += vec4(x*0.25f, x*0.5f, x, 0);
	
	gl_FragColor = texture2D(texture, gl_TexCoord[0].st) * gl_Color + color*intensity;
}