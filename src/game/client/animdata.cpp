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
	m_Scale = vec2(0.18f, 0.18f);
	m_Flip = false;
	m_FlipBody = false;
	
	m_HeadAngle = 0.0f;
	m_HeadForce = 0.0f;
	m_HeadTargetAngle = 0.0f;
	
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
	
	m_LastUpdate = 0;
}
	
void CSkeletonAnimation::HeadTick()
{
	m_HeadForce += (m_HeadTargetAngle-m_HeadAngle)/8.0f;
	m_HeadForce *= 0.95f;
	
	m_HeadAngle += (m_HeadTargetAngle-m_HeadAngle) / 20.0f;
	m_HeadAngle +=  m_HeadForce;

	//m_HeadAngle = m_HeadTargetAngle;
}


void CSkeletonAnimation::UpdateHead()
{
	int64 currentTime = time_get();
	if ((currentTime-m_LastUpdate > time_freq()) || (m_LastUpdate == 0))
		m_LastUpdate = currentTime;
		
	int step = time_freq()/60;
	
	if (step <= 0)
		step = 1;
	
	int i = 0;
	
	for (;m_LastUpdate < currentTime; m_LastUpdate += step)
	{
		HeadTick();
		
		if (i++ > 1)
		{
			m_LastUpdate = currentTime;
			break;
		}
	}
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
