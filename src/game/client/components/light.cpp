#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/client/customstuff.h>
#include <game/client/components/camera.h>
#include <game/client/components/particles.h>
#include <generated/game_data.h>
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
	m_SmallCount = 0;
	m_BoxCount = 0;
}


void CLight::AddSimpleLight(vec2 Pos, vec4 Color, vec2 Size)
{
	if (!g_Config.m_ClLighting)
		return;
	
	if (Size.x <= 32 && Size.y <= 32)
	{
		if (m_SmallCount >= MAX_LIGHTSOURCES)
			return;
		
		m_aSmallSimpleLight[m_SmallCount++].Set(Pos, Color, Size);
	}
	else
	{
		if (m_Count >= MAX_LIGHTSOURCES)
			return;
		
		m_aSimpleLight[m_Count++].Set(Pos, Color, Size);
	}
}

void CLight::AddBoxLight(vec2 Pos, vec4 Color, vec2 Size, float Rot)
{
	if (!g_Config.m_ClLighting)
		return;
	
	if (m_BoxCount >= MAX_LIGHTSOURCES)
		return;
		
	m_aBoxLight[m_BoxCount++].Set(Pos, Color, Size, Rot);
}

void CLight::Update(float TimePassed)
{
	
}

void CLight::OnRender()
{
	// no updates
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
	
	CUIRect Screen;
		Graphics()->GetScreen(&Screen.x, &Screen.y, &Screen.w, &Screen.h);
	
	Graphics()->RenderToTexture(RENDERBUFFER_LIGHT);
	Graphics()->BlendAdditive();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_LIGHTS].m_Id);
	Graphics()->QuadsBegin();

	//m_pClient->m_pParticles->RenderLights();
	
	/*
	// render light sources to texture buffer
	for (int i = 0; i < m_Count; i++)
	{
		vec2 p = m_aLightsource[i].m_Pos;
		float Size = m_aLightsource[i].m_Size;
		
		Graphics()->SetColor(0.4f, 0.4f, 1, 1);
			
		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	*/
	
	// camera center light
	RenderLight(m_pClient->m_pCamera->m_Center, vec2(900, 700), vec4(1, 1, 1, 0.4f));
	
	// render light sources to texture buffer
	for (int i = 0; i < m_Count; i++)
	{
		const vec4 c = m_aSimpleLight[i].m_Color;
		Graphics()->SetColor(c.r, c.g, c.b, c.a);
			
		IGraphics::CQuadItem QuadItem(m_aSimpleLight[i].m_Pos.x, m_aSimpleLight[i].m_Pos.y, m_aSimpleLight[i].m_Size.x, m_aSimpleLight[i].m_Size.y);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SMALLLIGHT].m_Id);
	Graphics()->QuadsBegin();
	
	// render small light sources to texture buffer
	for (int i = 0; i < m_SmallCount; i++)
	{
		const vec4 c = m_aSmallSimpleLight[i].m_Color;
		Graphics()->SetColor(c.r, c.g, c.b, c.a);
			
		IGraphics::CQuadItem QuadItem(m_aSmallSimpleLight[i].m_Pos.x, m_aSmallSimpleLight[i].m_Pos.y, m_aSmallSimpleLight[i].m_Size.x, m_aSmallSimpleLight[i].m_Size.y);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BOXLIGHT].m_Id);
	Graphics()->QuadsBegin();
	
	// render box light sources to texture buffer
	for (int i = 0; i < m_BoxCount; i++)
	{
		Graphics()->QuadsSetRotation(m_aBoxLight[i].m_Rot);
		const vec4 c = m_aBoxLight[i].m_Color;
		Graphics()->SetColor(c.r, c.g, c.b, c.a);
			
		IGraphics::CQuadItem QuadItem(m_aBoxLight[i].m_Pos.x, m_aBoxLight[i].m_Pos.y, m_aBoxLight[i].m_Size.x, m_aBoxLight[i].m_Size.y);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	m_Count = 0;
	m_SmallCount = 0;
	m_BoxCount = 0;
	
	/*
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
	//}
	
	
	
	// raycasting
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
	
	vec2 Center = m_pClient->m_pCamera->m_Center / 32;
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	
	int w = int(ScreenX1-ScreenX0)/64+2;
	int h = int(ScreenY1-ScreenY0)/64+2;
	
	int x1 = int(Center.x) - w;
	int x2 = int(Center.x) + w;
	int y1 = int(Center.y) - h;
	int y2 = int(Center.y) + h;

	vec2 aLights[199];
	int LightCount = 0;
	
	// get light endpoints
	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			int t = Collision()->GetLightRay(ivec2(x*32, y*32));
			if (t > 0 || ((t == -1 || t == -3) && (y == y1 || y == y2)) || ((t == -2 || t == -3) && (x == x1 || x == x2)) || ((x == x1 || x == x2) && (y == y1 || y == y2)))
			{
				vec2 p = vec2(x, y)*32;
				
				if (t > 0)
				{
					for (int i = 0; i < 3; i++)
						if (LightCount < 199)
							aLights[LightCount++] = vec2(atan2(m_pClient->m_pCamera->m_Center.x-p.x, m_pClient->m_pCamera->m_Center.y-p.y)-(i-1)*0.025f, 0);
				}
				else
				{
					if (LightCount < 199)
						aLights[LightCount++] = vec2(atan2(m_pClient->m_pCamera->m_Center.x-p.x, m_pClient->m_pCamera->m_Center.y-p.y), 0);
				}
						

				 // render light endpoints
				 /*
				IGraphics::CFreeformItem FreeFormItem(
				m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y, m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y,
				p.x+Offset.x-2, p.y+Offset.y-2, p.x+Offset.x+2, p.y+Offset.y+2);
				Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				*/
				
			}
		}
	}
	
	// sort light array
	if (LightCount > 1)
	{
		int min;
		vec2 temp;
		
		for (int i = 0; i < LightCount - 1; i++) {
			min = i;
			for (int j = i + 1; j < LightCount; j++)
				if (aLights[j].x < aLights[min].x)
					min = j;

			temp = aLights[i];
			aLights[i] = aLights[min];
			aLights[min] = temp;
		}
	}
	
	
	// render lights
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);
	for (int i = 0; i < LightCount; i++)
	{
		vec2 p = m_pClient->m_pCamera->m_Center-vec2(sin(aLights[i].x), cos(aLights[i].x))*2000; // aLights[i].y
		
		int next = i+1;
		if (next >= LightCount)
			next = 0;
		
		vec2 p2 = m_pClient->m_pCamera->m_Center-vec2(sin(aLights[next].x), cos(aLights[next].x))*2000; // *aLights[next].y
		
		Collision()->IntersectLine(m_pClient->m_pCamera->m_Center, p, 0x0, &p, false, false, false);
		Collision()->IntersectLine(m_pClient->m_pCamera->m_Center, p2, 0x0, &p2, false, false, false);
		
		p -= vec2(sin(aLights[i].x), cos(aLights[i].x))*32;
		p2 -= vec2(sin(aLights[next].x), cos(aLights[next].x))*32;
		
		IGraphics::CColorVertex aColors[4] = {
		IGraphics::CColorVertex(0, 1.0f, 1.0f, 1.0f, 0.5f),
		IGraphics::CColorVertex(1, 1.0f, 1.0f, 1.0f, 0.5f),
		IGraphics::CColorVertex(2, 0.7f, 0.7f, 0.7f, 0.5f),
		IGraphics::CColorVertex(3, 0.7f, 0.7f, 0.7f, 0.5f)};
	
		Graphics()->SetColorVertex(aColors, 4);
		
		IGraphics::CFreeformItem FreeFormItem(
			m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y,
			m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y,
			p2.x, p2.y,
			p.x, p.y);
		
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);
		
		/*
		if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const struct CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
			
			if (pBuilding->m_Type == BUILDING_STAND)
				RenderLight(ivec2(pBuilding->m_X, pBuilding->m_Y));
		}
		else
		*/
		if (Item.m_Type == NETOBJTYPE_POWERUPPER)
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
	
	
	// reset the screen like it was before
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
}
