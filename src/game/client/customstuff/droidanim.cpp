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
		m_aLegVel[i] = vec2(0, 0);
		m_aLegPos[i] = vec2(0, 0);
	}
	
	m_Angle = 0;
}


void CDroidAnim::Tick()
{
	if (!m_pClient)
		return;
	
	m_aLegTargetPos[0] = m_Pos + vec2(-50, 120);
	m_aLegTargetPos[1] = m_Pos + vec2(-30, 140);
	m_aLegTargetPos[2] = m_Pos + vec2(30, 140);
	m_aLegTargetPos[3] = m_Pos + vec2(50, 120);
	
	if (distance(m_Pos, m_aLegPos[0]) > 260.0f)
	{
		m_aLegPos[0] = m_Pos;
		m_aLegPos[1] = m_Pos;
		m_aLegPos[2] = m_Pos;
		m_aLegPos[3] = m_Pos;
	}
	
	m_Angle += 0.01f*m_Vel.x;
	
	for (int i = 0; i < 4; i++)
	{
		//m_aLegAngle[i] += (i+1)*0.003f;
		
		m_aLegVel[i] -= (m_aLegPos[i] - m_aLegTargetPos[i]) / 12.0f;
		//m_aLegVel[i].y += 0.05f;
		m_aLegVel[i] += vec2(sin(m_Angle+i*0.7f), cos(m_Angle+i*0.7f)) * 6.0f;
		m_aLegVel[i] *= 0.9f;

		if (length(m_aLegVel[i]) > 4.0f)
			m_aLegVel[i] = normalize(m_aLegVel[i]) * 4.0f;
		
		//m_aLegPos[i] += m_aLegVel[i];
	
		//Collision()->MovePoint(&m_aLegPos[i], &m_aLegVel[i], 0.0f, NULL);
		
		vec2 o = vec2(sin(m_aLegAngle[i]), cos(m_aLegAngle[i])) * 34.0f;
		
		vec2 p = m_aLegPos[i] + o;
		Collision()->MoveBox(&p, &m_aLegVel[i], vec2(10, 10), 0.0f, false);
		
		//Collision()->IntersectLine(m_Pos, p, NULL, &p);
		
		m_aLegPos[i] = p - o;
	}
}
