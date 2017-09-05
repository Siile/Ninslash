#ifndef GAME_SERVER_ENTITIES_WEAPONS_SWORD_H
#define GAME_SERVER_ENTITIES_WEAPONS_SWORD_H

#include <game/server/entities/weapon.h>

class CSword : public CWeapon
{
public:
	CSword(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

private:
	int m_SwordhitTick;
};

#endif
