

#ifndef GAME_SERVER_ENTITIES_PICKUP_H
#define GAME_SERVER_ENTITIES_PICKUP_H

#include <game/server/entity.h>

const int PickupPhysSize = 14;

class CPickup : public CEntity
{
public:
	CPickup(CGameWorld *pGameWorld, int Type, int SubType = 0, int Ammo = -1);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	class CWeapon *m_pWeapon;
	
	float m_Ammo;
	
	void Respawn()
	{
		if (!m_Dropable || m_ResetableDropable)
		{
			m_SpawnTick = -1;
			m_Flashing = false;
			m_FlashTimer = 0;
			ClearWeapon();
		}
	}
	
	void SurvivalReset();
	
	void RespawnDropable()
	{
		m_SpawnTick = -1;
		m_Life = 480;
		m_Dropable = true;
		m_Flashing = false;
		m_FlashTimer = 0;
		m_Subtype = 0;
		m_Treasure = false;
		ClearWeapon();
	}
	
	void RespawnTreasure()
	{
		m_SpawnTick = -1;
		m_Life = 480;
		m_Dropable = false;
		m_Flashing = false;
		m_FlashTimer = 0;
		m_Subtype = 0;
		m_Treasure = true;
		m_SkipAutoRespawn = true;
		ClearWeapon();
	}
	
	void SetRandomWeapon();
	
	bool IsWeapon();
	
	void Hide()
	{
		m_SpawnTick = 1;
		m_Life = 0;
		m_Flashing = false;
		m_FlashTimer = 0;
		m_Subtype = 0;
	}
	
	void ClearWeapon();
	
	bool m_SkipAutoRespawn;
	
	bool m_Dropable;
	int m_Life;
	int m_SpawnTick;
	
	bool m_Treasure;
	
	bool m_Flashing;
	int m_FlashTimer;
	
	vec2 m_Vel;
	
	int GetType(){ return m_Type; }
	int GetSubtype(){ return m_Type; }
	
	void SetSubtype(int Type){ m_Subtype = Type; }
	
private:
	int m_Type;
	int m_Subtype;
	
	vec2 m_SpawnPos;
	bool m_ResetableDropable;
};

#endif
