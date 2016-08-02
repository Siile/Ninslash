#ifndef GAME_SERVER_ENTITIES_FLAG_H
#define GAME_SERVER_ENTITIES_FLAG_H

#include <game/server/entity.h>

class CFlag : public CEntity
{
public:
	static const int ms_PhysSize = 14;
	CCharacter *m_pCarryingCharacter;
	vec2 m_Vel;
	vec2 m_StandPos;

	int m_Team;
	int m_AtStand;
	int m_DropTick;
	int m_GrabTick;
	
	bool m_Hide;
	bool m_UseSnapping;
	
	// for domination
	int m_CaptureTeam;
	float m_CapturePoints;
	
	// to check if to be snapped / displayed
	bool m_ClosestFlagToCharacter[64];
	
	void ResetDistanceInfo()
	{
		for (int i = 0; i < 64; i++)
			m_ClosestFlagToCharacter[i] = false;
	}
	
	CFlag(CGameWorld *pGameWorld, int Team);

	virtual void Reset();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
};

#endif
