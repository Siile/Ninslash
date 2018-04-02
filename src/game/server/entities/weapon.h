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
	
	const int GetWeaponType() { return m_WeaponType; }
	const int GetPowerLevel() { return m_PowerLevel; }
	const int GetOwner() { return m_Owner; }
	
	void OnOwnerDeath(bool IsActive);
	
	void ReduceAmmo(int Amount);
	void IncreaseAmmo(int Amount);
	
	void Clear();
	
	bool Fire(float *pKnockback = NULL);
	
	bool Throw();
	bool Charge();
	bool ReleaseCharge(float *pKnockback = NULL);
	int GetCharge();
	
	bool Drop();
	bool CanSwitch();
	bool AddClip();
	
	void UpdateStats();
	
	void SetOwner(int CID);
	
	void SetCharge(int Charge);
	void SetPos(vec2 Pos, vec2 Vel, vec2 Direction, float Radius);
	
	bool FullAuto(){ return m_FullAuto; }
	
	bool UsesAmmo(){ return m_MaxAmmo > 0 ? true : false; }
	int GetAmmo(){ return m_Ammo; }
	
	void SetTurret(bool TurretBit = true);
	
	int m_Ammo;
	int m_MaxAmmo;
	int m_PowerLevel;
	bool m_Disabled; // for dropping shurikens safely
	
	void OnPlayerPick();
	void Deactivate();
	
protected:
	vec2 m_Vel;
	vec2 m_Direction;
	
	bool m_Stuck;
	
	bool m_Released;
	
	void Move();
	
	bool m_IsTurret;
	
	int m_BurstCount;
	int m_BurstMax;
	
	int m_WeaponType;
	int m_ReloadTimer;
	int m_BurstReloadTimer;
	bool m_FullAuto;
	int m_Owner;
	
	int m_AttackTick;
	int m_TriggerTick;
	int m_TriggerCount;
	
	void Trigger();
	
	int m_Charge;
	bool m_ChargeLocked;

	int m_FireSound;
	int m_FireSound2;
	bool m_CanFire;
	float m_FireRate;
	float m_KnockBack;
	
	bool m_UseAmmo;
	
	int m_ChargeSoundTimer;
	
	// for rendering thrown weapon
	float m_Angle;
	float m_AngleForce;
	
private:
	int m_LastNoAmmoSound;
	int m_DestructionTick;
	
	void SelfDestruct();
};

#endif
