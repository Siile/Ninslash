uniform float rnd;
uniform float intensity;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
	
	
	float f = rand(vec2(0, gl_TexCoord[0].y+rnd));
	gl_Position.x += f*0.015f;
}