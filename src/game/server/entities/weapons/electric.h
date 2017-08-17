#ifndef GAME_SERVER_ENTITIES_WEAPONS_ELECTRIC_H
#define GAME_SERVER_ENTITIES_WEAPONS_ELECTRIC_H

#include <game/server/entities/weapon.h>

class CElectric : public CWeapon
{
public:
	CElectric(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
