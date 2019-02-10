#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "character.h"
#include "weapon.h"
#include "building.h"
#include "turret.h"
#include "laser.h"
#include "projectile.h"
#include "superexplosion.h"
#include "droid.h"
#include "laserfail.h"
#include "staticlaser.h"

#include <game/weapons.h>
#include <game/buildables.h>

#include <game/server/playerdata.h>

inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }


#define RAD 0.017453292519943295769236907684886f

//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1)
			c.m_Presses++;
		else
			c.m_Releases++;
	}

	return c;
}


MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS)

// Character, "physical" player's part
CCharacter::CCharacter(CGameWorld *pWorld)
: CEntity(pWorld, CGameWorld::ENTTYPE_CHARACTER)
{
	m_Spawned = false;
	m_ProximityRadius = ms_PhysSize;
	m_HiddenHealth = 100;
	m_MaxHealth = 10;
	m_Health = 0;
	m_Armor = 0;
	m_Kits = 0;
	m_PainSoundTimer = 0;
	m_Silent = false;
	m_IgnoreCollision = false;
	m_SendInventoryTick = 0;
	m_ForceCoreSend = false;
	
	for (int i = 0; i < NUM_STATUSS; i++)
	{
		m_aStatus[i] = 0;
		m_aStatusFrom[i] = 0;
		m_aStatusWeapon[i] = 0;
	}
	
	m_LastStatusEffect = 0;
	m_DeathrayTick = 0;
	
	m_Type = CCharacter::PLAYER;
}


void CCharacter::Reset()
{
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	m_Spawned = true;
	m_DamagedByPlayer = false;
	m_PickedWeaponSlot = 0;
	m_MaskEffectTick = 0;
	
	for (int i = 0; i < NUM_PLAYERITEMS; i++)
		m_aItem[i] = 0;
	
	for (int i = 0; i < NUM_STATUSS; i++)
		m_aStatus[i] = 0;
	
	for (int i = 0; i < NUM_SLOTS; i++)
		m_apWeapon[i] = NULL;
	
	m_aStatus[STATUS_SPAWNING] = 0.7f*Server()->TickSpeed();
	
	m_SendInventoryTick = Server()->Tick() + Server()->TickSpeed()*2.5f;
	
	m_ChangeDirTick = 0;
	m_LastDir = 0;
	m_DamageSoundTimer = 0;
	
	m_ShieldHealth = 0;
	m_ShieldRadius = 0;
	
	m_WeaponSlot = 0;
	m_WantedSlot = 0;
	
	m_AcidTimer = 0;
	
	m_Recoil = vec2(0, 0);
	
	m_SkipPickups = 0;
	
	m_CryTimer = 0;
	m_CryState = 0;
	
	m_ExplodeOnDeath = false;
	
	m_EmoteLockStop = 0;
	m_DeathTileTimer = 0;
	m_DelayedKill = false;
	m_WeaponPicked = false;
	m_IsBot = false;
	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_PrevWeapon = WEAPON_HAMMER;
	m_QueuedCustomWeapon = -1;

	m_PainSoundTimer = 0;
	
	m_pPlayer = pPlayer;
	m_Pos = Pos;

	m_ChargeTick = 0;
	
	m_SpawnPos = Pos;
	
	m_LatestHitVel = vec2(0, 0);
	
	
	m_Core.Reset();
	m_Core.Init(&GameServer()->m_World.m_Core, GameServer()->Collision());
	m_Core.m_Pos = m_Pos;

	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));

	GameServer()->m_World.InsertEntity(this);
	m_Alive = true;

	if (pPlayer->m_pAI)
	{
		delete pPlayer->m_pAI;
		pPlayer->m_pAI = NULL;
	}
	
	GameServer()->m_pController->OnCharacterSpawn(this, pPlayer->m_IsBot);
	
	if (pPlayer->m_pAI)
	{
		m_IsBot = true;
		pPlayer->m_TeeInfos.m_IsBot = true;
		pPlayer->m_pAI->OnCharacterSpawn(this);
		pPlayer->m_IsBot = true;
		
		if (GameServer()->m_pController->IsCoop())
			m_Silent = true;
	}
	else
	{
		//m_apWeapon[0] = GameServer()->NewWeapon(GetStaticWeapon(SW_CLUSTER));
		//m_apWeapon[1] = GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(4, 2), 10));
		//m_apWeapon[1] = GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(1, 3), 15));
	}
	
	//m_apWeapon[0] = GameServer()->NewWeapon(GetModularWeapon(5, 9));
	//m_apWeapon[1] = GameServer()->NewWeapon(GetModularWeapon(5, 9));
	//m_apWeapon[2] = GameServer()->NewWeapon(GetModularWeapon(6, 9));

	/*
	m_apWeapon[0] = GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(1, 2), 10));
	*/
	
	
	/*
	int n = 0;
	//m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_SHIELD));
	//m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_INVIS));
	//m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_UPGRADE));
	m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_BAZOOKA));
	m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_CHAINSAW));
	m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_BOUNCER));
	m_apWeapon[n++] = GameServer()->NewWeapon(GetStaticWeapon(SW_FLAMER));
	*/
	
	//m_apWeapon[0] = GameServer()->NewWeapon(GetModularWeapon(3, 1));
	//m_apWeapon[1] = GameServer()->NewWeapon(GetStaticWeapon(SW_BALL));
	//m_apWeapon[2] = GameServer()->NewWeapon(GetStaticWeapon(SW_MASK5));
	//m_apWeapon[1] = GameServer()->NewWeapon(GetModularWeapon(5, 6));
	
	GiveStartWeapon();
	SendInventory();
	
	return true;
}


bool CCharacter::GiveBomb()
{
	if (FreeSlot() >= 0 && GetPlayer()->GetTeam() == TEAM_RED)
	{
		m_apWeapon[FreeSlot()] = GameServer()->NewWeapon(GetStaticWeapon(SW_BOMB));
		SendInventory();
		return true;
	}
	
	return false;
}


int CCharacter::FreeSlot()
{
	for (int i = 0; i < NUM_SLOTS; i++)
		if (!m_apWeapon[i])
			return i;
	
	return -1;
}

void CCharacter::RandomizeInventory()
{
	for (int x = 0; x < 16; x++)
	{
		int i = rand()%4;
		int j = rand()%12;
		
		if (i == j)
			continue;
		
		if (!m_apWeapon[j] || j == m_WeaponSlot)
			continue;
	
		bool CanSwitch = true;
		
		int wt1 = GetStaticType(GetWeaponType(i));
		int wt2 = GetStaticType(GetWeaponType(j));

		if ((i > 0 && i <= 3) || (j > 0 && j <= 3))
		{
			if ((wt1 >= SW_MASK1 && wt1 <= SW_MASK5) || (wt2 >= SW_MASK1 && wt2 <= SW_MASK5))
				continue;
		}
		
		if (i == 0 && (wt2 == SW_BOMB || (wt2 >= SW_MASK1 && wt2 <= SW_MASK5)))
			continue;
		
		if ((m_apWeapon[i] && !m_apWeapon[i]->CanSwitch()) || (m_apWeapon[j] && !m_apWeapon[j]->CanSwitch()))
			CanSwitch = false;
		
		if (CanSwitch)
		{
			CWeapon *pW1 = m_apWeapon[i];
			m_apWeapon[i] = m_apWeapon[j];
			m_apWeapon[j] = pW1;
		}
	}
	
	if (!m_IsBot)
		SendInventory();
}


void CCharacter::SaveData()
{
	if (m_IsBot || !m_Spawned || !GameServer()->m_pController->IsCoop())
		return;
	
	CPlayerData *pData = GameServer()->Server()->GetPlayerData(GetPlayer()->GetCID(), GetPlayer()->GetColorID());

	pData->m_Kits = m_Kits;
	pData->m_Armor = m_Armor;
	pData->m_Score = GetPlayer()->m_Score;
	pData->m_Gold = GetPlayer()->m_Gold;
	
	if (g_Config.m_SvMapGenLevel > pData->m_HighestLevel)
	{
		pData->m_HighestLevel = g_Config.m_SvMapGenLevel;
		pData->m_HighestLevelSeed = g_Config.m_SvMapGenSeed;
	}
	
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		if (m_apWeapon[i])
		{
			pData->m_aWeaponType[i] = GetWeaponType(i);
			pData->m_aWeaponAmmo[i] = m_apWeapon[i]->m_Ammo;
		}
		else
			pData->m_aWeaponType[i] = 0;
	}
	
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Data save - color=%d", GetPlayer()->GetColorID());
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "Character", aBuf);
}

	
bool CCharacter::GiveWeapon(class CWeapon *pWeapon)
{
	if (!pWeapon)
		return false;

	if (m_WeaponSlot < 0 || m_WeaponSlot > NUM_SLOTS)
		return false;
	
	if (m_apWeapon[m_WeaponSlot])
	{
		for (int i = 0; i < NUM_SLOTS; i++)
		{
			if (!m_apWeapon[i])
			{
				m_apWeapon[i] = pWeapon;
				pWeapon->OnPlayerPick();
				
				if (m_IsBot && GameServer()->m_pController->IsCoop())
					pWeapon->m_InfiniteAmmo = true;
				
				//SendInventory();
				return true;
			}
		}
		return false;
	}
	
	m_apWeapon[m_WeaponSlot] = pWeapon;
	pWeapon->OnPlayerPick();
	
	if (m_IsBot && GameServer()->m_pController->IsCoop())
		pWeapon->m_InfiniteAmmo = true;
	
	//SendInventory();
	return true;
}

