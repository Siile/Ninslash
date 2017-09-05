#include <game/server/gamecontext.h>
#include "rifle.h"

CRifle::CRifle(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_RIFLE;
	Reset();
}

void CRifle::Reset()
{
	if (m_Type == W_RIFLE)
		CWeapon::Reset();
}


void CRifle::CreateProjectile()
{
	int PowerLevel = m_PowerLevel;
	
	if (m_Charge >= 100)
		PowerLevel += 3;
	
	vec2 ProjStartPos = m_Pos+m_Direction*m_ProximityRadius*0.75f + vec2(0, -11);
	GameServer()->CreateProjectile(m_Owner, m_Type, PowerLevel, ProjStartPos, m_Direction);
}


void CRifle::Tick()
{
	CWeapon::Tick();
}