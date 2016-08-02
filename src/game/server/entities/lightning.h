

#ifndef GAME_SERVER_ENTITIES_LIGHTNING_H
#define GAME_SERVER_ENTITIES_LIGHTNING_H

#include <game/server/entity.h>

class CLightning : public CEntity
{
public:
	CLightning(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, float StepEnergy, int Owner, int Damage, int MaxDesc, int Num = 0);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

protected:
	bool HitCharacter(vec2 From, vec2 To);
	void DoBounce();

private:
	vec2 m_From;
	vec2 m_Dir;
	float m_Energy;
	float m_StartEnergy;
	float m_StepEnergy;
	int m_Bounces;
	int m_EvalTick;
	int m_Owner;
	int m_Damage;
	int m_Num;
	int m_MaxDesc;
};

#endif
