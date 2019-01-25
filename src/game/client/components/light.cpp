#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/client/customstuff.h>
#include <game/client/components/camera.h>
#include <game/client/components/particles.h>
#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/gamecore.h>
#include "light.h"

CLight::CLight()
{
	OnReset();
	m_RenderLight.m_pParts = this;
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
	return;
	
	if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
		return;

	if (m_Count >= MAX_LIGHTSOURCES)
		return;
	
	m_aLightsource[m_Count++] = *pPart;
}

void CLight::Update(float TimePassed)
{
	
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


void CLight::RenderLight(vec2 Pos, vec2 Size, vec4 Color)
{
	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size.x, Size.y);
	Graphics()->QuadsDraw(&QuadItem, 1);
}


void CLight::RenderLight(ivec2 Pos)
{
	vec4 c1 = vec4(1.0f, 0.75f, 0.5f, 0.9f);
	vec4 c2 = vec4(1.0f, 0.75f, 0.5f, 0.0f);
	
	IGraphics::CColorVertex aColors[4] = {
		IGraphics::CColorVertex(0, c1.r, c1.g, c1.b, c1.a),
		IGraphics::CColorVertex(1, c1.r, c1.g, c1.b, c1.a),
		IGraphics::CColorVertex(2, c2.r, c2.g, c2.b, c2.a),
		IGraphics::CColorVertex(3, c2.r, c2.g, c2.b, c2.a)};
	
	Graphics()->SetColorVertex(aColors, 4);
	
	vec2 From1 = vec2(Pos.x-16, Pos.y);
	vec2 From2 = vec2(Pos.x+16, Pos.y);
	vec2 To1 = vec2(Pos.x-300, Pos.y+600);
	vec2 To2 = vec2(Pos.x+300, Pos.y+600);
	
	if (Collision()->IntersectLine(From1, To1, 0x0, &To1))
		To1 -= normalize(From1-To1) * 240.0f;
	if (Collision()->IntersectLine(From2, To2, 0x0, &To2))
		To2 -= normalize(From2-To2) * 240.0f;
	
	
	IGraphics::CFreeformItem FreeFormItem(
		From1.x, From1.y,
		From2.x, From2.y,
		To1.x, To1.y,
		To2.x, To2.y);
								
	Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
}


void CLight::RenderLight(vec2 Pos1, vec2 Pos2, vec2 Pos3, vec2 Pos4, vec4 Color)
{
	IGraphics::CColorVertex aColors[4] = {
		IGraphics::CColorVertex(0, Color.r, Color.g, Color.b, Color.a),
		IGraphics::CColorVertex(1, Color.r, Color.g, Color.b, Color.a),
		IGraphics::CColorVertex(2, Color.r, Color.g, Color.b, 0),
		IGraphics::CColorVertex(3, Color.r, Color.g, Color.b, 0)};
	
	Graphics()->SetColorVertex(aColors, 4);
	
	IGraphics::CFreeformItem FreeFormItem(
		Pos1.x, Pos1.y,
		Pos2.x, Pos2.y,
		Pos3.x, Pos3.y,
		Pos4.x, Pos4.y);
								
	Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		
}


void CLight::RenderGroup(int Group)
{
	if (!g_Config.m_ClLighting)
		return;
	
	if(Client()->State() < IClient::STATE_ONLINE)
		return;
	
	Graphics()->RenderToTexture(RENDERBUFFER_LIGHT);
	Graphics()->BlendAdditive();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_LIGHTS].m_Id);
	Graphics()->QuadsBegin();

	m_pClient->m_pParticles->RenderLights();
	
	// render light sources to texture buffer
	/*
	for (int i = 0; i < m_Count; i++)
	{
		vec2 p = m_aLightsource[i].m_Pos;
		float Size = m_aLightsource[i].m_Size;
		
		Graphics()->SetColor(0.4f, 0.4f, 1, 1);
			
		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	*/
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);
		
		if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const struct CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
			
			if (pBuilding->m_Type == BUILDING_SCREEN)
			{
				vec4 c;
				switch (int(pBuilding->m_X/32)%4)
				{
					case 0: c = vec4(0.5f, 0.7f, 1.0f, 1); break;
					case 1: c = vec4(0.4f, 1.0f, 0.4f, 1); break;
					case 2: c = vec4(1.0f, 0.5f, 0.5f, 1); break;
					case 3: c = vec4(0.2f, 1.0f, 1.0f, 1); break;
					default: c = vec4(0.5f, 0.7f, 1.0f, 1); break;
				}
				RenderLight(vec2(pBuilding->m_X, pBuilding->m_Y-96), vec2(500, 320), c);
			}
			else if (pBuilding->m_Type == BUILDING_REACTOR)
			{
				//float r = 1.0f + sin(CustomStuff()->m_SawbladeAngle*0.5f)*0.25f;
				RenderLight(vec2(pBuilding->m_X, pBuilding->m_Y-32), vec2(300, 420), vec4(0.25f, 0.75f, 1.0f, 1.0f));
				//RenderLight(vec2(pBuilding->m_X, pBuilding->m_Y-32), vec2(300, 420)*r, vec4(0.75f, 1.0f, 1.0f, 1.0f));
			}
		}
		
		/*
		if(Item.m_Type == NETOBJTYPE_PICKUP)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			
			if(pPrev)
			{
				const CNetObj_Pickup *pCurrent2 = (const CNetObj_Pickup *)pData;
				const CNetObj_Pickup *pPrev2= (const CNetObj_Pickup *)pPrev;
			
				const vec2 Pos = mix(vec2(pPrev2->m_X, pPrev2->m_Y), vec2(pCurrent2->m_X, pCurrent2->m_Y), Client()->IntraGameTick());
			
				RenderLight(Pos, 140.0f, vec4(1, 1, 1, 0.4f));
			}
		}
		*/
	}
	
	// camera
	RenderLight(m_pClient->m_pCamera->m_Center, vec2(900, 700), vec4(1, 1, 1, 0.4f));
	
	Graphics()->QuadsEnd();
	
	
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);
		
		if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const struct CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
			
			if (pBuilding->m_Type == BUILDING_STAND)
				RenderLight(ivec2(pBuilding->m_X, pBuilding->m_Y));
		}
		else if (Item.m_Type == NETOBJTYPE_POWERUPPER)
		{
			const struct CNetObj_Powerupper *pBuilding = (const CNetObj_Powerupper *)pData;
			
				//float r = 1.0f + sin(CustomStuff()->m_SawbladeAngle*0.5f)*0.25f;
				//RenderLight(vec2(pBuilding->m_X, pBuilding->m_Y-32), vec2(300, 420), vec4(0.25f, 0.75f, 1.0f, 1.0f));
				const vec2 p = vec2(pBuilding->m_X, pBuilding->m_Y-22);
				RenderLight(p+vec2(-10, 0), p+vec2(+10, 0), p+vec2(-40, -70), p+vec2(+40, -70), vec4(0.25f, 1.0f, 0.5f, 1));
		}
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
	Graphics()->TextureSet(-1);
}
