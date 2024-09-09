

#include <engine/shared/config.h>

#include <base/math.h>
#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>

#include <game/client/customstuff.h>

#include "camera.h"
#include "controls.h"

CCamera::CCamera()
{
	m_CamType = CAMTYPE_UNDEFINED;
	m_LastUpdate = 0;
	m_Zoom = 1.0f;
	m_TargetZoom = 1.0f;
}

void CCamera::ConZoomIn(IConsole::IResult *pResult, void *pUserData)
{
	CCamera *pSelf = (CCamera *)pUserData;
	if(pSelf->m_pClient && pSelf->m_pClient->m_Snap.m_pLocalInfo && pSelf->m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_SPECTATORS)
		pSelf->m_TargetZoom = max(pSelf->m_TargetZoom-0.05f, 0.5f);
}

void CCamera::ConZoomOut(IConsole::IResult *pResult, void *pUserData)
{
	CCamera *pSelf = (CCamera *)pUserData;
	// removing zooming out due to visual issues with snapping & background layers
	if(pSelf->m_pClient && pSelf->m_pClient->m_Snap.m_pLocalInfo && pSelf->m_pClient->m_Snap.m_pLocalInfo->m_Team == TEAM_SPECTATORS)
		pSelf->m_TargetZoom = min(pSelf->m_TargetZoom+0.05f, 1.0f);
}

void CCamera::ConZoomDefault(IConsole::IResult *pResult, void *pUserData)
{
	CCamera *pSelf = (CCamera *)pUserData;
	pSelf->m_TargetZoom = 1.f;
}

void CCamera::OnInit()
{
	IConsole *pConsole = Kernel()->RequestInterface<IConsole>();
	if(pConsole)
	{
		pConsole->Register("+zoom_in", "", CFGFLAG_CLIENT, ConZoomIn, this, "Zoom in");
		pConsole->Register("+zoom_out", "", CFGFLAG_CLIENT, ConZoomOut, this, "Zoom out");
		pConsole->Register("+zoom_default", "", CFGFLAG_CLIENT, ConZoomDefault, this, "Set zoom to default");
	}
}

void CCamera::OnRender()
{
	// update camera center
	if(m_pClient->m_Snap.m_SpecInfo.m_Active && !m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
	{
		if(m_CamType != CAMTYPE_SPEC)
		{
			m_pClient->m_pControls->m_MousePos = m_PrevCenter;
			m_pClient->m_pControls->ClampMousePos();
			m_CamType = CAMTYPE_SPEC;
		}
		
		m_Center2 = m_pClient->m_pControls->m_MousePos;
			
		int64 currentTime = time_get();
		if ((currentTime-m_LastUpdate > time_freq()) || (m_LastUpdate == 0))
			m_LastUpdate = currentTime;
				
		int step = time_freq()/800;
			
		if (step <= 0)
			step = 1;
				
		int i = 0;
				
		for (;m_LastUpdate < currentTime; m_LastUpdate += step)
		{
			m_Center.x += (m_Center2.x-m_Center.x) / float(50);
			m_Center.y += (m_Center2.y-m_Center.y) / float(50);
			m_Zoom += (m_TargetZoom-m_Zoom) / float(50);
			
			if (i++ > 20)
				break;
		}
		
		CustomStuff()->m_LocalTeam = TEAM_SPECTATORS;
		
		//m_Center = m_pClient->m_pControls->m_MousePos;
	}
	else
	{
		m_TargetZoom = 1.f;
		m_Zoom = 1.f;
		
		if(m_CamType != CAMTYPE_PLAYER)
		{
			m_pClient->m_pControls->ClampMousePos();
			m_CamType = CAMTYPE_PLAYER;
		}

		vec2 CameraOffset(0, 0);

		float l = length(m_pClient->m_pControls->m_MousePos);
		if(l > 0.0001f) // make sure that this isn't 0
		{
			float DeadZone = g_Config.m_ClMouseDeadzone;
			float FollowFactor = g_Config.m_ClMouseFollowfactor/100.0f;
			float OffsetAmount = max(l-DeadZone, 0.0f) * FollowFactor;

			CameraOffset = normalize(m_pClient->m_pControls->m_MousePos)*OffsetAmount;
		}

		if(m_pClient->m_Snap.m_SpecInfo.m_Active)
			m_Center = m_pClient->m_Snap.m_SpecInfo.m_Position + CameraOffset;
		else
		{
			m_Center = m_pClient->m_LocalCharacterPos + CameraOffset;
			
			if (CustomStuff()->m_CameraShake > 0.0f)
				m_Center += vec2(frandom()-frandom(), frandom()-frandom())*CustomStuff()->m_CameraShake;
		}
	}

	m_PrevCenter = m_Center;
	
	// pass necessary data to shaders
	Graphics()->CameraToShaders(g_Config.m_GfxScreenWidth, g_Config.m_GfxScreenHeight, m_Center.x, m_Center.y);
}
