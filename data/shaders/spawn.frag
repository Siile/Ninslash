uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	//intensity = 1.0f;
	float f = max(0, rand(vec2(gl_FragCoord.y, gl_FragCoord.x+rnd)) - rand(vec2(0, gl_FragCoord.x+rnd))*intensity);

	vec4 color = vec4(0, f, 0, texture2D(texture, gl_TexCoord[0].st).w * gl_Color.w * f);
	
	vec4 c1 = texture2D(texture, gl_TexCoord[0].st) * gl_Color;
	
	//c1.w *= 1.0f-intensity;
	//c1.w -= gl_FragCoord.y*0.1f;
	
	gl_FragColor = c1*(1-intensity*2) + color*intensity*2;
}