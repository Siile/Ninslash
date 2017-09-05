#ifndef GAME_SERVER_ENTITIES_WEAPONS_FLAMER_H
#define GAME_SERVER_ENTITIES_WEAPONS_FLAMER_H

#include <game/server/entities/weapon.h>

class CFlamer : public CWeapon
{
public:
	CFlamer(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

};

#endif
