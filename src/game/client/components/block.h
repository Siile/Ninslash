#ifndef GAME_CLIENT_COMPONENTS_BLOCK_H
#define GAME_CLIENT_COMPONENTS_BLOCK_H
#include <base/vmath.h>
#include <game/client/component.h>

class CBlocks : public CComponent
{
	friend class CGameClient;
	
	int *m_pBlocks;
	int *m_pBlockSyncTick;
	int m_Width;
	int m_Height;
	
	void SetBlock(ivec2 Pos, int Block);
public:
	CBlocks();
	~CBlocks();

	void ResetBlocks();
	void RenderBlocks();
	
	virtual void OnInit();
	virtual void OnReset();
	virtual void OnRender();
	virtual void OnMapLoad();
};
#endif
