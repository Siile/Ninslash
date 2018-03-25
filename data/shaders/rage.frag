#version 120

uniform sampler2D texture;
uniform float rnd;
uniform float intensity;
uniform float colorswap;
uniform int screenwidth;
uniform int screenheight;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


vec2  center  = vec2(0.5f, 0.5f);
float border  = 0.16f;
float blur    = 0.02f;

void main( void ) {
	float radius = intensity*1.0f;

	vec2 p = gl_TexCoord[0].st;
	float a = abs(distance(p,center));

	float t = 1. - smoothstep(radius - border, radius - border + blur, a) + smoothstep(radius - blur, radius, a);
	float c = mix(1.0f, 0.0f, t);
	
	
	//float f = min(1.0f, (rand(vec2(0, int(gl_FragCoord.x/8)*8)) * rand(vec2(0, int(gl_FragCoord.y/8)*8)) * a)*2.0f);
	//float f = rand(vec2(int(gl_FragCoord.y/5), int(gl_FragCoord.x/3)))*rand(vec2(int(gl_FragCoord.y/5), int(gl_FragCoord.x/3)));

	
	gl_FragColor = vec4((1.0f-a*2.0f)/2.0f, c, c, max(0.0f, 0.75f-a*1.5f) * c);
}


/*
float Hash( vec2 p)
{
     vec3 p2 = vec3(p.xy,1.0);
    return fract(sin(dot(p2,vec3(37.1,61.7, 12.4)))*3758.5453123);
}

float noise(in vec2 p)
{
    vec2 i = floor(p);
     vec2 f = fract(p);
     f *= f * (3.0-2.0*f);

    return mix(mix(Hash(i + vec2(0.,0.)), Hash(i + vec2(1.,0.)),f.x),
               mix(Hash(i + vec2(0.,1.)), Hash(i + vec2(1.,1.)),f.x),
               f.y);
}

float fbm(vec2 p)
{
     float v = 0.0;
     v += noise(p*1.0)*.5;
     v += noise(p*2.)*.25;
     v += noise(p*4.)*.125;
     return v;
}

void main( void ) 
{

	vec2 uv = gl_TexCoord[0].st*2.0f;
	uv -= vec2(1.0f, 1.0f);
	
	float timeVal = intensity*0.1;
	vec3 finalColor = vec3( 0.0 );
	for( int i=0; i < 10; ++i )
	{
		float indexAsFloat = float(i);
		float amp = 10.0 + (indexAsFloat*5.0);
		float period = 10.0 + (indexAsFloat+2.0);
		float thickness = mix( 0.7, 1.0, noise(uv*0.0) );
		float t = 0.3*abs( 1.0 / (sin(uv.x + fbm( uv + timeVal * period )) * amp) * thickness );
		float show = fract(abs(sin(timeVal))) >= 0.9 ? 1.0 : 0.0;
		show = 1.50;
		finalColor +=  t * vec3( 0.3, 0.5, 2.0 ) * show;
	}
	
	gl_FragColor = vec4( finalColor, 1.0 );

}
*/