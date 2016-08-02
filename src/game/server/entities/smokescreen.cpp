#include <game/server/gamecontext.h>
#include "smokescreen.h"

#define RAD 0.017453292519943295769236907684886f


CSmokescreen::CSmokescreen(CGameWorld *pGameWorld, vec2 Pos, int MaxLife, int StartLife)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_SMOKESCREEN)
{
	m_ProximityRadius = ms_PhysSize;
	m_Pos = Pos;
	m_MaxLife = MaxLife;
	m_Life = StartLife;
	m_NextIn = 0;
	
	Reset();
}

void CSmokescreen::Reset()
{
	
}



void CSmokescreen::Tick()
{
	if (m_Life % 2 == 0)
	{
		if (GameServer()->Collision()->CheckPoint(m_Pos.x+32, m_Pos.y+32) == 0)
			GameServer()->CreateDeath(m_Pos+vec2(32, 32), -1);
		if (GameServer()->Collision()->CheckPoint(m_Pos.x-32, m_Pos.y-32) == 0)
		GameServer()->CreateDeath(m_Pos+vec2(-32, -32), -1);
	}
	else
	{
		if (GameServer()->Collision()->CheckPoint(m_Pos.x-32, m_Pos.y+32) == 0)
			GameServer()->CreateDeath(m_Pos+vec2(-32, 32), -1);
		if (GameServer()->Collision()->CheckPoint(m_Pos.x+32, m_Pos.y-32) == 0)
			GameServer()->CreateDeath(m_Pos+vec2(32, -32), -1);
	}
		
	m_Life++;
		
	if (m_Life > m_MaxLife)
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}
		
}
	
void CSmokescreen::TickPaused()
{
	/*
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
	*/
}

void CSmokescreen::Snap(int SnappingClient)
{
	
}
