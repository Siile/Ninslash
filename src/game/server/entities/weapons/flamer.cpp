#include <game/server/gamecontext.h>
#include "flamer.h"

CFlamer::CFlamer(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_FLAMER;
	Reset();
}

void CFlamer::Reset()
{
	if (m_Type == W_FLAMER)
		CWeapon::Reset();
}


void CFlamer::CreateProjectile()
{
	
}


void CFlamer::Tick()
{
	CWeapon::Tick();
}