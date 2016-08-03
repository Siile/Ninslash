uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	float f = rand(vec2(0, gl_TexCoord[0].y)) * rand(vec2(0, gl_FragCoord.y+rnd));// * max(0.2f, 1.0f - cs*0.5);

	vec4 c1 = texture2D(texture, gl_TexCoord[0].st) * gl_Color;
	c1 *= 1.0f-intensity+rnd*0.1f;
	
	vec4 color = vec4(-f, f, -f, 0);
	gl_FragColor = c1 + color*intensity;
}