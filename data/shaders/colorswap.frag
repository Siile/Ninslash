#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform float colorswap;
uniform float colorswap_b;
uniform float colorswap_g;
uniform float weaponcharge;

vec2 ColorSwap(vec2 xy, float m)
{
	return vec2(mix(xy.x, xy.y, m), mix(xy.y, xy.x, m));
}

void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	
	vec4 c2 = c;
	
	c.xy = ColorSwap(c2.xy, colorswap_b);
	c.xz = ColorSwap(c2.xz, colorswap_g);
	gl_FragColor = c * gl_Color;
	
	
	/*
	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	float r = c.r; float b = c.b; float g = c.g;

	float swap2 = (0.5f - abs(0.5f - colorswap))*2.0f;
	c.r = r*(1.0f-swap2) + g*swap2;
	c.b = b*(1.0f-swap2) + r*swap2;
	c.g = g*(1.0f-swap2) + b*swap2;
	
	float swap1 = colorswap * (1.0f - swap2);
	r = c.r; b = c.b; g = c.g;
	
	c.r = r*(1.0f-swap1) + b*swap1;
	c.b = b*(1.0f-swap1) + r*swap1;
	c.g = g*(1.0f-swap1) + r*swap1;
	
	//gl_FragColor = c * gl_Color;
	
	// weapon charge
	vec4 c2 = c;
	float v = max(1.0f - (c2.r + c2.g + c2.b) / 1.5f, -2.0f);
	c2.b += v * max(weaponcharge*1.2f, -2.0f);
	c2.g += v * min(weaponcharge, 0.95f);
	c2.r += v * min(weaponcharge, 0.95f);

	gl_FragColor = c * gl_Color* (1.0f - weaponcharge) + c2 * gl_Color * weaponcharge;
	*/
}