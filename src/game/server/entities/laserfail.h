#ifndef GAME_SERVER_ENTITIES_LASERFAIL_H
#define GAME_SERVER_ENTITIES_LASERFAIL_H

#include <game/server/entity.h>

// used for lightning wall destruction effect
class CLaserFail : public CEntity
{
public:
	CLaserFail(CGameWorld *pGameWorld, vec2 From, vec2 To, int PowerLevel);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

private:
	vec2 m_From;
	int m_EvalTick;
	int m_PowerLevel;
};

#endif
