uniform sampler2D texture;
uniform float rnd;
uniform float intensity;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	// grayscale
	//float c = (texture2D(texture, gl_TexCoord[0].st).x+texture2D(texture, gl_TexCoord[0].st).y+texture2D(texture, gl_TexCoord[0].st).z)/3.0f;
	//vec4 color = vec4(c, c, c, texture2D(texture, gl_TexCoord[0].st).w);
	//gl_FragColor = color;
	
	// water tutorial
	//vec2 displacement=texture2D (u_texture_displacement, v_texCoords/6.0).xy;
	//float t= v_texCoords.y + displacement.y *0.1-0.15+ (sin (v_texCoords.x * 60.0+timedelta) * 0.005);
	//gl_FragColor = v_color * texture2D (u_texture, vec2 (v_texCoords.x, t));

	//vec4 color = vec4((texture2D(texture, gl_TexCoord[0].st).x, texture2D(texture, gl_TexCoord[0].st).y, texture2D(texture, gl_TexCoord[0].st).z, texture2D(texture, gl_TexCoord[0].st).w);
	
	// matrix like
	/*
	float cs = (texture2D(texture, gl_TexCoord[0].st).x+texture2D(texture, gl_TexCoord[0].st).y+texture2D(texture, gl_TexCoord[0].st).z);
	float f = rand(vec2(0, gl_TexCoord[0].y)) * rand(vec2(0, gl_FragCoord.y+rnd)) * max(0.0f, 1.0f - cs*2);

	vec4 color = vec4(0, f*0.5f, f, 0);
	gl_FragColor = texture2D(texture, gl_TexCoord[0].st) * gl_Color + color;
	*/

	//float cs = (texture2D(texture, gl_TexCoord[0].st).x+texture2D(texture, gl_TexCoord[0].st).y+texture2D(texture, gl_TexCoord[0].st).z);
	float f = rand(vec2(0, gl_TexCoord[0].y)) * rand(vec2(0, gl_FragCoord.y+rnd));// * max(0.2f, 1.0f - cs*0.5);

	vec4 color = vec4(-f*0.5f, f*0.5f, f, 0);
	gl_FragColor = texture2D(texture, gl_TexCoord[0].st) * gl_Color + color*intensity;
	
	/* full screen distortion
	float cs = (texture2D(texture, gl_TexCoord[0].st).x+texture2D(texture, gl_TexCoord[0].st).y+texture2D(texture, gl_TexCoord[0].st).z);
	float f = rand(vec2(0, gl_FragCoord.y+rnd));

	vec4 color = vec4(0, 0, f*0.5f, 0);
	gl_FragColor = texture2D(texture, gl_TexCoord[0].st) * gl_Color + color;
	*/
}