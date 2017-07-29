#ifndef GAME_CLIENT_COMPONENTS_DROIDS_H
#define GAME_CLIENT_COMPONENTS_DROIDS_H
#include <base/system.h>
#include <game/client/component.h>

class CDroids : public CComponent
{
	void RenderWalker(const CNetObj_Droid *pPrev, const CNetObj_Droid *pCurrent, int ItemID);
	void RenderStar(const CNetObj_Droid *pPrev, const CNetObj_Droid *pCurrent, int ItemID);
	void RenderCrawler(const CNetObj_Droid *pPrev, const CNetObj_Droid *pCurrent, int ItemID);
	
public:
	virtual void OnReset();
	virtual void OnRender();
};

#endif
