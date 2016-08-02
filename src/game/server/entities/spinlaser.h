

#ifndef GAME_SERVER_ENTITIES_SPINLASER_H
#define GAME_SERVER_ENTITIES_SPINLASER_H

#include <game/server/entity.h>

class CSpinlaser : public CEntity
{
public:
	CSpinlaser(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner1, int Owner2, int Damage);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	
	void Update(vec2 Pos, vec2 Dir);

protected:
	bool HitCharacter(vec2 From, vec2 To);
	void Spin();

private:
	vec2 m_From;
	vec2 m_Dir;
	
	int m_EvalTick;
	int m_Owner1;
	int m_Owner2;
	
	float m_Energy;
	
	int m_Damage;
	
	int m_DamageTimer;
};

#endif