int CCharacter::GetWeaponType(int Slot)
{
	if (Slot < 0)
	{
		if (m_WeaponSlot < 0 || m_WeaponSlot >= NUM_SLOTS)
			return WEAPON_NONE;
		
		if (!m_apWeapon[m_WeaponSlot])
			return WEAPON_NONE;

		return m_apWeapon[m_WeaponSlot]->GetWeaponType();
	}
	else
	{

		if (Slot >= NUM_SLOTS)
			return WEAPON_NONE;
		
		if (!m_apWeapon[Slot])
			return WEAPON_NONE;

		return m_apWeapon[Slot]->GetWeaponType();
	}
}

int CCharacter::GetWeaponPowerLevel(int WeaponSlot)
{
	if (WeaponSlot < 0)
		WeaponSlot = m_WeaponSlot;
	
	if (WeaponSlot < 0 || WeaponSlot > NUM_SLOTS)
		return 0;
	
	if (!m_apWeapon[WeaponSlot])
		return 0;

	return m_apWeapon[WeaponSlot]->GetPowerLevel();
}


bool CCharacter::SetLandmine()
{
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x-16, m_Pos.y+24)&CCollision::COLFLAG_SOLID && GameServer()->Collision()->GetCollisionAt(m_Pos.x+16, m_Pos.y+24)&CCollision::COLFLAG_SOLID)
	{
		//new CLandmine(&GameServer()->m_World, m_Pos + vec2(0, 16), m_pPlayer->GetCID());
		CBuilding *b = new CBuilding(&GameServer()->m_World, m_Pos + vec2(0, 6), BUILDING_MINE1, GameServer()->m_pController->IsTeamplay() ? m_pPlayer->GetTeam() : TEAM_NEUTRAL);
		b->m_DamageOwner = GetPlayer()->GetCID();
		GameServer()->CreateSound(m_Pos, SOUND_BODY_LAND);
		return true;
	}
	return false;
}


bool CCharacter::SetElectromine()
{
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x-16, m_Pos.y+24)&CCollision::COLFLAG_SOLID && GameServer()->Collision()->GetCollisionAt(m_Pos.x+16, m_Pos.y+24)&CCollision::COLFLAG_SOLID)
	{
		//new CLandmine(&GameServer()->m_World, m_Pos + vec2(0, 16), m_pPlayer->GetCID());
		CBuilding *b = new CBuilding(&GameServer()->m_World, m_Pos + vec2(0, 6), BUILDING_MINE2, GameServer()->m_pController->IsTeamplay() ? m_pPlayer->GetTeam() : TEAM_NEUTRAL);
		b->m_DamageOwner = GetPlayer()->GetCID();
		GameServer()->CreateSound(m_Pos, SOUND_BODY_LAND);
		return true;
	}
	return false;
}





void CCharacter::Teleport(vec2 Pos)
{
	m_Pos = Pos;
	m_Core.m_Pos = m_Pos;

	m_Core.Reset();
	
	m_Pos = Pos;
	m_Core.m_Pos = m_Pos;
	
	if (GetPlayer()->m_pAI)
		GetPlayer()->m_pAI->StandStill(15);
}

void CCharacter::Destroy()
{
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	m_Alive = false;
}

void CCharacter::SendInventory()
{
	if (m_IsBot)
		return;
	
	CNetMsg_Sv_Inventory Msg;
	Msg.m_Item1 = GetWeaponType(0);
	Msg.m_Item2 = GetWeaponType(1);
	Msg.m_Item3 = GetWeaponType(2);
	Msg.m_Item4 = GetWeaponType(3);
	Msg.m_Item5 = GetWeaponType(4);
	Msg.m_Item6 = GetWeaponType(5);
	Msg.m_Item7 = GetWeaponType(6);
	Msg.m_Item8 = GetWeaponType(7);
	Msg.m_Item9 = GetWeaponType(8);
	Msg.m_Item10 = GetWeaponType(9);
	Msg.m_Item11 = GetWeaponType(10);
	Msg.m_Item12 = GetWeaponType(11);
	Msg.m_Gold = GetPlayer()->GetGold();
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, GetPlayer()->GetCID());
}


void CCharacter::InventoryRoll()
{
	if (m_WeaponSlot < 0 || m_WeaponSlot >= NUM_SLOTS)
		return;
	
	int w1 = m_WeaponSlot;
	int w2 = (m_WeaponSlot+4)%NUM_SLOTS;
	int w3 = (m_WeaponSlot+8)%NUM_SLOTS;
	
	if (!m_apWeapon[w1] && !m_apWeapon[w2] && !m_apWeapon[w3])
		return;
	
	if ((m_apWeapon[w1] && !m_apWeapon[w1]->CanSwitch()) || (m_apWeapon[w2] && !m_apWeapon[w2]->CanSwitch()) || (m_apWeapon[w3] && !m_apWeapon[w3]->CanSwitch()))
		return;
	
	CWeapon *pW1 = m_apWeapon[w1];
	m_apWeapon[w1] = m_apWeapon[w2];
	m_apWeapon[w2] = m_apWeapon[w3];
	m_apWeapon[w3] = pW1;
	
	GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
	
	SendInventory();
}

	
void CCharacter::DropItem(int Slot, vec2 Pos)
{
	if (Slot < 0 || Slot >= 12)
		return;

	
	if (UpgradeTurret(Pos, vec2(Pos.x > m_Pos.x ? -1 : 1, 0), Slot))
	{
		m_apWeapon[Slot] = 0;
		SendInventory();
		return;
	}
	
	if (m_apWeapon[Slot] && m_apWeapon[Slot]->Drop())
	{
		//vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
		vec2 Direction = normalize(Pos - m_Pos);
		
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
		
		GameServer()->m_pController->DropWeapon(m_Pos+vec2(0, -16), m_Core.m_Vel/1.7f + Direction*10 + vec2(0, -3), m_apWeapon[Slot]);
		m_SkipPickups = 20;
				
		m_apWeapon[Slot] = NULL;
		SendInventory();
		return;
	}
}


void CCharacter::SwapItem(int Item1, int Item2)
{
	if (Item1 < 0 || Item1 >= NUM_SLOTS || Item2 >= NUM_SLOTS)
		return;
	
	if (Item1 == Item2)
		return;
	
	CWeapon *t = m_apWeapon[Item1];
	
	int w1 = GetWeaponType(Item1);
	int w2 = GetWeaponType(Item2);
	
	if (IsStaticWeapon(w1) && GetStaticType(w1) == SW_UPGRADE && Item1 != Item2)
	{
		if (GetWeapon(Item2))
		{
			if (GetWeaponCharge(w1) >= 4 && GetWeapon(Item2)->Supercharge())
			{
				m_apWeapon[Item1]->m_DestructionTick = 1;
				m_apWeapon[Item1] = NULL;
				
				// supercharge sound
				GameServer()->CreateSound(m_Pos, SOUND_UPGRADE);
			}
			else if (GetWeaponCharge(w1) >= 4 && GetStaticType(w2) == SW_UPGRADE)
			{
				GameServer()->CreateSound(m_Pos, SOUND_NEGATIVE);
			}
			else if (GetWeapon(Item2)->Overcharge())
			{
				m_apWeapon[Item1]->m_DestructionTick = 1;
				m_apWeapon[Item1] = NULL;
				
				// overcharge sound
				GameServer()->CreateSound(m_Pos, SOUND_UPGRADE);
			}
			else
				GameServer()->CreateSound(m_Pos, SOUND_NEGATIVE);
			
			SendInventory();
			return;
		}
	}
	
	// combine melee
	if (IsModularWeapon(w1) && IsModularWeapon(w2) && GetPart(w1, 0) == 5 && GetPart(w2, 0) == 5 && GetPart(w1, 1) == GetPart(w2, 1))
	{
		if (!m_apWeapon[Item1]->CanSwitch() || !m_apWeapon[Item2]->CanSwitch())
			return;
		
		m_apWeapon[Item1]->m_DestructionTick = 1;
		m_apWeapon[Item1] = NULL;
		m_apWeapon[Item2]->m_DestructionTick = 1;
		m_apWeapon[Item2] = NULL;
		m_apWeapon[Item2] = new CWeapon(&GameServer()->m_World, GetChargedWeapon(GetModularWeapon(6, GetPart(w1, 1)), max(GetWeaponCharge(w1), GetWeaponCharge(w2))));
		
		GameServer()->CreateSound(m_Pos, SOUND_UPGRADE);
		SendInventory();
		return;
	}
	
	// swap slots
	if (Item2 >= 0)
	{
		m_apWeapon[Item1] = m_apWeapon[Item2];
		m_apWeapon[Item2] = t;
	}
	
	// confirm inventory to the client
	SendInventory();
}


