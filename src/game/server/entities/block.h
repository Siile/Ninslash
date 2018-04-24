#ifndef GAME_SERVER_ENTITIES_BLOCK_H
#define GAME_SERVER_ENTITIES_BLOCK_H

#include <game/server/entity.h>

const int BlockPhysSize = 32;

class CBlock : public CEntity
{
public:
	CBlock(CGameWorld *pGameWorld, int Type, vec2 Pos);

	virtual void Reset();
	virtual void Tick();
	virtual void SurvivalReset();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int m_Type;
	int m_Life;
	int m_MaxLife;
	int m_DestroyTick;
	
	bool m_aSnapped[MAX_CLIENTS];
	int m_aSnapTimer[MAX_CLIENTS];
	
	void Sync();
	void Destroy();

	void TakeDamage(int Damage);
	
private:
};

#endif
