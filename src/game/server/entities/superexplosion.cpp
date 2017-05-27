#include <game/server/gamecontext.h>
#include "superexplosion.h"

#define RAD 0.017453292519943295769236907684886f


CSuperexplosion::CSuperexplosion(CGameWorld *pGameWorld, vec2 Pos, int Player, int Weapon, int MaxLife, int StartLife, bool IsTurret)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_SUPEREXPLOSION)
{
	m_ProximityRadius = ms_PhysSize;
	m_Pos = Pos;
	m_Player = Player;
	m_Weapon = Weapon;
	m_MaxLife = MaxLife;
	m_Life = StartLife;
	m_NextIn = 0;
	m_IsTurret = IsTurret;
	GameWorld()->InsertEntity(this);
}

void CSuperexplosion::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}



void CSuperexplosion::Tick()
{
	if (m_NextIn-- <= 0)
	{
		m_NextIn = 6;
		
		//GameServer()->CreateExplosion(m_Pos, m_Player, m_Weapon, 0, false);
		
		int PowerLevel = 0;
		
		if (m_Life == 0)
			PowerLevel = 1;
		
		GameServer()->CreateElectricExplosion(m_Pos, m_Player, m_Weapon, PowerLevel, false, m_IsTurret);
		
		m_Life++;
		
		if (m_Life > m_MaxLife)
		{
			GameServer()->m_World.DestroyEntity(this);
			return;
		}
	}
}
	
void CSuperexplosion::TickPaused()
{
	/*
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
	*/
}

void CSuperexplosion::Snap(int SnappingClient)
{
	
}
