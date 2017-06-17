#ifndef GAME_SERVER_ENTITIES_LIGHTNING_H
#define GAME_SERVER_ENTITIES_LIGHTNING_H

#include <game/server/entity.h>

class CLightning : public CEntity
{
public:
	CLightning(CGameWorld *pGameWorld, vec2 Pos, vec2 From);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

protected:

private:
	vec2 m_From;
	vec2 m_Dir;
	int m_EvalTick;
};

#endif
