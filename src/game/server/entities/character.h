

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


class CCharacter : public CEntity
{
	MACRO_ALLOC_POOL_ID()

public:
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
	
	bool PlayerCollision(){ return m_Core.m_PlayerCollision; }
	
	bool Wallrun(){ return (abs(m_Core.m_Wallrun) > 0 && abs(m_Core.m_Wallrun) < 10);}
	
	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();
	
	bool m_ExplodeOnDeath;
	
	void Warp();
	void Deathray(bool Kill = true);
	void Electrocute(float Duration);
	void Jumppad();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	void FireWeapon();

	void ReleaseWeapons();
	
	void Die(int Killer, int Weapon, bool SkipKillMessage = false, bool IsTurret = false);
	//bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon, vec2 Pos, int Type = DAMAGETYPE_NORMAL, bool IsTurret = false);
	bool TakeDamage(int From, int Weapon, int Dmg, vec2 Force, vec2 Pos);
	void SetAflame(float Duration, int From, int Weapon);
	void TakeDeathtileDamage();
	void TakeSawbladeDamage(vec2 SawbladePos);
	void TakeDeathrayDamage();

	vec2 m_SpawnPos;
	
	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();
	
	void Teleport(vec2 Pos);
	
	bool IncreaseHealth(int Amount);
	bool IncreaseAmmo(int Amount);
	bool IncreaseArmor(int Amount);
	
	bool AddMine();
	bool AddKit();
	
	void SetArmor(int Armor);
	void SetHealth(int Health);
	void RefillHealth();

	void SetEmote(int Emote, int Tick);
	void SetEmoteFor(int Emote, int Ticks, int LockEmote = 0, bool UseTime = true);

	bool IsAlive() const { return m_Alive; }
	class CPlayer *GetPlayer() { return m_pPlayer; }
	
	class CWeapon *GetWeapon(int Slot = -1) {
		if (Slot >= NUM_SLOTS)
			return NULL;
		
		if (Slot < 0 && m_WeaponSlot >= 0 && m_WeaponSlot < NUM_SLOTS)
			return m_apWeapon[m_WeaponSlot];
		
		if (Slot < 0)
			return NULL;
		
		return m_apWeapon[Slot];
	}
	
	bool m_ForceCoreSend;
	
	bool m_IsBot;
	int m_HiddenHealth;
	int m_MaxHealth;
	
	bool m_Silent;
	
	bool m_WeaponPicked;
	
	void SaveData();
	
	int m_SkipPickups;
	
	int m_DeathTileTimer;
	
	int m_BombStatus;
	
	void ElectroShock()
	{
		m_Core.m_Vel /= 1.5f;
	}
	
	bool UpgradeWeapon();
	
	int m_WeaponSlot;
	int m_WantedSlot;
	
	
	bool AddClip(int Weapon = -1);
	
	// for pickup drops, easy access
	bool HasAmmo();
	
	bool GiveWeapon(class CWeapon *pWeapon);
	int GetWeaponType(int Slot = -1);
	int GetWeaponSlot(){ return clamp(m_WeaponSlot, 0, 3);}
	int GetWeaponPowerLevel(int WeaponSlot = -1);
	
	bool PickWeapon(class CWeapon *pWeapon);
	
	void RandomizeInventory();
	
	void AutoWeaponChange();
	
	void GiveStartWeapon();
	
	int m_PickedWeaponSlot;


	int m_Type;
	
	enum Types
	{
		PLAYER,
		ROBOT
	};
	
	bool SetLandmine();
	bool SetElectromine();

	void DropWeapon();
	
	int m_CryTimer;
	int m_CryState;
	
	void Cry();
	
	bool m_DelayedKill;
	
	CCharacterCore GetCore(){ return m_Core; }
	vec2 GetPosition(){ return m_Pos; }
	
	vec2 GetVel(){ return m_Core.m_Vel; }
	
	vec2 m_LatestHitVel;
	
	int m_QueuedCustomWeapon;
	
	int m_aItem[NUM_PLAYERITEMS];
	
	bool IgnoreCollision()
	{
		if (m_IgnoreCollision || m_aStatus[STATUS_SPAWNING] > 0.0f)
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
	bool GiveBuff(int Item);
	void GiveRandomBuff();
	
	int m_AttackTick;
	int m_DamageTakenTick;
	int m_SendInventoryTick;	
	
	int m_Kits;
	
	bool ScytheReflect();
	bool m_DamagedByPlayer;
	
	int GetArmor() { return m_Armor; }
	
	// inventory
	void DropItem(int Slot, vec2 Pos);
	void SwapItem(int Item1, int Item2);
	void CombineItem(int Item1, int Item2);
	void TakePart(int Item1, int Slot, int Item2);
	void SendInventory();
	
	void ReplaceWeapon(int Slot, int Part1, int Part2);
	void ReleaseWeapon(class CWeapon *pWeapon = NULL);
	bool TriggerWeapon(class CWeapon *pWeapon = NULL);
	
	bool UpgradeTurret(vec2 Pos, vec2 Dir, int Slot = -1);
	
private:
	// player controlling this character
	class CPlayer *m_pPlayer;
	
	class CWeapon *m_apWeapon[12];

	bool m_IgnoreCollision;
	
	int m_DeathrayTick;
	int m_DamageSoundTimer;
	
	int m_LastStatusEffect;
	
	int m_PrevWeapon;
	
	int m_aStatus[NUM_STATUSS];
	int m_aStatusFrom[NUM_STATUSS];
	int m_aStatusWeapon[NUM_STATUSS];
	void UpdateCoreStatus();
	
	bool m_Spawned;
	bool m_Alive;
	
	int m_AcidTimer;
	
	vec2 m_Recoil;

	int m_ChangeDirTick;
	int m_LastDir;
	
	int m_ChargeTick;
	
	int m_Flamethrower;
	void Flamethrower();
	
	CCharacter *m_LockedTo;
	
	int m_PainSoundTimer;
	
	// weapon info
	CEntity *m_apHitObjects[10];

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
