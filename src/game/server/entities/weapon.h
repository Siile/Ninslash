#ifndef GAME_SERVER_ENTITIES_WEAPON_H
#define GAME_SERVER_ENTITIES_WEAPON_H

#include <game/server/entity.h>


class CWeapon : public CEntity
{
public:
	static const int ms_PhysSize = 14;
	CWeapon(CGameWorld *pGameWorld, int Type);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	
	const int GetWeaponType() { return m_Type; }
	const int GetPowerLevel() { return m_PowerLevel; }
	
	void OnOwnerDeath(bool IsActive);
	
	void ReduceAmmo(int Amount);
	void IncreaseAmmo(int Amount);
	
	void Clear();
	
	bool Drop();
	bool Upgrade();
	bool CanSwitch();
	bool AddClip();
	
	bool UsesAmmo(){ return m_MaxAmmo > 0 ? true : false; }
	int GetAmmo(){ return m_Ammo; }
	
	int m_Ammo;
	int m_MaxAmmo;
	int m_PowerLevel;
	
private:
	int m_Type;
};

#endif
