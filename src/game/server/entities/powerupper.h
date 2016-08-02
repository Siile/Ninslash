#ifndef GAME_SERVER_ENTITIES_POWERUPPER_H
#define GAME_SERVER_ENTITIES_POWERUPPER_H

#include <game/server/entity.h>
#include <game/server/entities/building.h>


class CPowerupper : public CBuilding
{
public:
	CPowerupper(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Tick();
	virtual void Snap(int SnappingClient);
	
	int m_Item;
	int m_ItemTakenTick;
private:
};

#endif
