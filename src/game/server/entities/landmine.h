#ifndef GAME_SERVER_ENTITIES_LANDMINE_H
#define GAME_SERVER_ENTITIES_LANDMINE_H

#include <game/server/entity.h>

const int PickupPhysSize = 14;

class CLandmine : public CEntity
{
public:
	CLandmine(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int m_Life;
	int m_Owner;

	bool m_Flashing;
	int m_FlashTimer;

private:
};

#endif
