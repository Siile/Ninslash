#version 120

uniform sampler2D texture;
uniform float intensity;

void main (void)
{

	float r = texture2D(texture, gl_TexCoord[0].st).r;
	float g = texture2D(texture, gl_TexCoord[0].st).g;
	float b = texture2D(texture, gl_TexCoord[0].st).b;
	float a = texture2D(texture, gl_TexCoord[0].st).a;
	
	gl_FragColor = vec4(b, g, r, a) * gl_Color;
}