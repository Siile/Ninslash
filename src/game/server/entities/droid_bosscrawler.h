#ifndef GAME_SERVER_ENTITIES_DROID_BOSSCRAWLER_H
#define GAME_SERVER_ENTITIES_DROID_BOSSCRAWLER_H

#include <game/server/entity.h>
#include "droid.h"

const int BossCrawlerPhysSize = 80;

class CBossCrawler : public CDroid
{
public:
	CBossCrawler(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();

	virtual void TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Weapon);

private:
	bool FindTarget();
	bool Target();
	void Fire();
	
	void Move();
	void MoveDead();
	
	int m_Move;
	
	vec2 m_MoveTarget;
	float m_AngleTimer;
	
	int m_AttackCount;
	
	int m_JumpTick;
	float m_JumpForce;
};

#endif
