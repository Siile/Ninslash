#include <base/math.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>

#include <game/client/components/camera.h>
#include <game/client/components/effects.h>

#include <game/gamecore.h>
#include "radar.h"


void CRadar::RenderRadar(const CNetObj_Radar *pCurrent, const CNetObj_Radar *pPrev)
{
	vec2 Pos = mix(vec2(pPrev->m_TargetX, pPrev->m_TargetY), vec2(pCurrent->m_TargetX, pCurrent->m_TargetY), Client()->IntraGameTick());
	vec2 CameraPos = m_pClient->m_pCamera->m_Center;
	float a = GetAngle(Pos - CameraPos);
	
	CUIRect Screen;
	Graphics()->GetScreen(&Screen.x, &Screen.y, &Screen.w, &Screen.h);
	Graphics()->MapScreen(0,0,Graphics()->ScreenWidth(),Graphics()->ScreenHeight());
		
	vec2 SPos = vec2(cos(a), sin(a)) * (Graphics()->ScreenHeight()/2.1f);
	
	vec2 RPos = CameraPos + SPos;
	
	if (distance(RPos, CameraPos) > distance(Pos, CameraPos))
		RPos = Pos;
	
	RPos += vec2(Graphics()->ScreenWidth()/2, Graphics()->ScreenHeight()/2) -CameraPos;
	
	if ((pCurrent->m_Type == RADAR_CHARACTER || pCurrent->m_Type == RADAR_HUMAN) && abs(Pos.x - CameraPos.x) < 1000 && abs(Pos.y - CameraPos.y) < 800)
	{
	}
	else
	{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_RADAR].m_Id);
	Graphics()->QuadsBegin();
	
	float ca = min(1.0f, distance(Pos, CameraPos)*0.001f);
	
	if (pCurrent->m_Type == RADAR_REACTOR)
		ca *= 0.5f;
	
	Graphics()->SetColor(1, 1, 1, ca);
	if (pCurrent->m_Type == RADAR_CHARACTER || pCurrent->m_Type == RADAR_HUMAN)
		Graphics()->QuadsSetRotation(a);
	else
		Graphics()->QuadsSetRotation(0);
	
	RenderTools()->SelectSprite(SPRITE_RADAR1+pCurrent->m_Type);
	RenderTools()->DrawSprite(RPos.x, RPos.y, 80);
	
	Graphics()->QuadsEnd();
	}
	
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
}


void CRadar::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if (Item.m_Type == NETOBJTYPE_RADAR)
		{
			const struct CNetObj_Radar *pCurrent = (const CNetObj_Radar *)pData;
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			
			RenderRadar(pCurrent, pPrev ? (const CNetObj_Radar *)pPrev : pCurrent);
		}
	}
}