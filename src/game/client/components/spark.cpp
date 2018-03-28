#include <base/math.h>
#include <base/vmath.h>
#include <engine/graphics.h>
#include <engine/demo.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/gamecore.h>
#include "spark.h"

CSpark::CSpark()
{
	OnReset();
	m_RenderSpark.m_pParts = this;
	m_RenderArea1.m_pParts = this;
}


void CSpark::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_SPARKS; i++)
	{
		m_aSpark[i].m_PrevPart = i-1;
		m_aSpark[i].m_NextPart = i+1;
	}

	m_aSpark[0].m_PrevPart = 0;
	m_aSpark[MAX_SPARKS-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CSpark::Add(int Group, CSinglespark *pPart)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(pInfo->m_Paused)
			return;
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
			return;
	}

	if (m_FirstFree == -1)
		return;

	// remove from the free list
	int Id = m_FirstFree;
	m_FirstFree = m_aSpark[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aSpark[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aSpark[Id] = *pPart;

	// insert to the group list
	m_aSpark[Id].m_PrevPart = -1;
	m_aSpark[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aSpark[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aSpark[Id].m_Life = 0;
}

void CSpark::Update(float TimePassed)
{
	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aSpark[i].m_NextPart;

			// move the point
			vec2 Vel = m_aSpark[i].m_Vel*TimePassed;
			
			
			m_aSpark[i].m_Pos += Vel;
			
			m_aSpark[i].m_Vel = Vel* (1.0f/TimePassed);

			m_aSpark[i].m_Life += TimePassed;
			
			
			if (abs(m_aSpark[i].m_Vel.x) + abs(m_aSpark[i].m_Vel.y) > 60.0f)
			{
				if (m_aSpark[i].m_Rotspeed == 0.0f)
					m_aSpark[i].m_Rot = GetAngle(m_aSpark[i].m_Vel);
				else
				{
					m_aSpark[i].m_Rot += TimePassed * m_aSpark[i].m_Rotspeed;
				}
			}
				
			// check death
			if(m_aSpark[i].m_Life > m_aSpark[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aSpark[i].m_PrevPart != -1)
					m_aSpark[m_aSpark[i].m_PrevPart].m_NextPart = m_aSpark[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aSpark[i].m_NextPart;

				if(m_aSpark[i].m_NextPart != -1)
					m_aSpark[m_aSpark[i].m_NextPart].m_PrevPart = m_aSpark[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aSpark[m_FirstFree].m_PrevPart = i;
				m_aSpark[i].m_PrevPart = -1;
				m_aSpark[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CSpark::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	static int64 LastTime = 0;
	int64 t = time_get();

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(!pInfo->m_Paused)
			Update((float)((t-LastTime)/(double)time_freq())*pInfo->m_Speed);
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			Update((float)((t-LastTime)/(double)time_freq()));
	}

	LastTime = t;
}

void CSpark::RenderGroup(int Group)
{
	Graphics()->BlendNormal();
	
	
	if (Group == GROUP_SPARKS)
	{
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();

		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			/*
			float a = m_aSpark[i].m_Life / m_aSpark[i].m_LifeSpan;
			vec2 p = m_aSpark[i].m_Pos;
			float Size = m_aSpark[i].m_Size;

			Graphics()->QuadsSetRotation(m_aSpark[i].m_Rot);

			Graphics()->SetColor(
				m_aSpark[i].m_Color.r,
				m_aSpark[i].m_Color.g,
				m_aSpark[i].m_Color.b,
				1.2f-a); // pow(a, 0.75f) *

			IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
			Graphics()->QuadsDraw(&QuadItem, 1);
			*/
			

			float a = 1.0f - m_aSpark[i].m_Life / m_aSpark[i].m_LifeSpan;
			vec2 p = m_aSpark[i].m_Pos;
			
			/*
			Graphics()->SetColor(
				m_aSpark[i].m_Color.r,
				m_aSpark[i].m_Color.g,
				m_aSpark[i].m_Color.b,
				1.2f-a); // pow(a, 0.75f) *
			*/
				
			IGraphics::CColorVertex aColors[4] = {
				IGraphics::CColorVertex(0, m_aSpark[i].m_Color.r, m_aSpark[i].m_Color.g, m_aSpark[i].m_Color.b, a),
				IGraphics::CColorVertex(1, m_aSpark[i].m_Color.r, m_aSpark[i].m_Color.g, m_aSpark[i].m_Color.b, a),
				IGraphics::CColorVertex(2, m_aSpark[i].m_Color.r, m_aSpark[i].m_Color.g, m_aSpark[i].m_Color.b, 0.0f),
				IGraphics::CColorVertex(3, m_aSpark[i].m_Color.r, m_aSpark[i].m_Color.g, m_aSpark[i].m_Color.b, 0.0f)};
			Graphics()->SetColorVertex(aColors, 4);
				
			vec2 r = normalize(m_aSpark[i].m_Vel);
			
			vec2 Out = vec2(r.y, -r.x) * m_aSpark[i].m_Size;
				
			IGraphics::CFreeformItem Freeform1(
					m_aSpark[i].m_Vel.x/20.0f + p.x-Out.x, m_aSpark[i].m_Vel.y/20.0f + p.y-Out.y,
					m_aSpark[i].m_Vel.x/20.0f + p.x+Out.x, m_aSpark[i].m_Vel.y/20.0f + p.y+Out.y,
					p.x-Out.x, p.y-Out.y,
					p.x+Out.x, p.y+Out.y);
			Graphics()->QuadsDrawFreeform(&Freeform1, 1);

			i = m_aSpark[i].m_NextPart;
		}
		Graphics()->QuadsEnd();
		Graphics()->BlendNormal();
	}
	else if (Group == GROUP_AREA1)
	{
		Graphics()->TextureSet(-1);

		int i = m_aFirstPart[Group];
		while(i != -1)
		{
			float a = m_aSpark[i].m_Life / m_aSpark[i].m_LifeSpan;
			
			Graphics()->ShaderBegin(SHADER_RAGE, a);
			Graphics()->QuadsBegin();
		
			vec2 p = m_aSpark[i].m_Pos;
			
			Graphics()->SetColor(1, 1, 1, 1.0f);
			
			//vec2 Out = vec2(1, 1) * m_aSpark[i].m_Size;
				
			/*
			IGraphics::CFreeformItem Freeform1(
					p.x-Out.x, p.y-Out.y,
					p.x+Out.x, p.y-Out.y,
					p.x-Out.x, p.y+Out.y,
					p.x+Out.x, p.y+Out.y);
			Graphics()->QuadsDrawFreeform(&Freeform1, 1);
			*/
			
			IGraphics::CQuadItem QuadItem(p.x, p.y, m_aSpark[i].m_Size, m_aSpark[i].m_Size);
			Graphics()->QuadsDraw(&QuadItem, 1);
			
			Graphics()->QuadsEnd();

			i = m_aSpark[i].m_NextPart;
		}
		Graphics()->BlendNormal();

		Graphics()->ShaderEnd();
	}
}
