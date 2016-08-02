#ifndef GAME_SERVER_ENTITIES_ARROW_H
#define GAME_SERVER_ENTITIES_ARROW_H

#include <game/server/entity.h>

class CArrow : public CEntity
{
public:
	CArrow(CGameWorld *pGameWorld);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	bool m_Hide;
	vec2 m_Target;

private:
	int m_Range;
	vec2 m_aTipTargetPos[16], m_aBottomTargetPos[16];
	vec2 m_aTipPos[16], m_aBottomPos[16];
};

#endif