void CCharacter::CombineItem(int Item1, int Item2)
{
	if (Item1 < 0 || Item1 >= NUM_SLOTS || Item2 >= NUM_SLOTS)
		return;
	
	int w1 = GetWeaponType(Item1);
	int w2 = GetWeaponType(Item2);
	
	int p1_1 = GetPart(w1, 0);
	int p1_2 = GetPart(w1, 1);
	int p2_1 = GetPart(w2, 0);
	int p2_2 = GetPart(w2, 1);
	
	if (!p1_1 || !p1_2)
	{
		if (p1_1)
		{
			int t = p2_1;
			p2_1 = p1_1;
			p1_1 = t;
		}
		else if (p1_2)
		{
			int t = p2_2;
			p2_2 = p1_2;
			p1_2 = t;
		}
	}
	else
	{
		if (!p2_1 && p1_1)
		{
			p2_1 = p1_1;
			p1_1 = 0;
		}
		
		if (!p2_2 && p1_2)
		{
			p2_2 = p1_2;
			p1_2 = 0;
		}
	}
	
	ReplaceWeapon(Item1, p1_1, p1_2);
	ReplaceWeapon(Item2, p2_1, p2_2);
	
	// confirm inventory to the client
	SendInventory();
}


void CCharacter::TakePart(int Item1, int Slot, int Item2)
{
	if (Item1 < 0 || Item1 >= NUM_SLOTS || Item2 < 0 || Item2 >= NUM_SLOTS)
		return;
	
	int w1 = GetWeaponType(Item1);
	int w2 = GetWeaponType(Item2);
	
	if (IsStaticWeapon(w1) || IsStaticWeapon(w2))
		return;
	
	int p1_1 = GetPart(w1, 0);
	int p1_2 = GetPart(w1, 1);
	int p2_1 = GetPart(w2, 0);
	int p2_2 = GetPart(w2, 1);

	if (Slot == 0)
	{
		ReplaceWeapon(Item1, p2_1, p1_2);
		ReplaceWeapon(Item2, p1_1, p2_2);
		//m_aItem[Item1] = GetWeapon(p2_1, p1_2);
		//m_aItem[Item2] = GetWeapon(p1_1, p2_2);
	}
	else if (Slot == 1)
	{
		ReplaceWeapon(Item1, p1_1, p2_2);
		ReplaceWeapon(Item2, p2_1, p1_2);
		//m_aItem[Item1] = GetWeapon(p1_1, p2_2);
		//m_aItem[Item2] = GetWeapon(p2_1, p1_2);
	}
	
	// confirm inventory to the client
	SendInventory();
}


void CCharacter::ReplaceWeapon(int Slot, int Part1, int Part2)
{
	if (Slot < 0 || Slot >= NUM_SLOTS)
		return;
	
	if (!m_apWeapon[Slot])
		m_apWeapon[Slot] = GameServer()->NewWeapon(Part1, Part2);
	else
	{
		GameServer()->m_World.DestroyEntity(m_apWeapon[Slot]);
		m_apWeapon[Slot] = GameServer()->NewWeapon(Part1, Part2);
	}
}


bool CCharacter::TriggerWeapon(CWeapon *pWeapon)
{
	if (!pWeapon || GetWeapon() != pWeapon)
		return false;
	
	int w = GetWeaponType();
	
	if (IsStaticWeapon(w))
	{
		if (GetStaticType(w) == SW_BOMB)
		{
			ReleaseWeapon();
			return true;
		}
		
		if (GetStaticType(w) == SW_INVIS)
			return GiveBuff(PLAYERITEM_INVISIBILITY);
		
		if (GetStaticType(w) == SW_SHIELD)
			return GiveBuff(PLAYERITEM_SHIELD);
		
		if (GetStaticType(w) == SW_RESPAWNER && (!GameServer()->m_pController->IsCoop() || !m_IsBot))
			return GameServer()->RespawnAlly(m_Pos, GetPlayer()->GetTeam());
		
		return false;
	}
	
	return false;
}

void CCharacter::ReleaseWeapon(CWeapon *pWeapon)
{
	if (!pWeapon)
	{
		if (!GetWeapon())
			return;
		
		GetWeapon()->Throw();
		m_apWeapon[GetWeaponSlot()] = NULL;
	}
	else
	{
		for (int i = 0; i < NUM_SLOTS; i++)
		{
			if (m_apWeapon[i] == pWeapon)
			{
				m_apWeapon[i] = NULL;
				//if (GetWeaponSlot() == i)
				//	m_ActiveWeapon = WEAPON_NONE;
				
				break;
			}
		}
	}
	
	SendInventory();
}

	
bool CCharacter::IsBombCarrier()
{
	for (int i = 0; i < NUM_SLOTS; i++)
		if (GetStaticType(GetWeaponType(i)) == SW_BOMB)
			return true;
	
	return false;
}
	
	

bool CCharacter::PickWeapon(CWeapon *pWeapon)
{
	// cs | reactor defense
	if (GetStaticType(pWeapon->GetWeaponType()) == SW_BOMB && GetPlayer()->GetTeam() != TEAM_RED)
		return false;
	
	if (!GetWeapon())
	{
		pWeapon->SetOwner(GetPlayer()->GetCID());
		m_apWeapon[GetWeaponSlot()] = pWeapon;
		m_PickedWeaponSlot = GetWeaponSlot();
		SendInventory();
		return true;
	}
	
	bool Valid = true;
	
	int w = pWeapon->GetWeaponType();
	
	if (WeaponAutoPick(pWeapon->GetWeaponType()))
	{
		// check if weapon is lower level than currently held weapons overall
		if (WeaponMaxLevel(w) > 1)
		{
			float Weapons = 0.0f;
			float WeaponLevel = 0.0f;
			
			for (int i = 0; i < 4; i++)
			{
				if (WeaponMaxLevel(GetWeaponType(i)) > 1)
				{
					Weapons += 1.0f;
					WeaponLevel += GetWeaponCharge(GetWeaponType(i));
				}
				
				if (Weapons > 1.0f)
					WeaponLevel /= Weapons;
				
				if (GetWeaponCharge(w) < WeaponLevel && Weapons > 0.0f && GetWeaponCharge(w) < WeaponMaxLevel(w))
					Valid = false;
			}
		}
		
		for (int i = 0; i < NUM_SLOTS; i++)
		{
			if (GetChargedWeapon(GetWeaponType(i), 0) == GetChargedWeapon(pWeapon->GetWeaponType(), 0) && 
				GetWeaponCharge(GetWeaponType(i)) >= GetWeaponCharge(pWeapon->GetWeaponType()) && 
				GetStaticType(GetWeaponType(i)) != SW_UPGRADE && GetStaticType(GetWeaponType(i)) != SW_RESPAWNER)
				Valid = false;
		}
	}
	else
		Valid = false;
	
	if (GetStaticType(w) == SW_UPGRADE)
		Valid = true;
	
	if (Valid)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!m_apWeapon[i])
			{
				pWeapon->SetOwner(GetPlayer()->GetCID());
				m_apWeapon[i] = pWeapon;
				m_PickedWeaponSlot = i;
				SendInventory();
				return true;
			}
		}
	}
		
	return false;
}

bool CCharacter::UpgradeTurret(vec2 Pos, vec2 Dir, int Slot)
{
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "character", "Upgrade turret");
	
	if (Slot < 0)
		Slot = GetWeaponSlot();
	
	if (!GetWeapon(Slot))
		return false;
	
	if (!ValidForTurret(GetWeaponType(Slot)))
		return false;
	
	// check if near upgradeable buildings
	float CheckRange = 48.0f;
	CBuilding *pNear = NULL;
	CBuilding *apEnts[16];
	int Num = GameServer()->m_World.FindEntities(Pos+vec2(0, -20), 40, (CEntity**)apEnts, 16, CGameWorld::ENTTYPE_BUILDING);

	// check for turret stands
	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pTarget = apEnts[i];
		
		if (pTarget->m_Type == BUILDING_STAND && distance(pTarget->m_Pos, Pos+vec2(0, -20)) < CheckRange)
		{
			pNear = pTarget;
			break;
		}
	}
	
	// transform stand to turret
	if (pNear)
	{
		vec2 p = pNear->m_Pos;
		GameServer()->m_World.DestroyEntity(pNear);
		
		int Team = GetPlayer()->GetTeam();
		if (!GameServer()->m_pController->IsTeamplay())
			Team = GetPlayer()->GetCID();
		
		GetWeapon(Slot)->SetOwner(GetPlayer()->GetCID());
		CTurret *pTurret = new CTurret(&GameServer()->m_World, p, Team, GetWeapon(Slot));
		pTurret->SetAngle(Dir);
				
		// sound
		GameServer()->CreateSound(Pos, SOUND_BUILD_TURRET);
		return true;
	}
	
	return false;
}

