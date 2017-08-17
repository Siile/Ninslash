#ifndef GAME_SERVER_ENTITIES_WEAPONS_SHOTGUN_H
#define GAME_SERVER_ENTITIES_WEAPONS_SHOTGUN_H

#include <game/server/entities/weapon.h>

class CShotgun : public CWeapon
{
public:
	CShotgun(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
