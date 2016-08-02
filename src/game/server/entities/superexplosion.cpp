#include <game/server/gamecontext.h>
#include "superexplosion.h"

#define RAD 0.017453292519943295769236907684886f


CSuperexplosion::CSuperexplosion(CGameWorld *pGameWorld, vec2 Pos, int Player, int Weapon, int MaxLife, int StartLife, bool Superdamage)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_SUPEREXPLOSION)
{
	m_ProximityRadius = ms_PhysSize;
	m_Pos = Pos;
	m_Player = Player;
	m_Weapon = Weapon;
	m_MaxLife = MaxLife;
	m_Life = StartLife;
	m_NextIn = 0;
	
	m_Superdamage = Superdamage;
	
	Reset();
}

void CSuperexplosion::Reset()
{
	
}



void CSuperexplosion::Tick()
{
	if (m_NextIn-- <= 0)
	{
		m_NextIn = 2;
		
		if (m_Superdamage)
			GameServer()->CreateSoundGlobal(SOUND_GRENADE_EXPLODE, -1);
		else
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		
		if (m_Life == 0)
			GameServer()->CreateExplosion(m_Pos, m_Player, m_Weapon, false, m_Superdamage);
		else
		{
			int Steps = m_Life * 4;
			float StepAngle = 360 / float(Steps);
			float Angle = StepAngle / 2.0f;
			
			for (int i = 0; i < Steps; i++)
			{
				vec2 Dir = vec2(cosf(Angle*RAD), sinf(Angle*RAD));
				vec2 To = m_Pos + Dir * m_Life*64;

				if(!GameServer()->Collision()->IntersectLine(m_Pos + Dir * 48, To, NULL, NULL))
				{
					GameServer()->CreateExplosion(To, m_Player, m_Weapon, false, m_Superdamage);
				}
				
				Angle += StepAngle;
			}
		}
		
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
