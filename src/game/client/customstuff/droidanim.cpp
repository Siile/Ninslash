#include <game/client/gameclient.h>
#include "droidanim.h"


CDroidAnim::CDroidAnim(CGameClient *pClient)
{
	m_pClient = pClient;
	Reset();
}

CDroidAnim::~CDroidAnim()
{
}
	
void CDroidAnim::Reset()
{
	m_Status = 0;
	
	for (int i = 0; i < NUM_DROID_VALUE; i++)
		m_aValue[i] = 0.0f;
	
	for (int i = 0; i < NUM_DROID_VECTOR_VALUE; i++)
		m_aVectorValue[i] = vec2(0, 0);
	
	m_Dir = 1;
	m_Pos = vec2(0, 0);
	m_Vel = vec2(0, 0);
	
	m_aLegTargetPos[0] = vec2(-50, 30);
	m_aLegTargetPos[1] = vec2(-30, 40);
	m_aLegTargetPos[2] = vec2(30, 40);
	m_aLegTargetPos[3] = vec2(50, 30);
	
	for (int i = 0; i < 4; i++)
	{
		m_aLegAngle[i] = 0.0f;
		m_aLegTargetAngle[i] = 0.0f;
		m_aLegVel[i] = vec2(0, 0);
		m_aLegPos[i] = vec2(0, 0);
	}
	
	m_Anim = DROIDANIM_IDLE;
	m_Angle = 0;
	m_DisplayAngle = 0;
	m_TargetDisplayAngle = 0;
}


