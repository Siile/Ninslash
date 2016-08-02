#ifndef GAME_SERVER_ENTITIES_MONSTER_H
#define GAME_SERVER_ENTITIES_MONSTER_H

#include <game/server/entity.h>

const int MonsterPhysSize = 32;

class CMonster : public CEntity
{
public:
	CMonster(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	void TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Type = DAMAGETYPE_NORMAL);
	int m_Health;
	
private:
	int m_Status;
	int m_Dir;
	
	vec2 m_StartPos;
	
	int m_DamageTakenTick;
	int m_DeathTick;
};

#endif
