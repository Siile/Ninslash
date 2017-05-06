#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform int screenwidth;
uniform int screenheight;

void main (void)
{
	float StepX = 4.0 / screenwidth;
	float StepY = 4.0 / screenheight;
	const float chainsawHandleStart = 236.0 / 512.0;
	const float chainsawBladeEnd = 320.0 / 512.0;
	float gradient = clamp( 2.0 * (gl_TexCoord[0].x - chainsawHandleStart) / (chainsawBladeEnd - chainsawHandleStart), 0.0, 1.0);

	vec4 t = texture2D(texture, gl_TexCoord[0].xy);

	float nearbyAlpha =
		texture2D(texture, gl_TexCoord[0].xy + vec2(0.0, -StepY)).a +
		texture2D(texture, gl_TexCoord[0].xy + vec2(StepX, StepY)).a +
		texture2D(texture, gl_TexCoord[0].xy + vec2(-StepX, StepY)).a;
	nearbyAlpha = min(1.0, nearbyAlpha) * intensity * gradient;
	nearbyAlpha = min(1.0 - t.a, nearbyAlpha);

	gl_FragColor = vec4(min(1.0, t.r + nearbyAlpha), min(1.0, t.g + nearbyAlpha), t.b, t.a + nearbyAlpha) * gl_Color;
}
