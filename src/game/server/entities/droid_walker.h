#ifndef GAME_SERVER_ENTITIES_DROID_WALKER_H
#define GAME_SERVER_ENTITIES_DROID_WALKER_H

#include <game/server/entity.h>
#include "droid.h"

const int WalkerPhysSize = 60;

class CWalker : public CDroid
{
public:
	CWalker(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();

	virtual void TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Weapon);

	enum Mode
	{
		WALKER,
		DRONE,
	};
	
private:

	bool FindTarget();
	bool Target();
	void Fire();
};

#endif
