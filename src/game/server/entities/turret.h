#ifndef GAME_SERVER_ENTITIES_TURRET_H
#define GAME_SERVER_ENTITIES_TURRET_H

#include <game/server/entity.h>
#include <game/server/entities/building.h>


class CTurret : public CBuilding
{
public:
	CTurret(CGameWorld *pGameWorld, vec2 Pos, int Team, int Weapon);

	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int m_AttackTick;
	int m_TargetTimer;
	vec2 m_Target;
	int m_Angle;
	int m_Weapon;
	int m_PowerLevel;
	int m_Ammo;
	
	int m_Flamethrower;
	
	vec2 m_OriginalDirection;
	int m_OwnerPlayer;
	
	void SetAngle(vec2 Direction);
	
private:
	bool FindTarget();
	bool Target();
	void Fire();
	void Flamethrower();
	int m_TargetIndex;
	
	int m_ReloadTimer;
	int m_Chainsaw;
	float m_DelayedShotgunTick;
	void DelayedFire();
};

#endif
