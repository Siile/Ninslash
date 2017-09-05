#include <game/server/gamecontext.h>
#include "sweeper.h"

CSweeper::CSweeper(CGameWorld *pGameWorld)
: CWeapon(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = W_SCYTHE;
	Reset();
}

void CSweeper::Reset()
{
	if (m_Type == W_SCYTHE)
		CWeapon::Reset();
}


void CSweeper::CreateProjectile()
{
	
}


void CSweeper::Tick()
{
	CWeapon::Tick();
}