void CCharacter::DropWeapon()
{
	int Weapon = GetWeaponType();
	
	if (Weapon == WEAPON_NONE)
		return;
	
	if (GetWeaponSlot() < 0 || GetWeaponSlot() >= NUM_SLOTS)
		return;
	
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	
	if (m_HiddenHealth > 0)
	{
		if (UpgradeTurret(m_Pos, -Direction))
		{
			m_apWeapon[m_WeaponSlot] = 0;
			SendInventory();
			return;
		}
	}
	
	
	if (GetWeapon()->Drop())
	{
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
		
		if (GetStaticType(GetWeaponType()) == SW_BOMB)
			GameServer()->m_pController->DropWeapon(m_Pos+vec2(0, -16), (m_Core.m_Vel/1.7f + Direction*10 + vec2(0, -3))*0.75f, GetWeapon());
		else
			GameServer()->m_pController->DropWeapon(m_Pos+vec2(0, -16), m_Core.m_Vel/1.7f + Direction*10 + vec2(0, -3), GetWeapon());
		
		m_SkipPickups = 20;
				
		m_apWeapon[GetWeaponSlot()] = NULL;
		SendInventory();
		return;
	}
	
	if (GetWeapon()->ReleaseCharge())
	{
		if (GetWeaponFiringType(GetWeaponType()) == WFT_THROW)
			ReleaseWeapon();
		
		m_apWeapon[GetWeaponSlot()] = NULL;
		SendInventory();
		return;
	}
}

	
bool CCharacter::IsGrounded()
{
	
	if(GameServer()->Collision()->CheckPoint(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	
	int c1 = GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5);
	int c2 = GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5);
	
	if (c1&CCollision::COLFLAG_SOLID || c2&CCollision::COLFLAG_SOLID)
		return true;
	
	return false;
}




void CCharacter::DoWeaponSwitch()
{
	// auto free hands to weapon
	/*
	if (GetWeaponType() == WEAPON_NONE)
	{
		for (int i = 0; i < 3; i++)
			if (GetWeaponType(i) >= 0)
			{
				m_WantedSlot = i;
				break;
			}
	}
	*/
	
	if (m_WantedSlot != m_WeaponSlot)
	{
		if (m_apWeapon[m_WeaponSlot] && !m_apWeapon[m_WeaponSlot]->CanSwitch())
			return;
		
		if (m_apWeapon[m_WantedSlot] && !m_apWeapon[m_WantedSlot]->CanSwitch())
			return;
		
		m_WeaponSlot = m_WantedSlot;
		m_AttackTick = 0;
	}
}


void CCharacter::HandleWeaponSwitch()
{
	int WantedSlot = m_WeaponSlot;
	
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	if(Next < 128)
	{
		while(Next) // Next Weapon selection
		{
			//WantedSlot = clamp(WantedSlot+1, 0, 3);
			if (++WantedSlot > 3)
				WantedSlot = 0;
			Next--;
		}
	}
	if(Prev < 128)
	{
		while(Prev) // Prev Weapon selection
		{
			//WantedSlot = clamp(WantedSlot-1, 0, 3);
			if (--WantedSlot < 0)
				WantedSlot = 3;
			Prev--;
		}
	}
	
	if(m_LatestInput.m_WantedWeapon)
		WantedSlot = clamp(m_Input.m_WantedWeapon-2, 0, 3);
	
	m_WantedSlot = WantedSlot;
	
	DoWeaponSwitch();
}



void CCharacter::Jumppad()
{
	/*
	m_Core.m_Vel.y = -9.0f;
	m_Core.m_Action = COREACTION_JUMPPAD;
	m_Core.m_ActionState = 0;
	*/
	
	m_Core.Jumppad();
}



int CCharacter::Reflect()
{
	//if (m_ScytheTick > Server()->Tick()-Server()->TickSpeed()*0.2f)
	//	return true;

	if (GetMask() == 3 && frandom() < 0.6f)
		return m_ProximityRadius;
	
	if (GetWeapon() && GetWeapon()->Reflect())
		return GetWeapon()->Reflect();

	return 0;
}



void CCharacter::FireWeapon()
{
	if (m_aStatus[STATUS_SPAWNING] > 0.0f)
		return;

	if (GetWeaponType() == WEAPON_NONE)
		return;
	
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	GetWeapon()->SetPos(m_Pos, m_Core.m_Vel, Direction, m_ProximityRadius);
	GetWeapon()->SetOwner(GetPlayer()->GetCID());
	
	if (GetWeaponFiringType(GetWeaponType()) == WFT_CHARGE || GetWeaponFiringType(GetWeaponType()) == WFT_THROW)
	{
		float Knockback = 0.0f;
		
		// charge
		if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses || m_LatestInput.m_Fire&1)
		{
			if (GetWeapon()->Charge() && !m_ChargeTick)
			{
				m_ChargeTick = Server()->Tick();
				
				if (GetWeaponFiringType(GetWeaponType()) == WFT_THROW)
					m_AttackTick = Server()->Tick();
			}
		}
		// release
		else if (GetWeapon()->ReleaseCharge(&Knockback))
		{
			m_ChargeTick = 0;
			m_AttackTick = Server()->Tick();
			if (GetWeaponFiringType(GetWeaponType()) == WFT_THROW)
			{
				ReleaseWeapon();
				m_Core.m_ChargeLevel = -20;
				return;
			}
		}
		
		m_Core.m_ChargeLevel = GetWeapon()->GetCharge();
		return;
	}
	
	m_Core.m_ChargeLevel = 0;
	
	// trigger finger
	bool FullAuto = m_IsBot ? true : GetWeapon()->FullAuto();
	
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1))
		WillFire = true;

	if (m_Core.m_ChargeLevel < 0)
		WillFire = false;
	
	if(!WillFire)
		return;
	
	float Knockback = 0.0f;
	
	// fire
	if (GetWeapon()->Fire(&Knockback))
	{
		m_Recoil -= Direction * Knockback;
		m_AttackTick = Server()->Tick();
	}
	else
	{
		
		
	}
}

	
	
void CCharacter::HandleWeapons()
{
	/*
	if(m_ReloadTimer > 0)
	{
		m_ReloadTimer--;
		return;
	}
	*/
	
	// fire Weapon, if wanted
	FireWeapon();

	return;
}


void CCharacter::AutoWeaponChange()
{
	// todo
}


bool CCharacter::HasAmmo()
{
	if (!GetWeapon() || GetWeapon()->m_Ammo <= 0)
		return false;
	
	return true;
}


void CCharacter::GiveStartWeapon()
{
	//if (GameServer()->m_pController->IsCoop())
	if (str_comp(g_Config.m_SvGametype, "coop") == 0)
	{
		if (m_IsBot)
			return;
		
		// load saved weapons
		CPlayerData *pData = GameServer()->Server()->GetPlayerData(GetPlayer()->GetCID(), GetPlayer()->GetColorID());
		
		bool GotItems = false;
		
		for (int i = 0; i < NUM_SLOTS; i++)
		{
			if (pData->m_aWeaponType[i])
			{
				m_apWeapon[i] = GameServer()->NewWeapon(pData->m_aWeaponType[i]);
				m_apWeapon[i]->m_Ammo = pData->m_aWeaponAmmo[i];
			}
			
			if (m_apWeapon[i])
				GotItems = true;
		}
		
		if (!GotItems)
			m_apWeapon[0] = GameServer()->NewWeapon(GetStaticWeapon(SW_GUN1));
		
		m_Kits = pData->m_Kits;
		m_Armor = pData->m_Armor;
		GetPlayer()->m_Score = pData->m_Score;
		GetPlayer()->m_Gold = pData->m_Gold;
		
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Data load - color=%d", GetPlayer()->GetColorID());
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "Character", aBuf);
		
		return;
	}
	
	// CS / reactor defense
	if (str_comp(g_Config.m_SvGametype, "def") == 0)
	{
		bool GotItems = false;
		
		for (int i = 0; i < NUM_SLOTS; i++)
			if (m_apWeapon[i])
				GotItems = true;
		
		if (!GotItems)
		{
			if (frandom() < 0.5f)
				m_apWeapon[0] = GameServer()->NewWeapon(GetStaticWeapon(SW_GUN1));
			else
				m_apWeapon[0] = GameServer()->NewWeapon(GetStaticWeapon(SW_GUN2));
		}
		
		return;
	}
	
	// dm, tdm, ctf
	int w = 0;
	
	if (g_Config.m_SvRandomWeapons)
	{
		if (frandom() < 0.5f)
		{
			m_apWeapon[w++] = GameServer()->NewWeapon(GameServer()->m_pController->GetRandomModularWeapon());
		}
		else
		{
			if (frandom() < 0.5f)
				m_apWeapon[w++] = GameServer()->NewWeapon(GetStaticWeapon(SW_GUN1));
			else
				m_apWeapon[w++] = GameServer()->NewWeapon(GetStaticWeapon(SW_GUN2));
			
			if (frandom() < 0.5f)
				m_apWeapon[w++] = GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1));
			else
				m_apWeapon[w++] = GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE2));
		}
	}
}

void CCharacter::SetEmote(int Emote, int Tick)
{
	//if (m_EmoteLockStop > Tick)
	//	return;
	
	if (m_EmoteLockStop > Server()->Tick())
		return;

	m_EmoteType = Emote;
	m_EmoteStop = Tick;
}

void CCharacter::SetEmoteFor(int Emote, int Ticks, int LockEmote, bool UseTime)
{
	if (m_EmoteLockStop > Server()->Tick() && LockEmote == 0)
		return;

	m_EmoteType = Emote;
	
	if (UseTime)
	{
		m_EmoteStop = Server()->Tick() + Ticks * Server()->TickSpeed() / 1000;
		if (LockEmote > 0)
			m_EmoteLockStop = Server()->Tick() + LockEmote * Server()->TickSpeed() / 1000;
	}
	else
	{
		m_EmoteStop = Server()->Tick() + Ticks;
		if (LockEmote > 0)
			m_EmoteLockStop = Server()->Tick() + LockEmote;
	}
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
	// check for changes
	if(mem_comp(&m_Input, pNewInput, sizeof(CNetObj_PlayerInput)) != 0)
		m_LastAction = Server()->Tick();

	// copy new input
	mem_copy(&m_Input, pNewInput, sizeof(m_Input));
	m_NumInputs++;

	// it is not allowed to aim in the center
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
	mem_copy(&m_LatestInput, pNewInput, sizeof(m_LatestInput));

	// it is not allowed to aim in the center
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;

	if(m_NumInputs > 2 && m_pPlayer->GetTeam() != TEAM_SPECTATORS)
	{
		HandleWeaponSwitch();
		FireWeapon();
	}

	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
}

