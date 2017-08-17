#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform float colorswap;
uniform float weaponcharge;

void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	float r = c.r; float b = c.b; float g = c.g;

	float swap2 = (0.5f - abs(0.5f - colorswap))*2.0f;
	c.r = r*(1.0f-swap2) + g*swap2;
	c.b = b*(1.0f-swap2) + g*swap2;
	c.g = g*(1.0f-swap2) + b*swap2;
	
	float swap1 = colorswap * (1.0f - swap2);
	r = c.r; b = c.b;
	
	c.r = r*(1.0f-swap1) + b*swap1;
	c.b = b*(1.0f-swap1) + r*swap1;
	
	//gl_FragColor = c * gl_Color;
	
	// weapon charge
	vec4 c2 = c;
	float v = max(1.0f - (c2.r + c2.g + c2.b) / 1.5f, -2.0f);
	c2.b += v * max(weaponcharge*1.2f, -2.0f);
	c2.g += v * min(weaponcharge, 0.95f);
	c2.r += v * min(weaponcharge, 0.95f);

	gl_FragColor = c * gl_Color* (1.0f - weaponcharge) + c2 * gl_Color * weaponcharge;
}