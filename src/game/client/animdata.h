#ifndef GAME_CLIENT_ANIMDATA_H
#define GAME_CLIENT_ANIMDATA_H

#include <base/tl/string.h>
#include <base/vmath.h>
#include <map>


enum PlayerAnimation
{
	PANIM_IDLE,
	PANIM_WALK,
	PANIM_RUN,
	PANIM_RUNBACK,
	PANIM_WALL,
	PANIM_JUMP,
	PANIM_JUMPGROUND,
	PANIM_JUMPSLIDE,
	PANIM_AIRJUMP,
	PANIM_WALLRUN,
	PANIM_WALLJUMP,
	PANIM_ROLL,
	PANIM_SLIDE,
	PANIM_SLIDEDOWN,
	PANIM_SLIDEUP,
	PANIM_SLIDERUN, // slopes
	PANIM_SLIDERUN_BACK,
	PANIM_SLIDE_FRONT,
	PANIM_SLIDE_BACK,
	NUM_PANIMS
};


const string Anim[NUM_PANIMS] = {
	"idle",
	"walk",
	"run",
	"runback",
	"wall",
	"jump",
	"jump_ground",
	"jump_slide",
	"airjump",
	"wallrun",
	"walljump",
	"roll",
	"slide",
	"slide_down",
	"slide_up",
	"sliderun",
	"sliderun_back",
	"slide_front",
	"slide_back",
};


class CSkeletonAnimation
{
public:
	float m_Speed;
	float m_Time;
	int m_Anim;
	
	float m_Mix;
	int m_NewAnim;
	
	bool m_Flip;
	
	float m_BodyTilt;
	float m_HeadTilt;
	float m_HeadTiltCorrect;
	
	
	// for roll animation
	bool m_FlipBody;
	
	int m_Eyes;
	
	vec2 m_Scale;
	
	vec4 m_ColorFeet;
	vec4 m_ColorBody;
	vec4 m_ColorTopper;
	vec4 m_ColorSkin;
	
	CSkeletonAnimation();
	
	void Reset();
	
	int GetAnimation(){ return m_Anim; }
	
	void SetAnimation(int Anim, float Speed = 0.0f);
	void SetSpeed(float Speed);
	void Tick();
};

#endif