void CCharacter::ResetInput()
{
	m_Input.m_Direction = 0;
	m_Input.m_Hook = 0;
	m_Input.m_Down = 0;
	// simulate releasing the fire button
	if((m_Input.m_Fire&1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}

bool CCharacter::Invisible()
{
	if (m_aStatus[STATUS_SPAWNING] > 0)
		return true;
		
	if (m_DamageTakenTick > Server()->Tick() - Server()->TickSpeed() * 1.0f && frandom() < 0.4f)
		return false;
		
	if (m_AttackTick > Server()->Tick() - Server()->TickSpeed() * 1.0f && frandom() < 0.4f)
		return false;
	
	if (m_aStatus[STATUS_INVISIBILITY] > 0 && m_aStatus[STATUS_SHIELD] <= 0)
		return true;
		
	return false;
}


void CCharacter::UseKit(int Kit, vec2 Pos)
{
	if (Kit < 0 || Kit >= NUM_BUILDABLES)
		return;
		
	if (m_Kits >= BuildableCost[Kit])
	{
		if (GameServer()->AddBuilding(Kit, Pos, GetPlayer()->GetCID()))
		{
			m_Kits -= BuildableCost[Kit];
			GameServer()->CreateSound(Pos, SOUND_BUILD);
		}
	}
}
	
	
void CCharacter::SelectItem(int Item)
{
	if (m_aItem[Item] <= 0)
		return;
	
	if (Item == PLAYERITEM_RAGE && m_aStatus[STATUS_DASH] <= 0)
	{
		m_aStatus[STATUS_DASH] = Server()->TickSpeed() * 20.0f;
		m_aItem[Item]--;
	}
	
	if (Item == PLAYERITEM_LANDMINE)
	{
		if (SetLandmine())
			m_aItem[Item]--;
	}
	
	if (Item == PLAYERITEM_ELECTROMINE)
	{
		if (SetElectromine())
			m_aItem[Item]--;
	}
	
	if (Item == PLAYERITEM_SHIELD && m_aStatus[STATUS_SHIELD] <= 0)
	{
		m_aStatus[STATUS_SHIELD] = Server()->TickSpeed() * 20.0f;
		m_ShieldHealth = 100;
		m_ShieldRadius = 16;
		m_aItem[Item]--;
	}
	
	if (Item == PLAYERITEM_INVISIBILITY && m_aStatus[STATUS_INVISIBILITY] <= 0)
	{
		m_aStatus[STATUS_INVISIBILITY] = Server()->TickSpeed() * 20.0f;
		m_aItem[Item]--;
	}
}

bool CCharacter::UpgradeWeapon()
{
	if (GetWeapon() && GetWeapon()->Upgrade())
	{
		GameServer()->CreateSound(m_Pos, SOUND_UPGRADE);
		SendInventory();
		return true;
	}
		
	return false;
}

bool CCharacter::GiveBuff(int Item)
{
	if (Item < 0)
		return false;
	
	if (Item == PLAYERITEM_UPGRADE)
		return UpgradeWeapon();
		
	if (Item == PLAYERITEM_SHIELD)
	{
		if (m_aStatus[STATUS_SHIELD] > 0)
			return false;
		
		m_aStatus[STATUS_SHIELD] = Server()->TickSpeed() * 20.0f;
		m_ShieldHealth = 100;
		m_ShieldRadius = 16;
		return true;
	}
	
	if (Item == PLAYERITEM_INVISIBILITY)
	{
		if (m_aStatus[STATUS_INVISIBILITY] > 0)
			return false;
		
		m_aStatus[STATUS_INVISIBILITY] = Server()->TickSpeed() * 15.0f;
		return true;
	}
	
	return false;
}

void CCharacter::GiveRandomBuff()
{
	// disabled
	return;
	
	int Buff = -1;
	
	while (Buff < 0 || Buff == PLAYERITEM_FILL || Buff == PLAYERITEM_LANDMINE || Buff == PLAYERITEM_ELECTROMINE || (Buff == PLAYERITEM_FUEL && g_Config.m_SvUnlimitedTurbo))
		Buff = rand()%NUM_PLAYERITEMS;
	
	GiveBuff(Buff);
}


int CCharacter::GetMask()
{
	for (int i = 0; i < 12; i++)
	{
		int w = GetStaticType(GetWeaponType(i));
		if (w >= SW_MASK1 && w <= SW_MASK5 && GetWeaponSlot() != i)
			return w-(SW_MASK1-1);
	}
	
	return 0;
}


void CCharacter::UpdateCoreStatus()
{
	m_Core.m_Health = m_HiddenHealth;
	
	m_Core.m_Status = 0;
	
	if (GameServer()->m_pController->IsCoop() && m_IsBot)
		m_aStatus[STATUS_SLOWMOVING] = 9999;
	
	// end shield effect when needed
	if (m_ShieldHealth <= 0 || m_aStatus[STATUS_SHIELD] <= 0)
	{
		m_ShieldHealth = 0;
		m_aStatus[STATUS_SHIELD] = 0;
		m_ShieldRadius = 0;
	}
	
	// check if carrying bomb (reactor defense)
	bool m_BombCarrier = false;
	
	for (int w = 0; w < NUM_SLOTS; w++)
	{
		if (GetStaticType(GetWeaponType(w)) == SW_BOMB && GetWeaponSlot() != w)
		{
			m_BombCarrier = true;
			break;
		}
	}
	
	
	if (m_BombCarrier)
		m_aStatus[STATUS_BOMBCARRIER] = 100;
	else
		m_aStatus[STATUS_BOMBCARRIER] = 0;
	
	/*
	m_aStatus[STATUS_MASK1] = 10;
	m_aStatus[STATUS_MASK2] = 10;
	*/
	
	// pack statuses
	for (int i = 0; i < NUM_STATUSS; i++)
	{
		if (m_aStatus[i] > 0)
		{
			m_Core.m_Status |= 1 << i;
			m_aStatus[i]--;
		}
	}
	
	// store mask to status
	m_Core.m_Status |= GetMask() << STATUS_MASK1;
	
	if (g_Config.m_SvUnlimitedTurbo)
		m_Core.m_JetpackPower = 200;
	
	if(m_LastStatusEffect+Server()->TickSpeed()/3 <= Server()->Tick())
	{
		m_LastStatusEffect = Server()->Tick();
		
		 // flame damage
		if (m_aStatus[STATUS_AFLAME] > 0)
			TakeDamage(m_aStatusFrom[STATUS_AFLAME], m_aStatusWeapon[STATUS_AFLAME], 2, vec2(0, 0), vec2(0, 0));
		//	TakeDamage(vec2(0, 0), 4, m_aStatusFrom[STATUS_AFLAME], m_aStatusWeapon[STATUS_AFLAME], vec2(0, 0), DAMAGETYPE_FLAME);
	}
	
	// rolling stops flames a bit faster
	if (m_Core.m_Roll > 0 && m_aStatus[STATUS_AFLAME] > 0)
		m_aStatus[STATUS_AFLAME]--;
}


void CCharacter::Tick()
{
	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "debug", "Tick");

	// stress testing pickups
	
	//for (int i = 0; i < 50; i++)
	//	GameServer()->m_pController->DropWeapon(m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_UPGRADE)));
	
	/*
	GameServer()->m_pController->DropPickup(m_Pos, POWERUP_HEALTH, vec2(frandom()-frandom(), frandom()-frandom()*1.4f)*14.0f, 0);
	GameServer()->m_pController->DropPickup(m_Pos, POWERUP_ARMOR, vec2(frandom()-frandom(), frandom()-frandom()*1.4f)*14.0f, 0);
	GameServer()->m_pController->DropPickup(m_Pos, POWERUP_AMMO, vec2(frandom()-frandom(), frandom()-frandom()*1.4f)*14.0f, 0);
	*/
	
	//GameServer()->m_pController->DropPickup(m_Pos, POWERUP_COIN, vec2(frandom()-frandom(), frandom()-frandom()*1.4f)*14.0f, 0);
	//GameServer()->m_pController->DropPickup(m_Pos, POWERUP_HEALTH, vec2(frandom()-frandom(), frandom()-frandom()*1.4f)*14.0f, 0);
	
	if (m_PainSoundTimer > 0)
		m_PainSoundTimer--;
	
	if (m_SkipPickups > 0)
		m_SkipPickups--;
	
	if (m_DamageSoundTimer > 0)
		m_DamageSoundTimer--;
	
	// ugly way to test / visualize waypoints
	if (!m_IsBot && GameServer()->m_ShowWaypoints)
	{
		vec2 WpPos = GameServer()->Collision()->GetClosestWaypointPos(m_Pos);
		new CStaticlaser(&GameServer()->m_World, m_Pos, WpPos, 2);
	}
	
	if (m_SendInventoryTick && m_SendInventoryTick < Server()->Tick())
	{
		SendInventory();
		m_SendInventoryTick = 0;
	}
	
	if (IsBombCarrier())
	{
		GameServer()->m_pController->m_BombPos = m_Pos;
		GameServer()->m_pController->m_BombStatus = BOMB_CARRIED;
	}
	
	if (GetMask() == 1)
	{
		if (!m_MaskEffectTick || m_MaskEffectTick < Server()->Tick())
		{
			IncreaseHealth(1);
			m_MaskEffectTick = Server()->Tick()+Server()->TickSpeed()*0.5f;
		}
	}
	else
		m_MaskEffectTick = 0;
	
	
	if (g_Config.m_SvInfiniteGrenades)
	{
		bool GotGrenade = false;
		
		for (int w = 0; w < 4; w++)
		{
			if (GetStaticType(GetWeaponType(w)) == SW_GRENADE1)
				GotGrenade = true;
		}
		
		if (!GotGrenade)
		{
			int Slot = rand()%4;
			if (GetWeaponType(Slot) == 0 && Slot != GetWeaponSlot())
			{
				m_apWeapon[Slot] = GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1));
				SendInventory();
			}
		}
	}
	
	/*
	if(m_pPlayer->m_ForceBalanced)
	{
		char Buf[128];
		str_format(Buf, sizeof(Buf), "You were moved to %s due to team balancing", GameServer()->m_pController->GetTeamName(m_pPlayer->GetTeam()));
		GameServer()->SendBroadcast(Buf, m_pPlayer->GetCID());

		m_pPlayer->m_ForceBalanced = false;
	}
	*/

	UpdateCoreStatus();
	
	if (m_aStatus[STATUS_SPAWNING] > 0.0f)
		return;
	
	m_Core.m_Input = m_Input;

	float RecoilCap = 17.5f;
	
	if ((m_Core.m_Vel.x < RecoilCap && m_Recoil.x > 0) || (m_Core.m_Vel.x > -RecoilCap && m_Recoil.x < 0))
		m_Core.m_Vel.x += m_Recoil.x*0.7f;
	
	if ((m_Core.m_Vel.y < RecoilCap && m_Recoil.y > 0) || (m_Core.m_Vel.y > -RecoilCap && m_Recoil.y < 0))
		m_Core.m_Vel.y += m_Recoil.y*0.7f;
	
	m_Recoil *= 0.6f;
	
	if (m_Core.m_KickDamage >= 0 && m_Core.m_KickDamage < MAX_CLIENTS)
	{
		//TakeDamage(vec2(0, 0), 20, m_Core.m_KickDamage, WEAPON_WORLD, vec2(0, 0), DAMAGETYPE_NORMAL);
		GameServer()->CreateSound(m_Pos, SOUND_KICKHIT);
	}
	
	m_Core.m_ClientID = GetPlayer()->GetCID();
	m_Core.Tick(true);
	
	if (m_Core.m_BallHitVel.x != 0.0f || m_Core.m_BallHitVel.y != 0.0f)
		GameServer()->m_pController->m_LastBallToucher = GetPlayer()->GetCID();
	
	// anti head stuck
	if(GameServer()->Collision()->CheckPoint(m_Pos.x, m_Pos.y-m_ProximityRadius/3.f-42) && (!m_Core.IsGrounded() && m_Core.m_Slide == 0))
	{
		m_Pos.y += 1.0f;
		m_Core.m_Pos.y += 1.0f;
		m_Core.m_Vel.y = 0.0f;
	}
	
	// monster damage
	//if (m_Core.m_MonsterDamage)
	//	TakeDamage(normalize(m_Core.m_Vel), 10, -1, DEATHTYPE_DROID_WALKER, vec2(0, 0));
	
	if (m_Core.m_FluidDamage && m_AcidTimer <= 0)
	{
		TakeDamage(-1, WEAPON_ACID, 2, normalize(m_Core.m_Vel), vec2(0, 0));
		m_AcidTimer = 4;
	}
	
	if (m_AcidTimer > 0)
		m_AcidTimer--;
	
	//	TakeDamage(normalize(m_Core.m_Vel), 2, -1, WEAPON_WORLD, vec2(0, 0), DAMAGETYPE_FLUID);

	if (m_Core.m_KickDamage >= 0 && m_Core.m_KickDamage < MAX_CLIENTS)
	{
		//TakeDamage(vec2(0, 0), 20, m_Core.m_KickDamage, WEAPON_WORLD, vec2(0, 0), DAMAGETYPE_NORMAL);
		GameServer()->CreateSound(m_Pos, SOUND_KICKHIT);
	}

	
	if (m_CryTimer > 0)
		m_CryTimer--;

	// handle death-tiles
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f-24)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f-24)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH)
	{
		m_DeathTileTimer = 10;
		TakeDeathtileDamage();
	}
	
	// handle insta death-tiles and leaving gamelayer
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f-24)&CCollision::COLFLAG_INSTADEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_INSTADEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f-24)&CCollision::COLFLAG_INSTADEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_INSTADEATH ||
		GameLayerClipped(m_Pos))
	{
		Die(m_pPlayer->GetCID(), DEATHTYPE_SPIKE);
	}
	
	// delayed death ray
	if (m_DeathrayTick > 0 && m_DeathrayTick <= Server()->Tick())
		TakeDeathrayDamage();
	
	
	if (m_DelayedKill)
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);
		m_LatestHitVel = vec2(0, 0);
	}

	if (m_DeathTileTimer > 0)
		m_DeathTileTimer--;
	
	// GameServer()->CreateDeath(m_Pos+vec2(frandom()*100, frandom()*100) - vec2(frandom()*100, frandom()*100), -1);
	
	// handle Weapons
	HandleWeapons();

	// Previnput
	m_PrevInput = m_Input;
	
	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "debug", "Tick end");
	
	return;
}

