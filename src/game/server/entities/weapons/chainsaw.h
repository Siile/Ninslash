#ifndef GAME_SERVER_ENTITIES_WEAPONS_CHAINSAW_H
#define GAME_SERVER_ENTITIES_WEAPONS_CHAINSAW_H

#include <game/server/entities/weapon.h>

class CChainsaw : public CWeapon
{
public:
	CChainsaw(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void CreateProjectile();

private:
	int m_ChainsawTick;
};

#endif
