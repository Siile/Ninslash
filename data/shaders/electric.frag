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

void main (void)
{
	float f = rand(vec2(0, gl_TexCoord[0].y/4)) * rand(vec2(0, gl_FragCoord.y/4+rnd));// * max(0.2f, 1.0f - cs*0.5);

	vec4 color = vec4(-f*0.5f, f*0.5f, f, 0);
	
	float x = sin(gl_TexCoord[0].x/16.0f+intensity)*0.5f + sin(gl_TexCoord[0].y/12.0f+intensity)*0.5f;
	x = clamp(x, 0.0f, 0.75f);
	x *= rand(vec2(0, gl_TexCoord[0].y));
	x *= rnd;
	color += vec4(x*0.25f, x*0.5f, x, 0);
	
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
	
	// weapon charge
	vec4 c2 = c;
	float v = max(1.0f - (c2.r + c2.g + c2.b) / 1.5f, -2.0f);
	c2.b += v * max(weaponcharge*1.2f, -2.0f);
	c2.g += v * min(weaponcharge, 0.95f);
	c2.r += v * min(weaponcharge, 0.95f);

	gl_FragColor = c * gl_Color* (1.0f - weaponcharge) + c2 * gl_Color * weaponcharge + color*intensity;
	
	//gl_FragColor = c * gl_Color + color*intensity;
}