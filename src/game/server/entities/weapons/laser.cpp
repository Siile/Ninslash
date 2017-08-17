#include <game/server/gamecontext.h>
#include "laser.h"

CLaserrifle::CLaserrifle(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_LASER;
	Reset();
}

void CLaserrifle::Reset()
{
	if (m_Type == W_LASER)
		CWeapon::Reset();
}


void CLaserrifle::CreateProjectile()
{
	vec2 ProjStartPos = m_Pos+m_Direction*m_ProximityRadius*0.75f + vec2(0, -11);
	GameServer()->CreateProjectile(m_Owner, m_Type, m_PowerLevel, ProjStartPos, m_Direction);
}


void CLaserrifle::Tick()
{
	CWeapon::Tick();
}