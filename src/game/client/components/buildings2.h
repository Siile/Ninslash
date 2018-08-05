#ifndef GAME_CLIENT_COMPONENTS_BUILDINGS2_H
#define GAME_CLIENT_COMPONENTS_BUILDINGS2_H
#include <base/system.h>
#include <game/client/component.h>

// rendered over the game layer
class CBuildings2 : public CComponent
{
	void RenderLightningWall(const struct CNetObj_Building *pCurrent);
	void RenderLightningWallTop(const struct CNetObj_Building *pCurrent);
	void RenderSawblade(const struct CNetObj_Building *pCurrent);
	void RenderFlametrap(const struct CNetObj_Building *pCurrent, const struct CNetObj_Building *pPrev);
	void RenderJumppad(const struct CNetObj_Building *pCurrent);
	void RenderGenerator(const struct CNetObj_Building *pCurrent, const struct CNetObj_Building *pPrev);
	
	void RenderBuildMode();

public:
	virtual void OnReset();
	virtual void OnRender();
};

#endif
