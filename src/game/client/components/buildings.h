#ifndef GAME_CLIENT_COMPONENTS_BUILDINGS_H
#define GAME_CLIENT_COMPONENTS_BUILDINGS_H
#include <base/system.h>
#include <game/client/component.h>

class CBuildings : public CComponent
{
	void RenderTurret(const struct CNetObj_Turret *pCurrent);
	void RenderPowerupper(const struct CNetObj_Powerupper *pCurrent);
	void RenderSawblade(const struct CNetObj_Building *pCurrent);
	void RenderMine(const struct CNetObj_Building *pCurrent);
	void RenderElectromine(const struct CNetObj_Building *pCurrent);
	void RenderBarrel(const struct CNetObj_Building *pCurrent);
	void RenderLazer(const struct CNetObj_Building *pCurrent);
	void RenderBase(const struct CNetObj_Building *pCurrent);
	void RenderStand(const struct CNetObj_Building *pCurrent);
	void RenderFlametrap(const struct CNetObj_Building *pCurrent);
	void RenderSwitch(const struct CNetObj_Building *pCurrent);
	void RenderDoor1(const struct CNetObj_Building *pCurrent);

public:
	virtual void OnReset();
	virtual void OnRender();
};

#endif