void CCharacter::TickDefered()
{
	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GameServer()->Collision());
		m_ReckoningCore.Tick(false);
		m_ReckoningCore.Move();
		m_ReckoningCore.Quantize();
	}

	//lastsentcore
	vec2 StartPos = m_Core.m_Pos;
	vec2 StartVel = m_Core.m_Vel;
	//bool StuckBefore = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.Move();
	//bool StuckAfterMove = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	//bool StuckAfterQuant = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

	/*
	if(!StuckBefore && (StuckAfterMove || StuckAfterQuant))
	{
		// Hackish solution to get rid of strict-aliasing warning
		union
		{
			float f;
			unsigned u;
		}StartPosX, StartPosY, StartVelX, StartVelY;

		StartPosX.f = StartPos.x;
		StartPosY.f = StartPos.y;
		StartVelX.f = StartVel.x;
		StartVelY.f = StartVel.y;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "STUCK!!! %d %d %d %f %f %f %f %x %x %x %x",
			StuckBefore,
			StuckAfterMove,
			StuckAfterQuant,
			StartPos.x, StartPos.y,
			StartVel.x, StartVel.y,
			StartPosX.u, StartPosY.u,
			StartVelX.u, StartVelY.u);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}
	*/

	int Events = m_Core.m_TriggeredEvents;
	int Mask = CmaskAllExceptOne(m_pPlayer->GetCID());

	if(Events&COREEVENT_GROUND_JUMP) GameServer()->CreateSound(m_Pos, SOUND_PLAYER_JUMP, Mask);

	if(m_pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		m_Pos.x = m_Input.m_TargetX;
		m_Pos.y = m_Input.m_TargetY;
	}

	// update the m_SendCore if needed
	{
		CNetObj_Character Predicted;
		CNetObj_Character Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		// only allow dead reackoning for a top of 3 seconds
		if(m_ForceCoreSend || m_ReckoningTick+Server()->TickSpeed()*3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ForceCoreSend = false;
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
}

void CCharacter::TickPaused()
{
	++m_AttackTick;
	++m_DamageTakenTick;
	++m_ReckoningTick;
	if(m_LastAction != -1)
		++m_LastAction;
	if(m_EmoteStop > -1)
		++m_EmoteStop;
}


void CCharacter::SetArmor(int Armor)
{
	m_Armor = Armor;
}

void CCharacter::SetHealth(int Health)
{
	m_MaxHealth = Health;
	m_HiddenHealth = Health;
}

void CCharacter::RefillHealth()
{
	m_HiddenHealth = m_MaxHealth;
}

bool CCharacter::IncreaseHealth(int Amount)
{
	if(m_HiddenHealth >= m_MaxHealth)
		return false;
	
	if (GetMask() == 4)
		Amount *= 2;
	
	m_HiddenHealth = clamp(m_HiddenHealth+Amount, 0, m_MaxHealth);
	
	//GetPlayer()->m_InterestPoints += 40;
	
	return true;
}

bool CCharacter::AddKits(int Amount)
{
	if (GameServer()->m_pController->IsInfection() && GetPlayer()->GetTeam() == TEAM_BLUE)
		return false;
	
	if (m_Kits < 99)
	{
		m_Kits = min(m_Kits+Amount, 99);
		return true;
	}
	
	return false;
}

bool CCharacter::AddKit()
{
	if (GameServer()->m_pController->IsInfection() && GetPlayer()->GetTeam() == TEAM_BLUE)
		return false;
	
	if (m_Kits < 99)
	{
		m_Kits = min(m_Kits+5, 99);
		return true;
	}
	
	return false;
}


bool CCharacter::AddClip(int Weapon)
{
	if (GetWeapon() && GetWeapon()->AddClip())
		return true;
	
	for (int i = 0; i < NUM_SLOTS; i++)
		if (m_apWeapon[i] && m_apWeapon[i]->AddClip())
			return true;

	return false;
}

bool CCharacter::IncreaseAmmo(int Amount)
{
	if (GetMask() == 4)
	{
		if (AddClip())
		{
			AddClip();
			return true;
		}
		
		return false;
	}
	
	return AddClip();
}


bool CCharacter::IncreaseArmor(int Amount)
{
	if(m_Armor >= 100)
		return false;
	
	if (GetMask() == 4)
		Amount *= 2;
		
	m_Armor = clamp(m_Armor+Amount, 0, 100);
	return true;
}



void CCharacter::ReleaseWeapons()
{
	// drop mask
	if (!m_IsBot || !GameServer()->m_pController->IsCoop())
	{
		for (int i = 0; i < 4; i++)
		{
			int w = GetStaticType(GetWeaponType(i));
			
			if (w >= SW_MASK1 && w <= SW_MASK5 && GetWeaponSlot() != i)
			{
				GameServer()->m_pController->DropWeapon(m_Pos+vec2(0, -16), (m_Core.m_Vel/1.7f + vec2(0, -3))*0.75f, m_apWeapon[i]);
				m_apWeapon[i] = NULL;
				break;
			}
		}
	}
	
	for (int i = 0; i < NUM_SLOTS; i++)
		if (m_apWeapon[i])
		{
			if (GetStaticType(m_apWeapon[i]->GetWeaponType()) == SW_BOMB)
			{
				GameServer()->m_pController->DropWeapon(m_Pos+vec2(0, -16), (m_Core.m_Vel/1.7f + vec2(0, -3))*0.75f, m_apWeapon[i]);
			}
			else
			{
				m_apWeapon[i]->OnOwnerDeath(i == m_WeaponSlot);
			}
			
			m_apWeapon[i] = NULL;
		}
}


void CCharacter::Die(int Killer, int Weapon, bool SkipKillMessage, bool IsTurret)
{
	//if (Weapon < 0)
	//	Weapon = 0;
	// we got to wait 0.5 secs before respawning
	//m_pPlayer->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
	
	SaveData();
	
	m_pPlayer->m_DeathTick = Server()->Tick();
	
	if (g_Config.m_SvSurvivalMode)
		m_pPlayer->m_RespawnTick = Server()->Tick();
	else
		m_pPlayer->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*1;
	
	//if (Killer == m_pPlayer->GetCID() && (Weapon == WEAPON_HAMMER || Weapon == WEAPON_GAME))
	//	SkipKillMessage = true;
	
	if (Killer == NEUTRAL_BASE)
		Killer = GetPlayer()->GetCID();
	
	/*
	if (Weapon == W_DROID_STAR && Killer >= 0 && Killer != GetPlayer()->GetCID())
		Weapon = DEATHTYPE_DROID_STAR;
	
	if (Weapon == W_DROID_WALKER && Killer >= 0 && Killer != GetPlayer()->GetCID())
		Weapon = DEATHTYPE_DROID_WALKER;
	*/
	
	if (!SkipKillMessage && Killer >= 0)
	{
		int ModeSpecial = GameServer()->m_pController->OnCharacterDeath(this, GameServer()->m_apPlayers[Killer], Weapon);
		
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "kill killer='%d:%s' victim='%d:%s' weapon=%d special=%d",
			Killer, Server()->ClientName(Killer),
			m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()), Weapon, ModeSpecial);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

		// send the kill message
		if (Weapon != WEAPON_GAME)
		{
			CNetMsg_Sv_KillMsg Msg;
			Msg.m_Killer = Killer;
			Msg.m_Victim = m_pPlayer->GetCID();
			Msg.m_Weapon = Weapon;
			Msg.m_ModeSpecial = ModeSpecial;
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
		}
	}
	else
		GameServer()->m_pController->OnCharacterDeath(this, NULL, Weapon);
	
	// a nice sound
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_DIE);

	// this is for auto respawn after 3 secs
	m_pPlayer->m_DieTick = Server()->Tick();

	ReleaseWeapons();
	
	m_Alive = false;
	GameServer()->m_World.RemoveEntity(this);
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	
	if ((Killer >= 0 && Weapon != WEAPON_GAME) || !m_IsBot)
		GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID());
	

	//if (m_ExplodeOnDeath && Killer >= 0)
	//	GameServer()->CreateExplosion(m_Pos, Killer, Weapon, 0, false, false);
}


