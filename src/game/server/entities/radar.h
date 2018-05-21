#ifndef GAME_SERVER_ENTITIES_RADAR_H
#define GAME_SERVER_ENTITIES_RADAR_H

#include <game/server/entity.h>

class CRadar : public CEntity
{
	int m_Type;
	int m_ObjectiveID;
	vec2 m_TargetPos;
	bool m_Active;
	int m_ActiveTick;
	
public:
	CRadar(CGameWorld *pGameWorld, int Type, int ObjectiveID = 0);

	void Activate(vec2 Pos) { m_Active = true; m_TargetPos = Pos;}
	void Activate(vec2 Pos, int ActiveTick) { m_Active = true; m_TargetPos = Pos; m_ActiveTick = ActiveTick; }
	
	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif
