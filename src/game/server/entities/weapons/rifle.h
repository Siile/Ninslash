#ifndef GAME_SERVER_ENTITIES_WEAPONS_RIFLE_H
#define GAME_SERVER_ENTITIES_WEAPONS_RIFLE_H

#include <game/server/entities/weapon.h>

class CRifle : public CWeapon
{
public:
	CRifle(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