void CDroidAnim::Tick()
{
	if (!m_pClient)
		return;
	
	/*
	m_aLegTargetPos[0] = m_Pos + vec2(-50, 120);
	m_aLegTargetPos[1] = m_Pos + vec2(-30, 140);
	m_aLegTargetPos[2] = m_Pos + vec2(30, 140);
	m_aLegTargetPos[3] = m_Pos + vec2(50, 120);
	*/
	
	
	const float la = m_Angle*1.4f;
	const float la2 = m_Angle*1.4f+pi;
	const float la3 = m_Angle*2.7f;
	const float la4 = m_Angle*2.7f+pi;
				
	const float OffX1 = -cos(la)*30;
	const float OffY1 = sin(la)*50;
	const float OffX2 = -cos(la2)*30;
	const float OffY2 = sin(la2)*50;
	
	if (m_Anim == DROIDANIM_IDLE)
	{
		
		m_aLegTargetPos[0] = m_Pos + vec2(-50+OffX1, 64+OffY1);
		m_aLegTargetPos[1] = m_Pos + vec2(-30+OffX2, 64+OffY2);
		m_aLegTargetPos[2] = m_Pos + vec2(30+OffX2, 64+OffY2);
		m_aLegTargetPos[3] = m_Pos + vec2(50+OffX1, 64+OffY1);
	}
	else if (m_Anim == DROIDANIM_ATTACK)
	{
		if (m_Dir == 1)
		{
			if (m_Vel.x > 0)
			{
				const float OffX3 = -cos(la3)*64;
				const float OffY3 = sin(la3)*50;
				const float OffX4 = -cos(la4)*64;
				const float OffY4 = sin(la4)*50;
				
				m_aLegTargetPos[0] = m_Pos + vec2(-50+OffX3, 64+OffY3);
				m_aLegTargetPos[1] = m_Pos + vec2(-30+OffX4, 64+OffY4);
				m_aLegTargetPos[2] = m_Pos + vec2(20+OffX2, 64+OffY2);
				m_aLegTargetPos[3] = m_Pos + vec2(40+OffX1, 64+OffY1);
			}
			else
			{
				m_aLegTargetPos[0] = m_Pos + vec2(-40+OffX1, 64+OffY1);
				m_aLegTargetPos[1] = m_Pos + vec2(-20+OffX2, 64+OffY2);
				m_aLegTargetPos[2] = m_Pos + vec2(20+OffX2, 64+OffY2);
				m_aLegTargetPos[3] = m_Pos + vec2(40+OffX1, 64+OffY1);
			}
		}
		else
		{
			if (m_Vel.x < 0)
			{
				const float OffX3 = sin(la3)*64;
				const float OffY3 = cos(la3)*50;
				const float OffX4 = sin(la4)*64;
				const float OffY4 = cos(la4)*50;
				
				m_aLegTargetPos[0] = m_Pos + vec2(-40+OffX1, 64+OffY1);
				m_aLegTargetPos[1] = m_Pos + vec2(-20+OffX2, 64+OffY2);
				m_aLegTargetPos[2] = m_Pos + vec2(30+OffX3, 64+OffY3);
				m_aLegTargetPos[3] = m_Pos + vec2(50+OffX4, 64+OffY4);
			}
			else
			{
				m_aLegTargetPos[0] = m_Pos + vec2(-40+OffX1, 64+OffY1);
				m_aLegTargetPos[1] = m_Pos + vec2(-20+OffX2, 64+OffY2);
				m_aLegTargetPos[2] = m_Pos + vec2(20+OffX2, 64+OffY2);
				m_aLegTargetPos[3] = m_Pos + vec2(40+OffX1, 64+OffY1);
			}
		}
	}
	else if (m_Anim == DROIDANIM_JUMPATTACK)
	{
		if (m_Dir == 1)
		{
			if (m_Vel.x > 0)
			{
				const float OffX3 = -cos(la3)*60;
				const float OffY3 = sin(la3)*40;
				const float OffX4 = -cos(la4)*60;
				const float OffY4 = sin(la4)*40;
				
				m_aLegTargetPos[0] = m_Pos + vec2(-50+OffX3, 64+OffY3);
				m_aLegTargetPos[1] = m_Pos + vec2(-30+OffX4, 64+OffY4);
				m_aLegTargetPos[2] = m_Pos + vec2(20+OffX2, 64+OffY2);
				m_aLegTargetPos[3] = m_Pos + vec2(40+OffX1, 64+OffY1);
			}
			else
			{
				m_aLegTargetPos[0] = m_Pos + vec2(-40+OffX1, 64+OffY1);
				m_aLegTargetPos[1] = m_Pos + vec2(-20+OffX2, 64+OffY2);
				m_aLegTargetPos[2] = m_Pos + vec2(20+OffX2, 64+OffY2);
				m_aLegTargetPos[3] = m_Pos + vec2(40+OffX1, 64+OffY1);
			}
		}
		else
		{
			if (m_Vel.x < 0)
			{
				const float OffX3 = sin(la3)*60;
				const float OffY3 = cos(la3)*40;
				const float OffX4 = sin(la4)*60;
				const float OffY4 = cos(la4)*40;
				
				m_aLegTargetPos[0] = m_Pos + vec2(-40+OffX1, 64+OffY1);
				m_aLegTargetPos[1] = m_Pos + vec2(-20+OffX2, 64+OffY2);
				m_aLegTargetPos[2] = m_Pos + vec2(30+OffX3, 64+OffY3);
				m_aLegTargetPos[3] = m_Pos + vec2(50+OffX4, 64+OffY4);
			}
			else
			{
				m_aLegTargetPos[0] = m_Pos + vec2(-40+OffX1, 64+OffY1);
				m_aLegTargetPos[1] = m_Pos + vec2(-20+OffX2, 64+OffY2);
				m_aLegTargetPos[2] = m_Pos + vec2(20+OffX2, 64+OffY2);
				m_aLegTargetPos[3] = m_Pos + vec2(40+OffX1, 64+OffY1);
			}
		}
	}
	
	m_Angle += 0.01f*clamp(m_Vel.x, -6.0f, 6.0f);
	
	m_TargetDisplayAngle = ((m_aLegPos[2].y+m_aLegPos[3].y)-(m_aLegPos[0].y+m_aLegPos[1].y))*0.01f - m_Vel.x*0.02f;
	m_DisplayAngle -= (m_DisplayAngle - m_TargetDisplayAngle) / 6.0f;
	
	
	
	for (int i = 0; i < 4; i++)
	{
		//m_aLegAngle[i] += (i+1)*0.003f;
		
		m_aLegTargetAngle[i] = m_DisplayAngle*0.75f;
		
		if (i == 0 || i == 1)
		{
			if (m_Vel.x > 0 && m_Dir == 1 && m_Anim == DROIDANIM_JUMPATTACK)
				m_aLegTargetAngle[i] += 1.0f - (i == 0 ? sin(la3-0.2f)*1.2f : sin(la4-0.2f)*1.2f);
			else if (m_Vel.x > 0 && m_Dir == 1 && m_Anim == DROIDANIM_ATTACK)
				m_aLegTargetAngle[i] += 1.0f - (i == 0 ? sin(la3-0.2f)*1.2f : sin(la4-0.2f)*1.2f);
		}
		else
		{
			if (m_Vel.x < 0 && m_Dir == -1 && m_Anim == DROIDANIM_JUMPATTACK)
				m_aLegTargetAngle[i] -= 1.0f - (i == 2 ? cos(la3+0.2f)*1.2f : cos(la4+0.2f)*1.2f);
			else if (m_Vel.x < 0 && m_Dir == -1 && m_Anim == DROIDANIM_ATTACK)
				m_aLegTargetAngle[i] -= 1.0f - (i == 2 ? cos(la3+0.2f)*1.2f : cos(la4+0.2f)*1.2f);
		}
		
		m_aLegAngle[i] -= (m_aLegAngle[i] - m_aLegTargetAngle[i]) / 4.0f;
		
		/*
		if (distance(m_aLegPos[i], m_aLegTargetPos[i]) > 80)
		{
			//m_aLegPos[i] = m_aLegTargetPos[i];
			m_aLegPos[i] = m_aLegTargetPos[i] + normalize(m_aLegPos[i]-m_aLegTargetPos[i]) * 80.0f;
			//m_aLegVel[i] = vec2(0, 0);
		}
		*/
		
		if (abs(m_aLegPos[i].x - m_aLegTargetPos[i].x) > 120 || abs(m_aLegPos[i].y - m_aLegTargetPos[i].y) > 120)
			m_aLegPos[i] = m_aLegTargetPos[i];
		
		if (m_aLegPos[i].x - m_aLegTargetPos[i].x < -70.0f)
			m_aLegPos[i].x = m_aLegTargetPos[i].x - 70;
		if (m_aLegPos[i].x - m_aLegTargetPos[i].x > 70.0f)
			m_aLegPos[i].x = m_aLegTargetPos[i].x + 70;
		if (m_aLegPos[i].y - m_aLegTargetPos[i].y < -70.0f)
			m_aLegPos[i].y = m_aLegTargetPos[i].y - 70;
		if (m_aLegPos[i].y - m_aLegTargetPos[i].y > 70.0f)
			m_aLegPos[i].y = m_aLegTargetPos[i].y + 70;
		
		m_aLegVel[i] -= (m_aLegPos[i] - m_aLegTargetPos[i]) / 5.0f;
		//m_aLegVel[i].y += 0.05f;
		//m_aLegVel[i] += vec2(sin(m_Angle+i*0.7f), cos(m_Angle+i*0.7f)) * 6.0f;
		//m_aLegVel[i].y -= m_Vel.y*0.15f;
		m_aLegVel[i] *= 0.9f;

		if (length(m_aLegVel[i]) > 10.0f)
			m_aLegVel[i] = normalize(m_aLegVel[i]) * 10.0f;
		
		//m_aLegPos[i] += m_aLegVel[i];
	
		//Collision()->MovePoint(&m_aLegPos[i], &m_aLegVel[i], 0.0f, NULL);
		
		vec2 o = vec2(0, 0);//vec2(sin(m_aLegAngle[i]), cos(m_aLegAngle[i])) * 34.0f;
		
		vec2 p = m_aLegPos[i] + o;
		
		
		if (m_aLegVel[i].y < 0)
		{
			float VelY = m_aLegVel[i].y;
			Collision()->MoveBox(&p, &m_aLegVel[i], vec2(10, 10), 0.0f, false);
			
			if (m_aLegVel[i].y != VelY)
			{
				m_aLegVel[i].y = VelY;
				p.y += -10;//VelY*10.0f;
			}
		}
		else
			Collision()->MoveBox(&p, &m_aLegVel[i], vec2(10, 10), 0.0f, false);
		
		//Collision()->IntersectLine(m_Pos, p, NULL, &p);
		
		m_aLegPos[i] = p - o;
	}
}
