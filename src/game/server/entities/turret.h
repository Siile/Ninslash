#ifndef GAME_SERVER_ENTITIES_TURRET_H
#define GAME_SERVER_ENTITIES_TURRET_H

#include <game/server/entity.h>
#include <game/server/entities/building.h>


class CTurret : public CBuilding
{
public:
	CTurret(CGameWorld *pGameWorld, vec2 Pos, int Team, class CWeapon *pWeapon);

	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	class CWeapon *m_pWeapon;
	
	int m_AttackTick;
	int m_TargetTimer;
	vec2 m_Target;
	int m_Angle;
	int m_Ammo;
	int m_Team;
	
	vec2 m_OriginalDirection;
	int m_OwnerPlayer;
	
	void SetAngle(vec2 Direction);
	int m_FlipY;
	
private:

	bool FindTarget();
	bool Target();
	bool Fire();
	
	int m_TargetIndex;
	int m_ReloadTimer;
};

#endif
