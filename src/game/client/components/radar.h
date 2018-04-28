#ifndef GAME_CLIENT_COMPONENTS_RADAR_H
#define GAME_CLIENT_COMPONENTS_RADAR_H
#include <base/vmath.h>
#include <game/client/component.h>

class CRadar : public CComponent
{
	friend class CGameClient;
	void RenderRadar(const struct CNetObj_Radar *pCurrent, const struct CNetObj_Radar *pPrev);
	
public:
	virtual void OnRender();
};
#endif
