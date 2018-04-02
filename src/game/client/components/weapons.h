#ifndef GAME_CLIENT_COMPONENTS_WEAPONS_H
#define GAME_CLIENT_COMPONENTS_WEAPONS_H
#include <base/system.h>
#include <game/client/component.h>

class CWeapons : public CComponent
{
	void RenderWeapon(const CNetObj_Weapon *pPrev, const CNetObj_Weapon *pCurrent);
	
public:
	virtual void OnReset();
	virtual void OnRender();
};

#endif
