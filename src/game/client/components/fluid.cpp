#include <base/math.h>
#include <base/vmath.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <game/generated/game_data.h>
#include <game/client/render.h>
#include <game/client/components/camera.h>
#include <game/client/components/effects.h>
#include <game/client/customstuff.h>
#include "fluid.h"


CFluid::CFluid()
{
	OnReset();
}


void CFluid::OnReset()
{
	m_PoolCount = 0;
	m_GlobalAcidLevel = 0.0f;
	
	for (int i = 0; i < MAX_POOLS; i++)
		m_aPool[i].Reset();
	
	m_GlobalPool.Reset();
}


void CFluid::Add(vec2 Pos)
{
	// check if part of pool
	for (int i = 0; i < MAX_POOLS; i++)
	{
		if (Pos.x >= m_aPool[i].m_Pos.x && Pos.y >= m_aPool[i].m_Pos.y &&
			Pos.x <= m_aPool[i].m_Pos.x+m_aPool[i].m_Size.x && Pos.y <= m_aPool[i].m_Pos.y+m_aPool[i].m_Size.y)
		{
			return;
		}
	}

	if (m_PoolCount >= MAX_POOLS)
		return;
	
	
	// scan size
	vec2 Size = vec2(0, 0);
	
	// sanity check
	int i = 0;
	
	while (Collision()->GetCollisionAt(Pos.x + Size.x, Pos.y) == CCollision::COLFLAG_DAMAGEFLUID && i++ < 100)
		Size.x += 32;
	
	i = 0;
	while (Collision()->GetCollisionAt(Pos.x, Pos.y + Size.y) == CCollision::COLFLAG_DAMAGEFLUID && i++ < 100)
		Size.y += 32;
	
	m_aPool[m_PoolCount].m_Pos = Pos;
	m_aPool[m_PoolCount].m_Size = Size;
	m_PoolCount++;
}




void CFluid::Generate()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "fluid", "Generating fluids");
	
	OnReset();
	
	for (int x = 0; x < Collision()->GetWidth(); x++)
	{
		for (int y = 0; y < Collision()->GetHeight(); y++)
		{
			if (Collision()->GetCollisionAt(x*32, y*32) == CCollision::COLFLAG_DAMAGEFLUID)
				Add(vec2(x*32, y*32));
		}
	}
}



