#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float time;
uniform float intensity;
uniform float colorswap;
//uniform float colorswap_b;
//uniform float colorswap_g;
uniform float weaponcharge;
uniform float zombie;
uniform float spawn;
uniform float visibility;
uniform float electro;
uniform float damage;
uniform float deathray;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
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

// weapon level up effect
vec4 Glow(vec4 c)
{
	float a = (gl_TexCoord[0].x*250.0f-time)*(0.1f-weaponcharge*0.01f);
	float v = max(0.0f, sin(a)*sin(a)*sin(a)*1.5f-0.5f);
	
	c.g += v * 0.25f*(-weaponcharge);
	c.b += v * 0.25f*(-weaponcharge);
	
	if (weaponcharge < -1.0f)
	{
		float a2 = (gl_TexCoord[0].x*100.0f+time*0.2f)*(0.1f-weaponcharge*0.01f);
		float v2 = max(0.0f, sin(a2)*sin(a2)*1.5f-0.5f);
		v2 *= v2;
		c.r += v2 * 0.2f*weaponcharge;
		c.g += v2 * 0.2f*weaponcharge;
		c.b += v2 * 0.2f*weaponcharge;
	}
	
	return mix(c, vec4(0.0f, 1.0f, 1.0f, c.a), (1 - abs(1 - c.a*2))*v*gl_Color.a*(-weaponcharge));
}

vec4 Zombie(vec4 c)
{
	float z = 1.0f;
	/*
	float a = (gl_TexCoord[0].x*250.0f-time)*(0.1f-z*0.01f);
	float v = max(0.0f, sin(a)*sin(a)*sin(a)*1.5f-0.5f);
	
	c.g += v * 0.25f*(-z);
	c.b += v * 0.25f*(-z);
	return mix(c, vec4(0.0f, 1.0f, 1.0f, c.a), (1 - abs(1 - c.a*2))*v*gl_Color.a*(-zombie));*/
	/*
	float a = (gl_TexCoord[0].y*(25.0f+cos(time*0.01f)*2.0f) * gl_TexCoord[0].x*(25.0f+sin(time*0.012f)*2.0f)-time*0.2f)*(0.1f-z*0.01f);
	float v = abs(sin(a)*sin(a)*0.9f);
	
	c.r += v * 0.25f*(-z);
	c.g += v * 0.1f*(-z);
	c.b += v * 0.15f*(-z);
	
	return mix(c, vec4(1.0f, 1.0f, 1.0f, c.a), (1 - abs(1 - c.a*2))*v*gl_Color.a*(-zombie));
	*/
	
	float c1 = texture2D(texture, gl_TexCoord[0].st).x + texture2D(texture, gl_TexCoord[0].st).y + texture2D(texture, gl_TexCoord[0].st).z + (1.0f-texture2D(texture, gl_TexCoord[0].st).a);
	float c2 = max(0.0f, 1.0f - c1*2.0f)*z;
	
	
	//return c*vec4(0.4f+sin(time*0.01f)*0.2f, 0.85f+sin(time*0.012f)*0.15f, 0.4f+sin(time*0.01f)*0.2f, 1.0f);
	
	return mix(c*vec4(0.4f+sin(time*0.01f)*0.2f, 0.85f+sin(time*0.012f)*0.15f, 0.4f+sin(time*0.01f)*0.2f, 1.0f), vec4(0, 0.2f + sin(time*0.021f)*0.2f, 0, 1.0f), c2);
}


void main (void)
{
	//zombie = 1.0f;
	
	vec4 c = texture2D(texture, gl_TexCoord[0].st) * gl_Color;
	vec4 c2 = c;
	
	c.xy = ColorSwap(c.xy, colorswap);
	c.xz = ColorSwap(vec2(mix(c.x, c2.x, intensity), c2.z), intensity);
	
	//if (zombie > -10.0f)
	//	c = Zombie(c);
	
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