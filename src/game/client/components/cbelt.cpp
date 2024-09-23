#include <base/math.h>
#include <base/vmath.h>
#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>

#include <generated/game_data.h>
#include <game/client/render.h>
#include <game/client/components/effects.h>
#include <game/client/customstuff.h>
#include "cbelt.h"


CCBelt::CCBelt()
{
	OnReset();
}


void CCBelt::OnReset()
{
	m_Count = 0;
	
	for (int i = 0; i < MAX_CBELTS; i++)
		m_aCBelt[i].Reset();
}


void CCBelt::Add(vec2 Pos)
{
	// check if part of 
	for (int i = 0; i < MAX_CBELTS; i++)
	{
		if (Pos.x >= m_aCBelt[i].m_Pos.x && Pos.y >= m_aCBelt[i].m_Pos.y &&
			Pos.x <= m_aCBelt[i].m_Pos.x+m_aCBelt[i].m_Size.x && Pos.y <= m_aCBelt[i].m_Pos.y+m_aCBelt[i].m_Size.y)
		{
			return;
		}
	}

	// amount limit reached
	if (m_Count >= MAX_CBELTS)
		return;
	
	
	// scan size
	vec2 Size = vec2(0, 0);
	int Dir = Collision()->IsForceTile(Pos.x, Pos.y);
	
	while (Collision()->IsForceTile(Pos.x + Size.x, Pos.y) == Dir)
		Size.x += 32;
	
	m_aCBelt[m_Count].m_Dir = Dir;
	m_aCBelt[m_Count].m_Pos = Pos;
	m_aCBelt[m_Count].m_Size = Size;
	m_Count++;
}




void CCBelt::Generate()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "cbelts", "Generating conveyor belts");
	
	OnReset();
	
	for (int x = 0; x < Collision()->GetWidth(); x++)
	{
		for (int y = 0; y < Collision()->GetHeight(); y++)
		{
			if (Collision()->IsForceTile(x*32, y*32) != 0)
				Add(vec2(x*32, y*32));
		}
	}
}





void CCBelt::OnRender()
{
	for (int i = 0; i < m_Count; i++)
		RenderCBelt(i);
	
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	static int64 LastTime = 0;
	int64 t = time_get();

	if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
	{
		if ((float)((t-LastTime)/(double)time_freq()) > 0.01f)
		{
			Update(0);
			LastTime = t;
		}
	}

}


void CCBelt::RenderCBelt(int i)
{
	vec2 Pos = m_aCBelt[i].m_Pos;
	float Width = m_aCBelt[i].m_Size.x;
	int Dir = m_aCBelt[i].m_Dir*-1;
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CBELT].m_Id);
	
	Graphics()->QuadsBegin();
	
	Graphics()->SetColor(1,1,1,1);

	float tx1 = m_aCBelt[i].m_Roll;
	float tx2 = 1.0f + (Width-128) / 128.0f + m_aCBelt[i].m_Roll;

	// y offset
	Pos.y -= 2.0f;
	
	// the belt part
	{
		Graphics()->QuadsSetSubsetFree(	tx1*Dir, 0.0f,
										tx2*Dir, 0.0f,
										tx1*Dir, 0.5f,
										tx2*Dir, 0.5f);
									
		IGraphics::CFreeformItem Freeform(
			Pos.x, Pos.y,
			Pos.x+Width, Pos.y,
			Pos.x, Pos.y + 32,
			Pos.x+Width, Pos.y + 32);
				
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	
	Pos.y -= 1.0f;
	
	// left corner
	{
		Graphics()->QuadsSetSubsetFree(	0.0f, 0.5f,
										1/4.0f, 0.5f,
										0.0f, 1.0f,
										1/4.0f, 1.0f);
		
		IGraphics::CFreeformItem Freeform(
			Pos.x-1, Pos.y,
			Pos.x-1+32, Pos.y,
			Pos.x-1, Pos.y + 32,
			Pos.x-1+32, Pos.y + 32);
				
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	
	// right corner
	{
		Graphics()->QuadsSetSubsetFree(	0.0f, 0.5f,
										1/4.0f, 0.5f,
										0.0f, 1.0f,
										1/4.0f, 1.0f);
		
		IGraphics::CFreeformItem Freeform(
			Pos.x+1+Width, Pos.y,
			Pos.x+1+Width-32, Pos.y,
			Pos.x+1+Width, Pos.y + 32,
			Pos.x+1+Width-32, Pos.y + 32);
				
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	
	
	Graphics()->QuadsEnd();
}


void CCBelt::Update(float TimePassed)
{
	for (int i = 0; i < m_Count; i++)
		m_aCBelt[i].Update();
}



