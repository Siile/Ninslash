#ifndef GAME_SERVER_ENTITIES_DROID_STAR_H
#define GAME_SERVER_ENTITIES_DROID_STAR_H

#include <game/server/entity.h>
#include "droid.h"

const int StarPhysSize = 60;

class CStar : public CDroid
{
public:
	CStar(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();

	virtual void TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int m_Weapon);
	
private:
	bool FindTarget();
	bool Target();
	void Fire();
	
	vec2 m_MoveTarget;
	float m_AngleTimer;
};

#endif
