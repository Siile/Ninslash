#ifndef GAME_SERVER_ENTITIES_BUILDING_H
#define GAME_SERVER_ENTITIES_BUILDING_H

#include <game/server/entity.h>

const int BuildingPhysSize = 32;
const int TurretPhysSize = 32;
const int SawbladePhysSize = 32;
const int MinePhysSize = 6;
const int BarrelPhysSize = 28;
const int LazerPhysSize = 10;
const int PowerupperPhysSize = 10;
const int BasePhysSize = 10;
const int StandPhysSize = 20;

class CBuilding : public CEntity
{
public:
	CBuilding(CGameWorld *pGameWorld, vec2 Pos, int Type, int Team);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int m_Type;
	int m_Team;
	int m_Life;
	int m_MaxLife;
	
	bool Repair(int Amount = 10);
	
	int m_aStatus[NUM_BSTATUS];
	int m_Status;
	
	vec2 m_Center;
	
	int m_DamageOwner;
	int m_DeathTimer;
	
	void TakeDamage(int Damage, int Owner, int Weapon);
	void Destroy();
	
protected:
	void UpdateStatus();
	
private:
	int m_SetTimer;
};

#endif
