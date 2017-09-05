#include <game/server/gamecontext.h>
#include "chainsaw.h"

CChainsaw::CChainsaw(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_CHAINSAW;
	Reset();
}

void CChainsaw::Reset()
{
	if (m_Type == W_CHAINSAW)
		CWeapon::Reset();
	
	m_ChainsawTick = 0;
}


void CChainsaw::CreateProjectile()
{
	m_ChainsawTick = Server()->Tick() + 500 * Server()->TickSpeed()/1000;
}


void CChainsaw::Tick()
{
	CWeapon::Tick();

	if (m_ChainsawTick && m_ChainsawTick >= Server()->Tick())
	{
		vec2 ProjPos = m_Pos+m_Direction*m_ProximityRadius*2.0f + vec2(0, -11);
		
		if (m_PowerLevel > 1)
			m_ReloadTimer *= 0.7f;
		
		GameServer()->CreateChainsawHit(m_Owner, m_Type, m_PowerLevel, m_Pos, ProjPos);
	}
}