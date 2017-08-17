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
	virtual void CreateProjectile();
	
	const int GetWeaponType() { return m_Type; }
	const int GetPowerLevel() { return m_PowerLevel; }
	
	void OnOwnerDeath(bool IsActive);
	
	void ReduceAmmo(int Amount);
	void IncreaseAmmo(int Amount);
	
	void Clear();
	
	bool Fire(float *pKnockback = NULL);
	
	
	bool Drop();
	bool Upgrade();
	bool CanSwitch();
	bool AddClip();
	
	void SetOwner(int CID);
	
	void SetPos(vec2 Pos, vec2 Direction, float Radius);
	
	bool FullAuto(){ return m_FullAuto; }
	
	bool UsesAmmo(){ return m_MaxAmmo > 0 ? true : false; }
	int GetAmmo(){ return m_Ammo; }
	
	int m_Ammo;
	int m_MaxAmmo;
	int m_PowerLevel;
	
protected:
	vec2 m_Pos;
	vec2 m_Direction;
	
	int m_Type;
	int m_ReloadTimer;
	bool m_FullAuto;
	int m_Owner;
	
private:

};

#endif
