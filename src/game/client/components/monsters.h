#ifndef GAME_CLIENT_COMPONENTS_MONSTERS_H
#define GAME_CLIENT_COMPONENTS_MONSTER_H
#include <base/system.h>
#include <game/client/component.h>

class CMonsters : public CComponent
{
	void RenderMonster(const CNetObj_Monster *pPrev, const CNetObj_Monster *pCurrent, int ItemID);
	
public:
	virtual void OnReset();
	virtual void OnRender();
};

#endif
