#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <generated/game_data.h>
#include <game/client/render.h>
#include <game/gamecore.h>
#include "splatter.h"

CSplatter::CSplatter()
{
	OnReset();
	m_RenderSplatter.m_pParts = this;
}


void CSplatter::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_SPLATTER; i++)
	{
		m_aSplatter[i].m_PrevPart = i-1;
		m_aSplatter[i].m_NextPart = i+1;
	}

	m_aSplatter[0].m_PrevPart = 0;
	m_aSplatter[MAX_SPLATTER-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CSplatter::Add(int Group, CBloodspill *pPart)
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
	m_FirstFree = m_aSplatter[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aSplatter[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aSplatter[Id] = *pPart;

	// insert to the group list
	m_aSplatter[Id].m_PrevPart = -1;
	m_aSplatter[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aSplatter[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aSplatter[Id].m_Life = 0;
}

void CSplatter::Update(float TimePassed)
{
	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aSplatter[i].m_NextPart;

			m_aSplatter[i].m_Life += TimePassed;
			m_aSplatter[i].m_Life += TimePassed * (m_aSplatter[i].m_Life / m_aSplatter[i].m_LifeSpan) * 3.0f;
				
			
			m_aSplatter[i].m_Pos.y += TimePassed*3.0f;
				
			// check blood death
			if(m_aSplatter[i].m_Life > m_aSplatter[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aSplatter[i].m_PrevPart != -1)
					m_aSplatter[m_aSplatter[i].m_PrevPart].m_NextPart = m_aSplatter[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aSplatter[i].m_NextPart;

				if(m_aSplatter[i].m_NextPart != -1)
					m_aSplatter[m_aSplatter[i].m_NextPart].m_PrevPart = m_aSplatter[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aSplatter[m_FirstFree].m_PrevPart = i;
				m_aSplatter[i].m_PrevPart = -1;
				m_aSplatter[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CSplatter::OnRender()
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

void CSplatter::RenderGroup(int Group)
{
	if (!g_Config.m_GfxMultiBuffering)
		return;
	
	Graphics()->RenderToTexture(RENDERBUFFER_SPLATTER);
	//Graphics()->BlendNormal();
	Graphics()->BlendAdditive();
	//gfx_blend_additive();
	//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SPLATTER].m_Id);
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_LIGHTS].m_Id);
	Graphics()->QuadsBegin();

	int i = m_aFirstPart[Group];
	while(i != -1)
	{
		//RenderTools()->SelectSprite(m_aSplatter[i].m_Spr);
		float a = m_aSplatter[i].m_Life / m_aSplatter[i].m_LifeSpan;
		vec2 p = m_aSplatter[i].m_Pos;
		float Size = m_aSplatter[i].m_Size;

		Graphics()->QuadsSetRotation(m_aSplatter[i].m_Rot);
		
		//Graphics()->SetColor(m_aSplatter[i].m_Color.r, m_aSplatter[i].m_Color.g, m_aSplatter[i].m_Color.b, 1.0f - a);
		
		/*
		Graphics()->SetColor(
			max(m_aSplatter[i].m_Color.r-a*0.9f, 0.15f),
			0,
			0,
			1.0f - a); //0.8f-a*0.8f); // pow(a, 0.75f) *
			*/
			
		//Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f-a);
		//Graphics()->SetColor(1.0f, 0.0f, 0.0f, 1.0f-a);
		vec4 c = m_aSplatter[i].m_Color;
		Graphics()->SetColor(c.r, c.g, c.b, 1.0f-a);

		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);

		i = m_aSplatter[i].m_NextPart;
	}
	Graphics()->QuadsEnd();
	Graphics()->BlendNormal();
	Graphics()->RenderToScreen();
	Graphics()->BlendNormal();
}
