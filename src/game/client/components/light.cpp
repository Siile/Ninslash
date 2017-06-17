#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/gamecore.h>
#include "light.h"

CLight::CLight()
{
	OnReset();
	m_RenderLight.m_pParts = this;
	m_Rendered = false;
}


void CLight::OnReset()
{
	/*
	for(int i = 0; i < MAX_LIGHTSOURCES; i++)
	{
		m_aLightsource[i].m_PrevPart = i-1;
		m_aLightsource[i].m_NextPart = i+1;
	}

	m_aLightsource[0].m_PrevPart = 0;
	m_aLightsource[MAX_LIGHTSOURCES-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
	*/
	
	m_Count = 0;
}

void CLight::Add(int Group, CLightsource *pPart)
{
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
		return;

	if (m_Count >= MAX_LIGHTSOURCES)
		return;
	
	m_aLightsource[m_Count++] = *pPart;
}

void CLight::Update(float TimePassed)
{
	if (m_Rendered)
	{
		m_Count = 0;
		m_Rendered = false;
	}
}

void CLight::OnRender()
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

void CLight::RenderGroup(int Group)
{
	if (!g_Config.m_ClLighting)
		return;
	
	Graphics()->RenderToTexture(RENDERBUFFER_LIGHT);
	Graphics()->BlendAdditive();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_LIGHTS].m_Id);
	Graphics()->QuadsBegin();

	// render light sources to texture buffer
	for (int i = 0; i < m_Count; i++)
	{
		vec2 p = m_aLightsource[i].m_Pos;
		float Size = m_aLightsource[i].m_Size;
		
		Graphics()->SetColor(0.4f, 0.4f, 1, 1);
			
		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	Graphics()->RenderToScreen();
	

	Graphics()->BlendLight();
	
	// render buffer to screen using special blending
	Graphics()->MapScreen(0,0,Graphics()->ScreenWidth(),Graphics()->ScreenHeight());
	Graphics()->TextureSet(-2, RENDERBUFFER_LIGHT);

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	{
		IGraphics::CQuadItem QuadItem(Graphics()->ScreenWidth() / 2, Graphics()->ScreenHeight() / 2, Graphics()->ScreenWidth(), -Graphics()->ScreenHeight());
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	Graphics()->BlendNormal();
	
	m_Rendered = true;
}