void CCharacter::Cry()
{
	if (m_CryTimer <= 0)
	{
		m_CryTimer = 50;
		if (m_CryState == 0 || m_CryState == 2)
		{
			GameServer()->CreateSound(m_Pos, SOUND_TEE_CRY);
			m_CryState++;
		}
		else if (m_CryState == 1)
		{
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);
			m_CryState++;
			m_CryTimer = 30;
		}
		else if (m_CryState == 3)
		{
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			m_CryState = 0;
		}
	}
	
}
#define RAD 0.017453292519943295769236907684886f


void CCharacter::Warp()
{
	GameServer()->CreateEffect(FX_MONSTERSPAWN, m_Pos);
	m_aStatus[STATUS_SPAWNING] = 5.5f*Server()->TickSpeed();
	//m_aStatus[STATUS_INVISIBILITY] = Server()->TickSpeed() * 20.0f;
	//m_IgnoreCollision = true;
}

void CCharacter::Deathray(bool Kill)
{
	if (m_DeathrayTick > 0)
		return;
	
	if (Kill)
	{
		m_DeathrayTick = Server()->Tick() + Server()->TickSpeed() * 0.2f;
		m_aStatus[STATUS_DEATHRAY] = 10.0f*Server()->TickSpeed();
	}
	else
	{
		m_aStatus[STATUS_DEATHRAY] = 0.25f*Server()->TickSpeed();
	}
}

void CCharacter::Electrocute(float Duration)
{
	if (m_aStatus[STATUS_ELECTRIC] < Duration*Server()->TickSpeed())
		m_aStatus[STATUS_ELECTRIC] = Duration*Server()->TickSpeed();
}


void CCharacter::SetAflame(float Duration, int From, int Weapon)
{
	if (IgnoreCollision())
		return;
	
	if(GameServer()->m_pController->IsFriendlyFire(m_pPlayer->GetCID(), From) && !g_Config.m_SvTeamdamage)
		return;
	
	if (m_aStatus[STATUS_AFLAME] < Duration*Server()->TickSpeed())
	{
		m_aStatusFrom[STATUS_AFLAME] = From;
		m_aStatusWeapon[STATUS_AFLAME] = Weapon;
		m_aStatus[STATUS_AFLAME] = Duration*Server()->TickSpeed();
	}
}


//bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon, vec2 Pos, int Type, bool IsTurret)
bool CCharacter::TakeDamage(int From, int Weapon, int Dmg, vec2 Force, vec2 Pos)
{
	// skip everything while spawning
	if (m_aStatus[STATUS_SPAWNING] > 0.0f)
		return false;
	
	if (!Dmg)
		return false;

	if (m_ShieldHealth <= 0)
		m_Recoil += Force;
	else
		m_Recoil += Force / 2;
	
	// signal AI
	if (Dmg > 0 && GetPlayer()->m_pAI && Weapon >= 0)
		GetPlayer()->m_pAI->ReceiveDamage(From, Dmg);
	
	if(GameServer()->m_pController->IsFriendlyFire(m_pPlayer->GetCID(), From) && !g_Config.m_SvTeamdamage)
		return false;
	
	float Flame = WeaponFlameAmount(Weapon);
	float Electro = WeaponElectroAmount(Weapon);
	
	if (From == m_pPlayer->GetCID())
	{
		if (GameServer()->m_pController->IsCoop())
			Dmg = max(1, Dmg/5);
		else
			Dmg = max(1, Dmg/2);
	}
	
	if (From >= 0)
		m_DamagedByPlayer = true;
	
	// disable self damage if weapon is forced
	//if (g_Config.m_SvForceWeapon && From == m_pPlayer->GetCID())
	//	return false;

	m_DamageTaken++;

	// damage / projectile end position
	vec2 DmgPos = m_Pos + vec2(0, -12);
	if (Pos.x != 0 && Pos.y != 0)
		DmgPos = Pos;

	if (Flame > 0.0f && Dmg > 2)
		SetAflame(Flame, From, Weapon);
		
	// create healthmod indicator
	if (m_ShieldHealth <= 0)
	{
		//if (Type == DAMAGETYPE_NORMAL)
		if (Flame == 0.0f && Electro == 0.0f && Weapon != WEAPON_ACID)
		{
			if(Server()->Tick() < m_DamageTakenTick+25)
				GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), Dmg * (m_Type == CCharacter::ROBOT ? -1 : 1), m_pPlayer->GetCID());
			else
			{
				m_DamageTaken = 0;
				GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), Dmg * (m_Type == CCharacter::ROBOT ? -1 : 1), m_pPlayer->GetCID());
			}
			
			if (m_Type == CCharacter::ROBOT && m_DamageSoundTimer <= 0)
				GameServer()->CreateBuildingHit(DmgPos);
		}
		else
		{
			GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), -Dmg, m_pPlayer->GetCID());
		}
		
		if (GetStaticType(Weapon) == SW_CHAINSAW)
			m_Core.m_Vel *= 0.9f;
		
		if (Electro > 0.0f)
			m_aStatus[STATUS_ELECTRIC] = max(1.0f*m_aStatus[STATUS_ELECTRIC], Electro*Server()->TickSpeed());
		
		/*
		else
		{
			if (Type == DAMAGETYPE_ELECTRIC)
			{
				//GameServer()->SendEffect(m_pPlayer->GetCID(), EFFECT_ELECTRODAMAGE);
				m_aStatus[STATUS_ELECTRIC] = 1.0f*Server()->TickSpeed();
			}
		
			// damage indicator but no blood
			if (Type != DAMAGETYPE_FLAME)
				GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), -Dmg, m_pPlayer->GetCID());
		}
		*/
	}
	
	if(Dmg)
	{
		//if (m_ShieldHealth > 0 && Type != DAMAGETYPE_FLAME)
		if (m_ShieldHealth > 0 && Flame == 0.0f)
		{
			GameServer()->CreateEffect(FX_SHIELDHIT, DmgPos);
			m_ShieldHealth -= Dmg + (g_Config.m_SvOneHitKill ? 1000 : 0);
			
			return false;
		}
		else
		{
			// block damage with armor
			if (m_Armor > 0 && !g_Config.m_SvOneHitKill)
			{
				int ArmorDmg = min(Dmg / 2, m_Armor);
				m_Armor -= ArmorDmg;
				Dmg -= ArmorDmg;
			}
			
			m_HiddenHealth -= Dmg + (g_Config.m_SvOneHitKill ? 1000 : 0);
			
			//if (Type == DAMAGETYPE_NORMAL)
			
			m_LatestHitVel = Force;
			
			if (Flame > 0.0f)
				GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), -Dmg, m_pPlayer->GetCID());
			
			m_Core.m_DamageTick = Server()->Tick();
		}
	}
	
	GetPlayer()->m_ActionTimer = 0;
	GetPlayer()->m_InterestPoints += Dmg * 4;

	m_DamageTakenTick = Server()->Tick();


	// do damage Hit sound
	//if (Type != DAMAGETYPE_FLAME && Weapon != DEATHTYPE_TESLACOIL && !IsTurret && m_DamageSoundTimer <= 0)
	if (Weapon != DEATHTYPE_TESLACOIL && m_DamageSoundTimer <= 0)
	{
		if(From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
		{
			m_DamageSoundTimer = 2;
			GameServer()->m_apPlayers[From]->m_InterestPoints += Dmg * 5;
			
			int Mask = CmaskOne(From);
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
					Mask |= CmaskOne(i);
			}
			GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
		}
	}

	// check for death
	if(m_HiddenHealth <= 0)
	{
		//Die(From, Weapon, false, IsTurret);
		Die(From, Weapon, false, false);

		// set attacker's face to happy (taunt!)
		if (From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
		{
			CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
			if (pChr)
			{
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
				//pChr->m_EmoteType = EMOTE_HAPPY;
				//pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
			}
		}

		return false;
	}

	if (m_PainSoundTimer <= 0 && !m_Silent)
	{
		//if (Dmg > 10 || frandom()*10 < 3)
		//	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
		//else
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);
		m_PainSoundTimer = 2;
	}

	SetEmote(EMOTE_PAIN, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
	//m_EmoteType = EMOTE_PAIN;
	//m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;
	
	return true;
}


void CCharacter::TakeSawbladeDamage(vec2 SawbladePos)
{
	if (m_ShieldHealth > 0)
	{
		GameServer()->CreateEffect(FX_SHIELDHIT, (m_Pos+SawbladePos) / 2.0f);
		m_ShieldHealth -= 5 + (g_Config.m_SvOneHitKill ? 1000 : 0);
		return;
	}
	
	m_DamageTaken++;
	
	m_Core.m_DamageTick = Server()->Tick();

	GameServer()->CreateDamageInd((m_Pos+SawbladePos) / 2.0f, GetAngle(normalize(vec2(frandom()-0.5f, frandom()-0.5f))), 3, m_pPlayer->GetCID());

	m_Core.m_Vel += normalize(m_Pos-SawbladePos)*2.0f;
	
	
	if (m_Armor > 0)
	{
		m_Armor = max(m_Armor-3, 0);
		m_HiddenHealth -= 2 + (g_Config.m_SvOneHitKill ? 1000 : 0);
	}
	else
		m_HiddenHealth -= 5 + (g_Config.m_SvOneHitKill ? 1000 : 0);
	
	m_DamageTakenTick = Server()->Tick();
	
	// check for death
	if(m_HiddenHealth <= 0)
	{
		Die(m_pPlayer->GetCID(), DEATHTYPE_SAWBLADE);
		return;
	}

	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	SetEmote(EMOTE_PAIN, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);

	if (GetPlayer()->m_pAI)
		GetPlayer()->m_pAI->ReceiveDamage(-1, 5);
}


void CCharacter::TakeDeathrayDamage()
{
	m_DamageTaken++;
	Die(m_pPlayer->GetCID(), DEATHTYPE_DEATHRAY);
}


void CCharacter::TakeDeathtileDamage()
{
	m_DamageTaken++;
	
	int top = GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y-32);
	int bot = GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y+32);
	int left = GameServer()->Collision()->GetCollisionAt(m_Pos.x-32, m_Pos.y);
	int right = GameServer()->Collision()->GetCollisionAt(m_Pos.x+32, m_Pos.y);
		
	m_Core.m_Jumped = 0;
		
	if(!top && bot)
		m_Core.m_Vel.y = -5.0f;
	if(!bot && top)
		m_Core.m_Vel.y = +5.0f;
	if(!left && right)
		m_Core.m_Vel.x = -5.0f;
	if(!right && left)
		m_Core.m_Vel.x = +5.0f;
		
	m_LatestHitVel = GetVel();


	// create healthmod indicator
	if(Server()->Tick() < m_DamageTakenTick+25)
	{
		// make sure that the damage indicators doesn't group together
		GameServer()->CreateDamageInd(m_Pos, GetAngle(normalize(vec2(frandom()-0.5f, frandom()-0.5f))), 3, m_pPlayer->GetCID());
	}
	else
	{
		GameServer()->CreateDamageInd(m_Pos, GetAngle(normalize(vec2(frandom()-0.5f, frandom()-0.5f))), 3, m_pPlayer->GetCID());
	}

	m_HiddenHealth -= 10;

	m_DamageTakenTick = Server()->Tick();
	
	
	// check for death
	if(m_HiddenHealth <= 0)
	{
		Die(m_pPlayer->GetCID(), DEATHTYPE_SPIKE);
		return;
	}

	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

	SetEmote(EMOTE_PAIN, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);

	if (GetPlayer()->m_pAI)
		GetPlayer()->m_pAI->ReceiveDamage(-1, 10);
}

void CCharacter::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, m_pPlayer->GetCID(), sizeof(CNetObj_Character)));
	if(!pCharacter)
		return;

	// write down the m_Core
	if(!m_ReckoningTick || GameServer()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pCharacter->m_Tick = 0;
		m_Core.Write(pCharacter);
	}
	else
	{
		pCharacter->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pCharacter);
	}

	// set emote
	if (m_EmoteStop < Server()->Tick())
	{
		m_EmoteType = EMOTE_NORMAL;
		m_EmoteStop = -1;
	}

	
	if (m_Core.m_DashTimer > 0)
		pCharacter->m_Movement = m_Core.m_DashTimer | m_Core.m_DashAngle<<6;
	else
		pCharacter->m_Movement = 0;
		
	
	pCharacter->m_Emote = m_EmoteType;

	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;
	pCharacter->m_Weapon = GetWeaponType();
	
	pCharacter->m_AttackTick = m_AttackTick;

	pCharacter->m_Direction = m_Input.m_Direction;
	pCharacter->m_Health = m_HiddenHealth;

	if(m_pPlayer->GetCID() == SnappingClient || SnappingClient == -1 ||
		(!g_Config.m_SvStrictSpectateMode && m_pPlayer->GetCID() == GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID))
	{

		pCharacter->m_Armor = m_Armor;

		if(GetWeapon())
			pCharacter->m_AmmoCount = GetWeapon()->GetAmmo();
		else
			pCharacter->m_AmmoCount = 0;
	}

	if(pCharacter->m_Emote == EMOTE_NORMAL)
	{
		if(250 - ((Server()->Tick() - m_LastAction)%(250)) < 5)
			pCharacter->m_Emote = EMOTE_BLINK;
	}

	// fake AI chatter flag
	//if (GetPlayer()->m_pAI && GetPlayer()->m_pAI->m_ChatterStartTick < Server()->Tick() && GetPlayer()->m_pAI->m_ChatterEndTick > Server()->Tick())
	//	pCharacter->m_PlayerFlags = PLAYERFLAG_CHATTING;
	//else
		pCharacter->m_PlayerFlags = GetPlayer()->m_PlayerFlags;
}

