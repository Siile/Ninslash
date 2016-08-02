#ifndef GAME_SERVER_ENTITIES_ELECTROMINE_H
#define GAME_SERVER_ENTITIES_ELECTROMINE_H

#include <game/server/entity.h>

//const int PickupPhysSize = 14;

class CElectromine : public CEntity
{
public:
	CElectromine(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int m_Life;
	int m_Owner;
	
	int m_ElectroTimer;

	bool m_Flashing;
	int m_FlashTimer;

private:
};

#endif
