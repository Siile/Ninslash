

#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <game/server/entity.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>

#include <game/weapons.h>

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

enum BaseDamage
{
	RED_BASE = -3,
	BLUE_BASE = -2,
	NEUTRAL_BASE = -1,
};

enum WeaponGroup
{
	WEAPONGROUP_PRIMARY,
	WEAPONGROUP_SECONDARY,
	WEAPONGROUP_TOOL,

};

class CCharacter : public CEntity
{
	MACRO_ALLOC_POOL_ID()

public:
	// weapon system
	int m_ActiveWeaponGroup;
	int m_ActivePrimaryWeapon;
	int m_ActiveSecondaryWeapon;
	int m_ActiveTool;

	//character's size
	static const int ms_PhysSize = 28;

	CCharacter(CGameWorld *pWorld);

	virtual void Reset();
	virtual void Destroy();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	bool IsGrounded();

	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();

	void Deathray();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	void FireWeapon();

	bool UsingMeleeWeapon()
	{
		if (m_ActiveCustomWeapon == WEAPON_HAMMER ||
			m_ActiveCustomWeapon == WEAPON_TOOL)
			return true;

		return false;
	}

	void Die(int Killer, int Weapon, bool SkipKillMessage = false, bool IsTurret = false);
	bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon, vec2 Pos, int Type = DAMAGETYPE_NORMAL, bool IsTurret = false);
	void SetAflame(float Duration, int From, int Weapon);
	void TakeDeathtileDamage();
	void TakeSawbladeDamage(vec2 SawbladePos);
	void TakeDeathrayDamage();

	vec2 m_SpawnPos;

	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();

	void Teleport(vec2 Pos);

	// ability STORE_HEALTH
	int m_HealthStored;

	bool m_UseMedkit;
	int m_MedkitTimer;

	bool IncreaseHealth(int Amount);
	bool IncreaseArmor(int Amount);

	bool AddMine();
	bool AddKit();

	void SetHealth(int Health);
	void RefillHealth();

	void SetEmote(int Emote, int Tick);
	void SetEmoteFor(int Emote, int Ticks, int LockEmote = 0, bool UseTime = true);

	bool IsAlive() const { return m_Alive; }
	class CPlayer *GetPlayer() { return m_pPlayer; }


	bool m_IsBot;
	int m_HiddenHealth;
	int m_MaxHealth;

	bool m_Silent;

	bool m_WeaponPicked;

	int GetActiveWeapon(){ return m_ActiveWeapon; }

	int m_SkipPickups;

	int m_DeathTileTimer;

	int m_BombStatus;

	void ElectroShock()
	{
		m_Core.m_Vel /= 1.5f;
	}

	// custom weapon system
	int m_ActiveCustomWeapon;

	struct CustomWeaponStat
	{
		int m_Ammo;
		bool m_Got;
		bool m_Disabled;
		bool m_Ready;
	} m_aWeapon[NUM_CUSTOMWEAPONS];

	bool AddClip(int Weapon = -1);

	// for pickup drops, easy access
	bool HasAmmo()
	{
		if (m_aWeapon[m_ActiveCustomWeapon].m_Ammo > 0 ||
			aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo == 0)
			return true;

		return false;
	}

	bool GotWeapon(int CustomWeapon){ return m_aWeapon[CustomWeapon].m_Got; }
	bool WeaponDisabled(int CustomWeapon){ return m_aWeapon[CustomWeapon].m_Disabled; }
	void DisableWeapon(int CustomWeapon){ m_aWeapon[CustomWeapon].m_Disabled = true; }

	bool GiveCustomWeapon(int CustomWeapon, float AmmoFill = 1.0f);
	void GiveRandomWeapon(int WeaponLevel = -1);
	void GiveAllWeapons();
	bool GiveAmmo(int *CustomWeapon, float AmmoFill);

	void SetCustomWeapon(int CustomWeapon);

	void AutoWeaponChange();

	void GiveStartWeapon();

	void ShowArmor();

	void ScanWeapons();
	int GetFirstWeapon(int ParentType);
	int GetWeapon(int ParentType);
	int GetNextWeapon(int ParentType);
	int GetPrevWeapon(int ParentType);

	// next that shares a parent
	int m_aNextWeapon[NUM_WEAPONS];


	bool SetLandmine();
	bool SetElectromine();

	void DropWeapon();
	void SwitchGroup();

	int m_CryTimer;
	int m_CryState;

	void Cry();

	bool m_DelayedKill;

	CCharacterCore GetCore(){ return m_Core; }
	vec2 GetPosition(){ return m_Pos; }

	vec2 GetVel(){ return m_Core.m_Vel; }

	vec2 m_LatestHitVel;

	int m_QueuedCustomWeapon;

	void SelectWeapon(int Weapon, int Group);
	int m_WeaponGroup;

	int m_aItem[NUM_PLAYERITEMS];

	bool IgnoreCollision()
	{
		if (m_aStatus[STATUS_SPAWNING] > 0.0f)
			return true;

		return false;
	}

	bool Invisible();

	bool IsSliding()
	{
		if (m_Core.m_Slide != 0)
			return true;

		return false;
	}


	int m_ShieldHealth;
	int m_ShieldRadius;

	void SelectItem(int Item);
	void UseKit(int Kit, vec2 Pos);
	void GiveBuff(int Item);
	void GiveRandomBuff();

	int m_AttackTick;
	int m_DamageTakenTick;

	int m_Kits;

private:
	// player controlling this character
	class CPlayer *m_pPlayer;

	int m_DeathrayTick;

	int m_LastStatusEffect;

	int m_PrevWeapon;

	int m_aStatus[NUM_STATUSS];
	int m_aStatusFrom[NUM_STATUSS];
	int m_aStatusWeapon[NUM_STATUSS];
	void UpdateCoreStatus();

	bool m_Alive;

	vec2 m_Recoil;

	int m_Chainsaw;
	void Chainsaw();

	CCharacter *m_LockedTo;

	int m_PainSoundTimer;

	// weapon info
	CEntity *m_apHitObjects[10];
	int m_NumObjectsHit;

	int m_aSelectedWeapon[3];

	int m_ActiveWeapon;
	int m_LastWeapon;

	int m_ReloadTimer;

	int m_DamageTaken;

	int m_EmoteType;
	int m_EmoteStop;
	int m_EmoteLockStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;

	int m_LastReloadingSound;

	// these are non-heldback inputs
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	CNetObj_PlayerInput m_PrevInput;
	CNetObj_PlayerInput m_Input;
	int m_NumInputs;
	int m_Jumped;


	int m_Health;
	int m_Armor;

	// ninja
	struct
	{
		vec2 m_ActivationDir;
		int m_ActivationTick;
		int m_CurrentMoveTime;
		int m_OldVelAmount;
	} m_Ninja;

	// the player core for the physics
	CCharacterCore m_Core;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

};

#endif
