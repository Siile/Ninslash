#ifndef GAME_SERVER_ENTITIES_WEAPONS_LASER_H
#define GAME_SERVER_ENTITIES_WEAPONS_LASER_H

#include <game/server/entities/weapon.h>

class CLaserrifle : public CWeapon
{
public:
	CLaserrifle(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
