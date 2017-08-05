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

vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
				   dot(p,vec2(269.5,183.3)), 
				   dot(p,vec2(419.2,371.9)) );
	return fract(sin(q)*43758.5453);
}

float iqnoise( in vec2 x, float u, float v )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
		
	float k = 1.0+63.0*pow(1.0-v,4.0);
	
	float va = 0.0;
	float wt = 0.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = vec2( float(i),float(j) );
		vec3 o = hash3( p + g )*vec3(u,u,1.0);
		vec2 r = g - f + o.xy;
		float d = dot(r,r);
		float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
		va += o.z*ww;
		wt += ww;
    }
	
    return va/wt;
}

vec3 color(vec2 pt, float size) {
    float rInv = min(size, intensity*size/40.0f)/length(pt);
    pt = pt * rInv - vec2(rInv+2.*mod(intensity,6000.),0.0);
	
	float v = min(0.5f, 0.0f+intensity*0.05f);
	
	return vec3(iqnoise(0.5f*pt, 2.0f, 1.0f)+0.240*rInv);
}

void main (void)
{
	vec2 scale = vec2(35.0f, 27.0f);
	vec3 c = color(vec2(gl_FragCoord.x*4/scale.x-screenwidth/(scale.x*2), gl_FragCoord.y*4/scale.y-screenheight/(scale.y*2)), screenwidth/50);
	
	gl_FragColor = vec4(0, 0.4f-c.g*0.4f, 1.0f-c.b*1.0f, 0.6f) * gl_Color;	
}