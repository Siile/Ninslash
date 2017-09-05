#ifndef GAME_SERVER_ENTITIES_WEAPONS_SWEEPER_H
#define GAME_SERVER_ENTITIES_WEAPONS_SWEEPER_H

#include <game/server/entities/weapon.h>

class CSweeper : public CWeapon
{
public:
	CSweeper(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
