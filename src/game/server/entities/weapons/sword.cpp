#include <game/server/gamecontext.h>
#include "sword.h"

CSword::CSword(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_SWORD;
	Reset();
}

void CSword::Reset()
{
	if (m_Type == W_SWORD)
		CWeapon::Reset();
	
	m_SwordhitTick = 0;
}


void CSword::CreateProjectile()
{
	m_SwordhitTick = 2;

	if (m_Charge >= 100)
	{
		GameServer()->TriggerSpecialAttack(m_Owner, m_Type);
		/*
		vec2 ProjPos = m_Pos+m_Direction*m_ProximityRadius*2.5f + vec2(0, -20);
		GameServer()->CreateProjectile(m_Owner, m_Type, m_PowerLevel, ProjPos, m_Direction);
		*/
	}
}


void CSword::Tick()
{
	CWeapon::Tick();
	
	if (m_SwordhitTick && --m_SwordhitTick == 0)
	{
		vec2 ProjPos = m_Pos+m_Direction*m_ProximityRadius*2.5f + vec2(0, -20);
		
		if (m_PowerLevel > 1)
			m_ReloadTimer *= 0.7f;
		
		GameServer()->CreateSwordHit(m_Owner, m_Type, m_PowerLevel, m_Pos, ProjPos);
	}
}