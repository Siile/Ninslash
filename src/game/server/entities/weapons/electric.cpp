#include <game/server/gamecontext.h>
#include "electric.h"

CElectric::CElectric(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_ELECTRIC;
	Reset();
}

void CElectric::Reset()
{
	if (m_Type == W_ELECTRIC)
		CWeapon::Reset();
}


void CElectric::CreateProjectile()
{
	vec2 ProjStartPos = m_Pos+m_Direction*m_ProximityRadius*0.75f + vec2(0, -11);
	GameServer()->CreateProjectile(m_Owner, m_Type, m_PowerLevel, ProjStartPos, m_Direction);
}


void CElectric::Tick()
{
	CWeapon::Tick();
}