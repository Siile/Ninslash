#include "animdata.h"


CSkeletonAnimation::CSkeletonAnimation()
{
	Reset();
}
	
void CSkeletonAnimation::Reset()
{
	m_Anim = 0;
	m_Time = 0.0f;
	m_Speed = 0.0f;
	m_Scale = vec2(0.2f, 0.2f);
	m_Flip = false;
	m_FlipBody = false;
	
	m_BodyTilt = 0.0f;
	m_HeadTilt = 0.0f;
	m_HeadTiltCorrect = 0.0f;
	
	m_Eyes = 0;
	m_ColorFeet = vec4(1, 1, 1, 1);
	m_ColorBody = vec4(1, 1, 1, 1);
	m_ColorTopper = vec4(1, 1, 1, 1);
	m_ColorSkin = vec4(1, 1, 1, 1);
	
	m_FeetPhysics = false;
	m_FeetDir = vec2(0, 0);
	m_FeetAngle = 0.0f;
	
	m_HeadOffset = vec2(0, 0);
}
	
void CSkeletonAnimation::SetAnimation(int Anim, float Speed)
{
	if (Anim == m_Anim)
	{
		m_Speed = Speed;
		return;
	}
	
	m_Anim = Anim;
	m_Time = 0.0f;
	m_Speed = Speed;
}


void CSkeletonAnimation::SetSpeed(float Speed)
{
	m_Speed = Speed;
}

void CSkeletonAnimation::Tick()
{
	m_Time += m_Speed/2.0f;
}
