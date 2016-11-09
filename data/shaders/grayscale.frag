#version 120

uniform sampler2D texture;
uniform float intensity;

void main (void)
{
	vec4 c = texture2D(texture, gl_TexCoord[0].st);
	float s = (c.r + c.g + c.b) / 3.0f;
	
	s = (s + (1.0f - intensity) + s * intensity) / 2.0f;
	
	gl_FragColor = vec4(s, s, s, c.a) * gl_Color;
}