#include <game/server/gamecontext.h>
#include "shotgun.h"

CShotgun::CShotgun(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_SHOTGUN;
	Reset();
}

void CShotgun::Reset()
{
	if (m_Type == W_SHOTGUN)
		CWeapon::Reset();
}


void CShotgun::CreateProjectile()
{
	vec2 ProjStartPos = m_Pos+m_Direction*m_ProximityRadius*0.75f + vec2(0, -11);
	GameServer()->CreateProjectile(m_Owner, m_Type, m_PowerLevel, ProjStartPos, m_Direction);
}


void CShotgun::Tick()
{
	CWeapon::Tick();
}