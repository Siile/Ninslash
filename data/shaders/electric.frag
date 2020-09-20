#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform float colorswap;
uniform float weaponcharge;

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


vec4 Electrify(vec4 c)
{
	float f = rand(vec2(int(gl_FragCoord.y/3*rnd), int(gl_FragCoord.x/3*rnd)))*rand(vec2(int(gl_FragCoord.y/3*rnd), int(gl_FragCoord.x/3*rnd)));
	float c1 = texture2D(texture, gl_TexCoord[0].st).x + texture2D(texture, gl_TexCoord[0].st).y + texture2D(texture, gl_TexCoord[0].st).z;
	c1 *= 3;
	
	float col = (1.0f - min(c1, 0.5f)) * f;
	vec4 color = vec4(-col*0.3f, col*0.9f, col, 0);
	c += color * intensity;
	
	return c;
}

void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st) * gl_Color;

	if (intensity > 0.0f)
		c = Electrify(c);
	
	gl_FragColor = c;
}