void CFluid::AddForce(vec2 Pos, vec2 Vel)
{
	for (int i = 0; i < m_PoolCount; i++)
	{
		if (Pos.x > m_aPool[i].m_Pos.x && Pos.x < m_aPool[i].m_Pos.x + m_aPool[i].m_Size.x)
		{
			if (Pos.y > m_aPool[i].m_Pos.y - 60 && Pos.y < m_aPool[i].m_Pos.y + m_aPool[i].m_Size.y)
			{
				int x = Pos.x - m_aPool[i].m_Pos.x;
				x /= 16;
				
				if (x >= 0 && x < m_aPool[i].m_Size.x/16)
				{
					if (abs(Pos.y - m_aPool[i].m_Pos.y+m_aPool[i].m_aSurfaceY[x]) < 16)
					{
						m_aPool[i].m_aSurfaceVel[x] = (m_aPool[i].m_aSurfaceVel[x] + Vel.y) / 3.0f;
						
						/*
						if (x > 0)
							m_aPool[i].m_aSurfaceVel[x-1] = (-m_aPool[i].m_aSurfaceVel[x-1] + Vel.y) / 6.0f;
						if (x < 32*4-1)
							m_aPool[i].m_aSurfaceVel[x+1] = (-m_aPool[i].m_aSurfaceVel[x+1] + Vel.y) / 6.0f;
						*/
						
						if (abs(m_aPool[i].m_aSurfaceVel[x] - Vel.y) > 3.0f)
							m_pClient->m_pEffects->Acid(vec2(Pos.x, m_aPool[i].m_Pos.y+m_aPool[i].m_aSurfaceY[x]), vec2((frandom()-frandom())*0.4f, -frandom()*0.7f));
							
						break;
					}
				}
			}
		}
	}
	
	vec2 Center = m_pClient->m_pCamera->m_Center;
	
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	vec2 Screen = vec2(int(ScreenX1-ScreenX0), int(ScreenY1-ScreenY0));
	
	vec2 GPos = Center;
	GPos.x -= Screen.x/2;
	
	// global acid pool
	//if (Pos.x > m_GlobalPool.m_Pos.x && Pos.x < m_GlobalPool.m_Pos.x + m_GlobalPool.m_Size.x)
		
	if (m_pClient->SurvivalAcid())
	{
		if (Pos.y > m_GlobalPool.m_Pos.y - 60 && Pos.y < m_GlobalPool.m_Pos.y + m_GlobalPool.m_Size.y)
		{
			int x = Pos.x - m_GlobalPool.m_Pos.x;
			x /= 16;
			
			x = int(x+GPos.x/16)%128;
				
			//if (x >= 0 && x < m_GlobalPool.m_Size.x/16)
			{
				if (abs(Pos.y - m_GlobalPool.m_Pos.y+m_GlobalPool.m_aSurfaceY[x]) < 16)
				{
					m_GlobalPool.m_aSurfaceVel[x] = (m_GlobalPool.m_aSurfaceVel[x] + Vel.y) / 3.0f;
						
					/*
					if (x > 0)
						m_GlobalPool.m_aSurfaceVel[x-1] = (-m_GlobalPool.m_aSurfaceVel[x-1] + Vel.y) / 6.0f;
					if (x < 32*4-1)
						m_GlobalPool.m_aSurfaceVel[x+1] = (-m_GlobalPool.m_aSurfaceVel[x+1] + Vel.y) / 6.0f;
					*/
						
					if (abs(m_GlobalPool.m_aSurfaceVel[x] - Vel.y) > 3.0f)
						m_pClient->m_pEffects->Acid(vec2(Pos.x, m_GlobalPool.m_Pos.y+m_GlobalPool.m_aSurfaceY[x]), vec2((frandom()-frandom())*0.4f, -frandom()*0.7f));
				}
			}
		}
	}
}



