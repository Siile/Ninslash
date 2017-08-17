#include <game/server/gamecontext.h>
#include "grenadelauncher.h"

CGrenadelauncher::CGrenadelauncher(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_GRENADELAUNCHER;
	Reset();
}

void CGrenadelauncher::Reset()
{
	if (m_Type == W_GRENADELAUNCHER)
		CWeapon::Reset();
}


void CGrenadelauncher::CreateProjectile()
{
	vec2 ProjStartPos = m_Pos+m_Direction*m_ProximityRadius*0.75f + vec2(0, -11);
	GameServer()->CreateProjectile(m_Owner, m_Type, m_PowerLevel, ProjStartPos, m_Direction);
}


void CGrenadelauncher::Tick()
{
	CWeapon::Tick();
}