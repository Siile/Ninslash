#ifndef GAME_SERVER_ENTITIES_ELECTROWALL_H
#define GAME_SERVER_ENTITIES_ELECTROWALL_H

#include <game/server/entity.h>


class CElectroWall : public CEntity
{
public:
	CElectroWall(CGameWorld *pGameWorld, vec2 Pos1, vec2 Pos2);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

private:
	vec2 m_Pos2;
	int m_StartTick;
	int m_DestructionTick;
	
	bool HitCharacter();
};

#endif