void CFluid::OnRender()
{
	if (g_Config.m_GfxMultiBuffering)
		Graphics()->RenderToTexture(RENDERBUFFER_ACID);
	
	for (int i = 0; i < m_PoolCount; i++)
		RenderPool(i);
	
	if (m_pClient->SurvivalAcid())
		RenderGlobalAcid();
	
	Graphics()->RenderToScreen();
	
	if (Client()->State() < IClient::STATE_ONLINE)
		return;

	static int64 LastTime = 0;
	int64 t = time_get();

	if (m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
	{
		if ((float)((t-LastTime)/(double)time_freq()) > 0.01f)
		{
			Update(0);
			LastTime = t;
		}
	}

}



void CFluid::RenderGlobalAcid()
{
	float AcidLevel = Collision()->GetGlobalAcidLevel();
	
	if (AcidLevel == 0.0f)
		return;

	
	vec2 Center = m_pClient->m_pCamera->m_Center;
	
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	
	int ScreenX = int(ScreenX1-ScreenX0);
	int ScreenY = int(ScreenY1-ScreenY0);
	
	vec2 Pos = Center;
	Pos.x -= ScreenX/2;
	
	Pos.y = AcidLevel;
	
	float StepX = 16;
	vec2 Size = vec2(ScreenX+StepX*2, ScreenY);
	
	//Size.y = AcidLevel-SScreen.y;
	Size.y = (abs(Center.y)+ScreenY) - AcidLevel;
	
	m_GlobalPool.m_Pos = Pos;
	m_GlobalPool.m_Size = Size;
	
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	if (g_Config.m_GfxMultiBuffering)
		Graphics()->SetColor(1,1,1,1);
	else
		Graphics()->SetColor(0, 0.8f, 0, 0.6f);

	int offX = -fmod(abs(Center.x), 16.0f);
	
	// fullscreen fluid
	if (Center.y > AcidLevel+ScreenY)
	{
		IGraphics::CFreeformItem Freeform(
			Center.x-ScreenX/2, Center.y-ScreenY/2,
			Center.x+ScreenX/2, Center.y-ScreenY/2,
			Center.x-ScreenX/2, Center.y+ScreenY/2,
			Center.x+ScreenX/2, Center.y+ScreenY/2);
			
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	else
	{
		// fluid
		for (int f = -3; f < Size.x / 16; f++)
		{
			int a = abs(int(f+(Pos.x)/16))%128;
			
			float y1 = Pos.y + m_GlobalPool.m_aSurfaceY[a];
			float y2 = Pos.y + m_GlobalPool.m_aSurfaceY[(a+1)%128];
			/*
			IGraphics::CFreeformItem Freeform(
				Pos.x+f*StepX+offX, y1,
				Pos.x+(f+1)*StepX+offX, y2,
				Pos.x+f*StepX+offX, Pos.y+Size.y,
				Pos.x+(f+1)*StepX+offX, Pos.y+Size.y);
				*/
				
			IGraphics::CFreeformItem Freeform(
				Pos.x+f*StepX+offX, y1,
				Pos.x+(f+1)*StepX+offX, y2,
				Pos.x+f*StepX+offX, Pos.y+Size.y,
				Pos.x+(f+1)*StepX+offX, Pos.y+Size.y);
				
			Graphics()->QuadsDrawFreeform(&Freeform, 1);
		}
		
		// top outline
		if (!g_Config.m_GfxMultiBuffering)
		{
			Graphics()->SetColor(0, 0, 0, 0.4f);

			for (int f = 0; f < Size.x / 16; f++)
			{
				int a = abs(int(f+Pos.x/16))%128;
				
				float y1 = Pos.y + m_GlobalPool.m_aSurfaceY[a];
				float y2 = Pos.y + m_GlobalPool.m_aSurfaceY[(a+1)%128];
				
				IGraphics::CFreeformItem Freeform(
					Pos.x+f*StepX+offX, y1,
					Pos.x+(f+1)*StepX+offX, y2,
					Pos.x+f*StepX+offX, y1+2,
					Pos.x+(f+1)*StepX+offX, y2+2);
					
				Graphics()->QuadsDrawFreeform(&Freeform, 1);
			}
		}
	}
	
	Graphics()->QuadsEnd();
	
}


void CFluid::RenderPool(int i)
{
	vec2 Pos = m_aPool[i].m_Pos;
	vec2 Size = m_aPool[i].m_Size;
	
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	if (g_Config.m_GfxMultiBuffering)
		Graphics()->SetColor(1,1,1,1);
	else
		Graphics()->SetColor(0, 0.8f, 0, 0.6f);

	float StepX = 32/2;
	
	// fluid
	for (int f = 0; f < Size.x / 16; f++)
	{
		if (f >= 32*4-1)
			break;
		
		float y1 = Pos.y + m_aPool[i].m_aSurfaceY[f];
		float y2 = Pos.y + m_aPool[i].m_aSurfaceY[f+1];
		
		IGraphics::CFreeformItem Freeform(
			Pos.x+f*StepX, y1,
			Pos.x+(f+1)*StepX, y2,
			Pos.x+f*StepX, Pos.y+Size.y,
			Pos.x+(f+1)*StepX, Pos.y+Size.y);
			
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	
	// top outline
	if (!g_Config.m_GfxMultiBuffering)
	{
		Graphics()->SetColor(0, 0, 0, 0.4f);
		for (int f = 0; f < Size.x / 16; f++)
		{
			if (f >= 32*4-1)
				break;
			
			float y1 = Pos.y + m_aPool[i].m_aSurfaceY[f];
			float y2 = Pos.y + m_aPool[i].m_aSurfaceY[f+1];
			
			IGraphics::CFreeformItem Freeform(
				Pos.x+f*StepX, y1,
				Pos.x+(f+1)*StepX, y2,
				Pos.x+f*StepX, y1+2,
				Pos.x+(f+1)*StepX, y2+2);
				
			Graphics()->QuadsDrawFreeform(&Freeform, 1);
		}
	}
	
	Graphics()->QuadsEnd();
}


void CFluid::Update(float TimePassed)
{
	for (int i = 0; i < m_PoolCount; i++)
	{
		m_aPool[i].UpdateTension();
		m_aPool[i].UpdateSurface();
	}
	
	m_GlobalPool.UpdateTension();
	m_GlobalPool.UpdateSurface();
}



