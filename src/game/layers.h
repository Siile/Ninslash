

#ifndef GAME_LAYERS_H
#define GAME_LAYERS_H

#include <engine/map.h>
#include <game/mapitems.h>

class CLayers
{
	int m_GroupsNum;
	int m_GroupsStart;
	int m_LayersNum;
	int m_LayersStart;
	CMapItemGroup *m_pGameGroup;
	CMapItemLayerTilemap *m_pGameLayer;
	class IMap *m_pMap;
	
	// MapGen: Direct layer access
	int m_GameGroupIndex;
	int m_GameLayerIndex;
	int m_BackgrounLayerIndex;
	int m_ForegroundLayerIndex;
	CMapItemLayerTilemap *m_pBackgrounLayer;
	CMapItemLayerTilemap *m_pForegroundLayer;

	// for generating layers
	class CTile *m_pTiles;

	//class CGenerator *m_pGenerator;
	
	void GenerateLayers();
	
public:
	CLayers();
	void Init(class IKernel *pKernel);
	int NumGroups() const { return m_GroupsNum; };
	class IMap *Map() const { return m_pMap; };
	CMapItemGroup *GameGroup() const { return m_pGameGroup; };
	CMapItemLayerTilemap *GameLayer() const { return m_pGameLayer; };
	CMapItemGroup *GetGroup(int Index) const;
	CMapItemLayer *GetLayer(int Index) const;

	// MapGen: Direct layer access
	int GetGameGroupIndex() const { return m_GameGroupIndex; }
	int GetGameLayerIndex() const { return m_GameLayerIndex; }
	CMapItemLayerTilemap *BackgroundLayer() const { return m_pBackgrounLayer; };
	int GetBackgroundLayerIndex() const { return m_BackgrounLayerIndex; }
	CMapItemLayerTilemap *ForegroundLayer() const { return m_pForegroundLayer; };
	int GetForegroundLayerIndex() const { return m_ForegroundLayerIndex; }
};

#endif
