#ifndef GAME_SERVER_ENTITIES_SHOP_H
#define GAME_SERVER_ENTITIES_SHOP_H

#include <game/server/entity.h>
#include <game/server/entities/building.h>


class CShop : public CBuilding
{
public:
	CShop(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset();
	virtual void SurvivalReset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	
	virtual int GetItem(int Slot);
	virtual void ClearItem(int Slot);
	
private:
	int m_aItem[4];
	bool m_Autofill;
	
	void FillSlots();
};

#endif
