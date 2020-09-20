#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float time;
uniform float intensity;
uniform float colorswap;
//uniform float colorswap_b;
//uniform float colorswap_g;
uniform float weaponcharge;
uniform float spawn;
uniform float visibility;
uniform float electro;
uniform float damage;
uniform float deathray;

vec2 center  = vec2(0.5f, 0.5f);

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 ColorSwap(vec2 xy, float m)
{
	return vec2(mix(xy.x, xy.y, m), mix(xy.y, xy.x, m));
}


vec4 Glow(vec4 c)
{
	vec2 p = gl_TexCoord[0].st;
	float d = abs(distance(p,center));
	float a = sin(d*10.0f - time*0.08f);
	
	float v = max(1.0f - (c.r + c.g + c.b) / 1.5f, -2.0f);
	c.b += v * sin(time*(0.085f)+d*19.0f)*(0.0f+intensity*0.35f);
	c.g += v * cos(time*(0.077f)+d*20.0f)*(0.0f+intensity*0.35f);
	c.r += v * sin(time*(0.091f)+d*21.0f)*(0.0f+intensity*0.35f);
	
	c.b += clamp(0.3f*a*weaponcharge*1.4f, -0.5f, 1.0f);
	c.g += clamp(0.3f*a*weaponcharge*1.4f, -0.5f, 1.0f);
	c.r += clamp(0.15f*a*weaponcharge*1.4f, -0.5f, 1.0f);
	
	return c;
}


void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st) * gl_Color;
	
	c = Glow(c);
	
	gl_FragColor = c;
}