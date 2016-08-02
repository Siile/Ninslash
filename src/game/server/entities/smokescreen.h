#ifndef GAME_SERVER_ENTITIES_SMOKESCREEN_H
#define GAME_SERVER_ENTITIES_SMOKESCREEN_H


#include <game/server/entity.h>

class CSmokescreen : public CEntity
{
public:
	static const int ms_PhysSize = 24;
	
	int m_Life;
	int m_MaxLife;
	
	int m_NextIn;
	
	CSmokescreen(CGameWorld *pGameWorld, vec2 Pos, int MaxLife, int StartLife = 0);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
};

#endif
