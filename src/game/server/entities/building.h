#ifndef GAME_SERVER_ENTITIES_BUILDING_H
#define GAME_SERVER_ENTITIES_BUILDING_H

#include <game/server/entity.h>

const int BuildingPhysSize = 32;
const int TurretPhysSize = 32;
const int TeslacoilPhysSize = 36;
const int SawbladePhysSize = 32;
const int MinePhysSize = 6;
const int BarrelPhysSize = 28;
const int LazerPhysSize = 10;
const int PowerupperPhysSize = 10;
const int BasePhysSize = 10;
const int StandPhysSize = 20;
const int LightningWallPhysSize = 20;
const int FlametrapPhysSize = 20;
const int SwitchPhysSize = 10;
const int DoorPhysSize = 40;
const int ReactorPhysSize = 50;
const int JumppadPhysSize = 60;

class CBuilding : public CEntity
{
public:
	CBuilding(CGameWorld *pGameWorld, vec2 Pos, int Type, int Team);

	virtual void Reset();
	virtual void Tick();
	virtual void SurvivalReset();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	int m_Type;
	int m_Team;
	int m_Life;
	int m_MaxLife;
	
	bool Repair(int Amount = 10);
	
	int m_aStatus[NUM_BSTATUS];
	int m_Status;
	
	bool m_Collision;
	
	vec2 m_Center;
	
	bool m_Mirror;
	
	int m_DamageOwner;
	int m_DeathTimer;
	
	bool Jumppad();
	void Trigger();
	void TakeDamage(int Damage, int Owner, int Weapon);
	void Destroy();
	
protected:
	void UpdateStatus();
	
	int m_TriggerTimer;
	
private:
	int m_SetTimer;
	
	// lightning wall
	void CreateLightningWallTop();
	int m_Height;
};

#endif
