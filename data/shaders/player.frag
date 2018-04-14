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

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 ColorSwap(vec2 xy, float m)
{
	return vec2(mix(xy.x, xy.y, m), mix(xy.y, xy.x, m));
}

vec4 WeaponCharge(vec4 c)
{
	float v = max(1.0f - (c.r + c.g + c.b) / 1.5f, -2.0f);
	c.b += v * max(weaponcharge*1.2f, -2.0f);
	c.g += v * min(weaponcharge, 0.95f);
	c.r += v * min(weaponcharge, 0.95f);
	
	return c;
}

vec4 Invisibility(vec4 c)
{
	float f = rand(vec2(0, gl_TexCoord[0].y)) * rand(vec2(0, gl_FragCoord.y+rnd));

	c.r = mix(0, c.r, visibility);
	c.b = mix(0, c.b, visibility);
	c.g = mix(min(1.0f, c.g*f*2.0f), c.g, visibility);
	
	c.a *= visibility;
	
	return c;
}

vec4 Electrify(vec4 c)
{
	float f = rand(vec2(int(gl_FragCoord.y/3*rnd), int(gl_FragCoord.x/3*rnd)))*rand(vec2(int(gl_FragCoord.y/3*rnd), int(gl_FragCoord.x/3*rnd)));
	float c1 = texture2D(texture, gl_TexCoord[0].st).x + texture2D(texture, gl_TexCoord[0].st).y + texture2D(texture, gl_TexCoord[0].st).z;
	c1 *= 3;
	
	float col = (1.0f - min(c1, 0.5f)) * f;
	vec4 color = vec4(-col*0.3f, col*0.9f, col, 0);
	c += color * electro;
	
	return c;
}

vec4 Damage(vec4 c)
{
	return mix(c, vec4(1, c.g*0.5f, c.b*0.5f, c.a), damage);
}

vec4 Deathray(vec4 c)
{
	float f = rand(vec2(0, gl_FragCoord.x+rnd));
	return mix(c, vec4(1.0f, 1.0f-f*0.5f, 1.0f-f, c.a), deathray);
}

vec4 Glow(vec4 c)
{
	float a = (gl_TexCoord[0].x*250.0f-time)*(0.1f-weaponcharge*0.01f);
	float v = max(0.0f, sin(a)*sin(a)*sin(a)*1.5f-0.5f);
	
	c.g += v * 0.25f*(-weaponcharge);
	c.b += v * 0.25f*(-weaponcharge);
	return mix(c, vec4(0.0f, 1.0f, 1.0f, c.a), (1 - abs(1 - c.a*2))*v*gl_Color.a*(-weaponcharge));
}


void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st) * gl_Color;
	vec4 c2 = c;
	
	c.xy = ColorSwap(c.xy, colorswap);
	c.xz = ColorSwap(vec2(mix(c.x, c2.x, intensity), c2.z), intensity);
	
	if (weaponcharge > 0.0f)
		c = WeaponCharge(c);
	
	if (visibility < 1.0f)
		c = Invisibility(c);
	
	if (electro > 0.0f)
		c = Electrify(c);
	
	if (damage > 0.0f)
		c = Damage(c);
	
	if (deathray > 0.0f)
		c = Deathray(c);
	
	if (weaponcharge < 0.0f)
		c = Glow(c);
	
	gl_FragColor = c;
}