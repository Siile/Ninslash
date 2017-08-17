#ifndef GAME_SERVER_ENTITIES_WEAPONS_GRENADELAUNCHER_H
#define GAME_SERVER_ENTITIES_WEAPONS_GRENADELAUNCHER_H

#include <game/server/entities/weapon.h>

class CGrenadelauncher : public CWeapon
{
public:
	CGrenadelauncher(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
