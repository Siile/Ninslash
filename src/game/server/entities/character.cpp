

#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "character.h"
#include "building.h"
#include "laser.h"
#include "lightning.h"
#include "electro.h"
#include "projectile.h"
#include "superexplosion.h"
#include "landmine.h"
#include "electromine.h"
#include "monster.h"

#include <game/weapons.h>

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
	m_ProximityRadius = ms_PhysSize;
	m_HiddenHealth = 100;
	m_MaxHealth = 10;
	m_Health = 0;
	m_Armor = 0;
	m_PainSoundTimer = 0;
	m_Silent = false;
	m_WeaponGroup = 0; // primary weapon
	m_aSelectedWeapon[0] = WEAPON_GUN;
	m_aSelectedWeapon[1] = WEAPON_HAMMER;
	m_aSelectedWeapon[2] = WEAPON_TOOL;
	m_ActiveWeaponGroup = 0;
	
	for (int i = 0; i < NUM_STATUSS; i++)
	{
		m_aStatus[i] = 0;
		m_aStatusFrom[i] = 0;
		m_aStatusWeapon[i] = 0;
	}
	
	m_LastStatusEffect = 0;
	m_DeathrayTick = 0;
}

bool CCharacter::Hooking()
{
	if (m_Core.m_HookState == HOOK_GRABBED || m_Core.m_HookState == HOOK_FLYING)
		return true;
		
	return false;
}

int CCharacter::HookedPlayer()
{
	if (m_Core.m_HookState == HOOK_GRABBED && m_Core.m_HookedPlayer >= 0)
		return m_Core.m_HookedPlayer;
		
	return -1;
}


void CCharacter::ForceHook(int PlayerID)
{
	if (m_Core.m_HookState != HOOK_GRABBED)
	{
		m_Core.m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
		m_Core.m_HookState = HOOK_GRABBED;
	}
	
	//CCharacterCore *pCharCore = GameServer()->m_World.m_Core.m_apCharacters[PlayerID];
	//if(pCharCore)
	//	m_Core.m_HookPos = pCharCore->m_Pos;

	m_Core.m_HookedPlayer = PlayerID;
	//m_Core.m_HookTick = Server()->Tick() - 50;
	m_Core.m_HookTick--;
	
}


void CCharacter::Reset()
{
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	for (int i = 0; i < NUM_PLAYERITEMS; i++)
		m_aItem[i] = 0;
	
	for (int i = 0; i < NUM_STATUSS; i++)
		m_aStatus[i] = 0;
	
	m_aStatus[STATUS_SPAWNING] = 0.5f*Server()->TickSpeed();
	
	m_ShieldHealth = 0;
	m_ShieldRadius = 0;
	
	m_Recoil = vec2(0, 0);
	
	m_SkipPickups = 0;
	m_HealthStored = 0;
	
	m_SwordReady = false;
	
	m_CryTimer = 0;
	m_CryState = 0;
	
	m_EmoteLockStop = 0;
	m_DeathTileTimer = 0;
	m_DelayedKill = false;
	m_WeaponPicked = false;
	m_IsBot = false;
	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_ActiveWeapon = WEAPON_HAMMER;
	m_LastWeapon = WEAPON_HAMMER;
	m_QueuedCustomWeapon = -1;

	m_PainSoundTimer = 0;
	m_Chainsaw = 0;
	
	m_pPlayer = pPlayer;
	m_Pos = Pos;

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

	GameServer()->m_pController->OnCharacterSpawn(this, pPlayer->m_IsBot);
	
	//GiveCustomWeapon(W_TOOL);
	GiveCustomWeapon(W_HAMMER);
	GiveCustomWeapon(W_PISTOL);
	//GiveCustomWeapon(W_CHAINSAW);
	SetCustomWeapon(W_HAMMER);
	
	if (pPlayer->m_pAI)
	{
		pPlayer->m_pAI->OnCharacterSpawn(this);
		m_IsBot = true;
	}

	GiveStartWeapon();
	
	// for testing items / effects
	//SelectItem(PLAYERITEM_INVISIBILITY);
	
	return true;
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



void CCharacter::DropWeapon()
{
	/*
	if (m_ActiveCustomWeapon != W_HAMMER && m_ActiveCustomWeapon != W_PISTOL)
	{
		vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
		GameServer()->m_pController->DropPickup(m_Pos-vec2(0, 16), POWERUP_WEAPON, Direction*13, m_ActiveCustomWeapon);
		m_aWeapon[m_ActiveCustomWeapon].m_Got = false;
		m_SkipPickups = 20;
		
		SetCustomWeapon(W_HAMMER);
	}
	*/
}




void CCharacter::SetCustomWeapon(int CustomWeapon)
{
	if(CustomWeapon < 0 || CustomWeapon >= NUM_CUSTOMWEAPONS)
		return;
	
	if(CustomWeapon == m_ActiveCustomWeapon)
		return;

	m_aSelectedWeapon[m_WeaponGroup] = CustomWeapon;
	
	m_LastWeapon = m_ActiveCustomWeapon;
	m_QueuedCustomWeapon = -1;
	m_ActiveCustomWeapon = CustomWeapon;
	GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
	


	/*
	if (!m_IsBot && GetPlayer()->m_EnableWeaponInfo == 1)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Using: %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), aBuf);
	}
	
	if (!m_IsBot && GetPlayer()->m_EnableWeaponInfo == 2 && GameServer()->m_BroadcastLockTick < Server()->Tick())
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Using: %s", aCustomWeapon[CustomWeapon].m_Name);
		GameServer()->SendBroadcast(aBuf, GetPlayer()->GetCID());
	}
	*/
}


	
bool CCharacter::IsGrounded()
{
	
	if(GameServer()->Collision()->CheckPoint(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	
	int c1 = GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5);
	int c2 = GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5);
	
	if (c1&CCollision::COLFLAG_SOLID || c1&CCollision::COLFLAG_NOHOOK || c2&CCollision::COLFLAG_SOLID || c2&CCollision::COLFLAG_NOHOOK)
		return true;
	
	return false;
}




void CCharacter::DoWeaponSwitch()
{
	// make sure we can switch
	if(m_ReloadTimer > 0 || m_QueuedCustomWeapon == -1)
		return;

	// switch Weapon
	SetCustomWeapon(m_QueuedCustomWeapon);
	m_ReloadTimer = 0;
	m_aNextWeapon[aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon] = m_ActiveCustomWeapon;
}


void CCharacter::SelectWeapon(int Weapon, int Group)
{
	if (m_aSelectedWeapon[Group] != Weapon)
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
	m_aSelectedWeapon[Group] = Weapon;
}


void CCharacter::HandleWeaponSwitch()
{
	if (m_ActiveWeaponGroup < 0 || m_ActiveWeaponGroup > 1)
		m_ActiveWeaponGroup = 0;
	

	int WantedGroup = m_WeaponGroup;
	
	
	// weapon group selection
	if(m_LatestInput.m_WantedWeapon)
		WantedGroup = m_Input.m_WantedWeapon-1;
	
	if(WantedGroup >= 0 && WantedGroup < 2)
		m_WeaponGroup = WantedGroup;
	
	m_ActiveWeaponGroup = WantedGroup;
	
	
	int WantedWeapon = m_aSelectedWeapon[m_WeaponGroup];
	//if(m_QueuedCustomWeapon != -1)
	//	WantedWeapon = m_QueuedCustomWeapon;
	
	// mouse scroll
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	
	if(Next < 128) // make sure we only try sane stuff
	{
		while(Next) // Next Weapon selection
		{
			WantedWeapon = (WantedWeapon+1)%NUM_CUSTOMWEAPONS;
			if(m_aWeapon[WantedWeapon].m_Got && !m_aWeapon[WantedWeapon].m_Disabled)
				Next--;
		}
	}

	if(Prev < 128) // make sure we only try sane stuff
	{
		while(Prev) // Prev Weapon selection
		{
			WantedWeapon = (WantedWeapon-1)<0?NUM_CUSTOMWEAPONS-1:WantedWeapon-1;
			if(m_aWeapon[WantedWeapon].m_Got && !m_aWeapon[WantedWeapon].m_Disabled)
				Prev--;
		}
	}
	
	m_aSelectedWeapon[m_WeaponGroup] = WantedWeapon;
	m_QueuedCustomWeapon = m_aSelectedWeapon[m_WeaponGroup];

	DoWeaponSwitch();
}





int CCharacter::GetWeapon(int ParentType)
{
	if (m_aNextWeapon[ParentType] < 0 || m_aNextWeapon[ParentType] > NUM_CUSTOMWEAPONS)
	{
		m_aNextWeapon[ParentType] = 0;
		return GetNextWeapon(ParentType);
	}
	
	if (aCustomWeapon[m_aNextWeapon[ParentType]].m_ParentWeapon == ParentType)
		return m_aNextWeapon[ParentType];
		
	else
		return GetNextWeapon(ParentType);
}

int CCharacter::GetPrevWeapon(int ParentType)
{
	int w = m_aNextWeapon[ParentType];
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS+1; i++)
	{
		if (--w < 0)
			w = NUM_CUSTOMWEAPONS;
			
		if (aCustomWeapon[w].m_ParentWeapon == ParentType && m_aWeapon[w].m_Got && !m_aWeapon[w].m_Disabled)
		{
			m_aNextWeapon[ParentType] = w;
			return w;
		}
	}
	
	return -1;
}

int CCharacter::GetNextWeapon(int ParentType)
{
	int w = m_aNextWeapon[ParentType];
	
	for (int i = 0; i < NUM_CUSTOMWEAPONS+1; i++)
	{
		if (++w >= NUM_CUSTOMWEAPONS)
			w = 0;
			
		if (aCustomWeapon[w].m_ParentWeapon == ParentType && m_aWeapon[w].m_Got && !m_aWeapon[w].m_Disabled)
		{
			m_aNextWeapon[ParentType] = w;
			return w;
		}
	}
	
	return -1;
}


int CCharacter::GetFirstWeapon(int ParentType)
{
	for (int i = 0; i < NUM_CUSTOMWEAPONS; i++)
	{
		if (aCustomWeapon[i].m_ParentWeapon == ParentType && m_aWeapon[i].m_Got && !m_aWeapon[i].m_Disabled)
		{
			return i;
		}
	}
	
	return -1;
}


void CCharacter::ScanWeapons()
{
	for (int i = 0; i < NUM_WEAPONS; i++)
	{
		int w = m_aNextWeapon[i];
		if (w < 0 || w >= NUM_CUSTOMWEAPONS)
		{
			m_aNextWeapon[i] = GetFirstWeapon(i);
			continue;
		}
		
		if (m_aWeapon[m_aNextWeapon[i]].m_Got && !m_aWeapon[m_aNextWeapon[i]].m_Disabled)
		{
			continue;
		}
		else
		{
			m_aNextWeapon[i] = GetFirstWeapon(i);
			continue;
		}
	}
}



void CCharacter::Chainsaw()
{	
	if (aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_CHAINSAW && m_Chainsaw >= Server()->Tick())
	{
		GetPlayer()->m_InterestPoints += 3;
		
		// massacre
		vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
		vec2 ProjStartPos = m_Core.m_Vel*2 +m_Pos+Direction*m_ProximityRadius*1.9f + vec2(0, -11);
		
		// test
		//GameServer()->CreateBuildingHit(ProjStartPos);
		
		{
		CCharacter *apEnts[MAX_CLIENTS];
			int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*0.5f, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; ++i)
			{
				CCharacter *pTarget = apEnts[i];

				if ((pTarget == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
					continue;

				vec2 Dir;
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);

				pTarget->TakeDamage(Dir*1.2f, aCustomWeapon[m_ActiveCustomWeapon].m_Damage,
									m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, ProjStartPos, false);
			}
		}
		
		// monsters
		{
			CMonster *apEnts[MAX_CLIENTS];
			int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*0.5f, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_MONSTER);

			for (int i = 0; i < Num; ++i)
			{
				CMonster *pTarget = apEnts[i];

				if (pTarget->m_Health <= 0)
					continue;

				vec2 Dir;
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);

				pTarget->TakeDamage(Dir*1.2f, aCustomWeapon[m_ActiveCustomWeapon].m_Damage, m_pPlayer->GetCID(), vec2(0, 0));
			}
		}
	}
	else
		m_Chainsaw = 0;
}


void CCharacter::FireWeapon()
{
	if (m_aStatus[STATUS_SPAWNING] > 0.0f)
		return;
	
	Chainsaw();
	DoWeaponSwitch();
	
	if(m_ReloadTimer > 0)
		return;
	
	
	// sword requires standing on ground before it can be reused
	if ((aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_SWORD || aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType == PROJTYPE_FLYHAMMER) && !m_SwordReady)
		return;
	
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = aCustomWeapon[m_ActiveCustomWeapon].m_FullAuto;
	
	if (m_IsBot)
		FullAuto = true;
	
	//bool UseAmmo = true;
	//if (aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo <= 0)
	//		UseAmmo = false;

	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1))
		WillFire = true;

	if(!WillFire)
		return;

		
	// check for ammo
	if(m_aWeapon[m_ActiveCustomWeapon].m_Ammo <= 0 && aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo > 0)
	{
		// 125ms is a magical limit of how fast a human can click
		m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
		if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
		{
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
			m_LastNoAmmoSound = Server()->Tick();
		}

		return;
	}
	
	// weapon knockback to self
	//m_Core.m_Vel -= Direction * aCustomWeapon[m_ActiveCustomWeapon].m_SelfKnockback;
	m_Recoil -= Direction * aCustomWeapon[m_ActiveCustomWeapon].m_SelfKnockback;

	vec2 ProjStartPos = m_Pos+Direction*m_ProximityRadius*0.75f + vec2(0, -11);

	// play sound
	if (aCustomWeapon[m_ActiveCustomWeapon].m_Sound >= 0)
		GameServer()->CreateSound(m_Pos, aCustomWeapon[m_ActiveCustomWeapon].m_Sound);
	
	
	int Damage = aCustomWeapon[m_ActiveCustomWeapon].m_Damage;
	
	m_ReloadTimer = aCustomWeapon[m_ActiveCustomWeapon].m_BulletReloadTime * Server()->TickSpeed() / 1000;
	
	
	// create the projectile
	switch(aCustomWeapon[m_ActiveCustomWeapon].m_ProjectileType)
	{
		// todo: clean this
		case PROJTYPE_CHAINSAW:
		{
			m_Chainsaw = Server()->Tick() + 500 * Server()->TickSpeed()/1000;
			break;
		}
			
		case PROJTYPE_HAMMER:
		{
			GetPlayer()->m_InterestPoints += 10;
			
			// reset objects Hit
			m_NumObjectsHit = 0;

			vec2 ProjStartPos = m_Core.m_Vel*3 +m_Pos+Direction*m_ProximityRadius*2.5f + vec2(0, -20);
			
			// for testing the center pos
			//GameServer()->CreateEffect(FX_ELECTROHIT, ProjStartPos);
			
			{
				CCharacter *apEnts[MAX_CLIENTS];
				int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*2.5f, (CEntity**)apEnts,
															MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

				for (int i = 0; i < Num; ++i)
				{
					CCharacter *pTarget = apEnts[i];

					if (pTarget == this || pTarget->IgnoreCollision()) // || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL)
						continue;
					
					if(length(pTarget->m_Pos-ProjStartPos) > 0.0f)
						GameServer()->CreateEffect(FX_BLOOD1, pTarget->m_Pos-normalize(pTarget->m_Pos-ProjStartPos)*m_ProximityRadius*2.5f);
					else
						GameServer()->CreateEffect(FX_BLOOD1, ProjStartPos);
						
					vec2 Dir;
					if (length(pTarget->m_Pos - m_Pos) > 0.0f)
						Dir = normalize(pTarget->m_Pos - m_Pos);
					else
						Dir = vec2(0.f, 0.f);

					pTarget->TakeDamage(Dir * 10.0f * aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, Damage,
						m_pPlayer->GetCID(), aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, vec2(0, 0), false);
				}
			}
			
			// monster collision
			{
				CMonster *apEnts[MAX_CLIENTS];
				int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*2.5f, (CEntity**)apEnts,
															MAX_CLIENTS, CGameWorld::ENTTYPE_MONSTER);

				for (int i = 0; i < Num; ++i)
				{
					CMonster *pTarget = apEnts[i];

					if (pTarget->m_Health <= 0)
						continue;
					
					if(length(pTarget->m_Pos-ProjStartPos) > 0.0f)
						GameServer()->CreateEffect(FX_BLOOD1, pTarget->m_Pos-normalize(pTarget->m_Pos-ProjStartPos)*m_ProximityRadius*2.5f);
					else
						GameServer()->CreateEffect(FX_BLOOD1, ProjStartPos);
						
					vec2 Dir;
					if (length(pTarget->m_Pos - m_Pos) > 0.0f)
						Dir = normalize(pTarget->m_Pos - m_Pos);
					else
						Dir = vec2(0.f, 0.f);

					pTarget->TakeDamage(Dir * 10.0f, Damage, m_pPlayer->GetCID(), vec2(0, 0));
				}
			}


		} break;
		
		case PROJTYPE_BULLET:
		{
			GetPlayer()->m_InterestPoints += 10;	
			
			float a = GetAngle(Direction);
			a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;
			
			CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_GUN,
				m_pPlayer->GetCID(),
				ProjStartPos,
				vec2(cosf(a), sinf(a)),
				(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
				Damage, 0, aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, -1, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);

			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(1);
			for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);

			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
		} break;

		case PROJTYPE_ELECTRIC:
		{
			GetPlayer()->m_InterestPoints += 15;
			
			float a = GetAngle(Direction);
			a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;
			
			CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_ELECTRIC,
				m_pPlayer->GetCID(),
				ProjStartPos,
				Direction,
				(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
				Damage, EXPLOSION_ELECTRIC, 0, SOUND_LASER_FIRE, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);
				
			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(1);
			for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);

			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
		} break;

		case PROJTYPE_PELLET:
		{
			GetPlayer()->m_InterestPoints += 30;
			
			// int ShotSpread = 2;

			// CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			// Msg.AddInt(ShotSpread*2+1);

			int ShotSpread = aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread / 2;
			
			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread);
				
			if (aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread%2)
			{
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+2];
					a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;
					float v = 1-(absolute(i)/(float)ShotSpread);
					float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
					CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_SHOTGUN,
						m_pPlayer->GetCID(),
						ProjStartPos,
						vec2(cosf(a), sinf(a))*Speed,
						//(int)(Server()->TickSpeed()*GameServer()->Tuning()->m_ShotgunLifetime), // 0.2f
						(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
						Damage, 0, aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, -1, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
					
					// pack the Projectile and send it to the client Directly
					CNetObj_Projectile p;
					pProj->FillInfo(&p);

					for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
						Msg.AddInt(((int *)&p)[i]);
				}
			}
			else
			{
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.130f, -0.050f, 0.050f, 0.130f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+3];
					a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;
					float v = 1-(absolute(i)/(float)ShotSpread);
					float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
					CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_SHOTGUN,
						m_pPlayer->GetCID(),
						ProjStartPos,
						vec2(cosf(a), sinf(a))*Speed,
						(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
						Damage, 0, aCustomWeapon[m_ActiveCustomWeapon].m_Knockback, -1, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon);
					
					// pack the Projectile and send it to the client Directly
					CNetObj_Projectile p;
					pProj->FillInfo(&p);

					for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
						Msg.AddInt(((int *)&p)[i]);
				}
			}

			Server()->SendMsg(&Msg, 0,m_pPlayer->GetCID());
		} break;

		case PROJTYPE_GRENADE:
		{
			GetPlayer()->m_InterestPoints += 30;
			
			CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_GRENADE,
				m_pPlayer->GetCID(),
				ProjStartPos,
				Direction,
				(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
				Damage, EXPLOSION_EXPLOSION, 0, SOUND_GRENADE_EXPLODE, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);

			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(1);
			for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);
			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
		} break;
		
		case PROJTYPE_FLAME:
		{
			GetPlayer()->m_InterestPoints += 30;
			
			// x3 for flame shotgun / flamer
			for (int i=0; i<3; i++)
			{
				CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_FLAMER,
					m_pPlayer->GetCID(),
					ProjStartPos,
					Direction+RandomDir()*0.15f,
					(int)(Server()->TickSpeed()*aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife),
					Damage, EXPLOSION_FLAME, 0, SOUND_GRENADE_EXPLODE, aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);

				// pack the Projectile and send it to the client Directly
				CNetObj_Projectile p;
				pProj->FillInfo(&p);

				CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
				Msg.AddInt(1);
				for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
					Msg.AddInt(((int *)&p)[i]);
				Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
			}
		} break;

		case PROJTYPE_LASER:
		{
			GetPlayer()->m_InterestPoints += 40;
			
			float a = GetAngle(Direction);
			a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;
			
			new CLaser(GameWorld(), ProjStartPos, vec2(cosf(a), sinf(a)), GameServer()->Tuning()->m_LaserReach, m_pPlayer->GetCID(), Damage, aCustomWeapon[m_ActiveCustomWeapon].m_Extra1);
		} break;
		
		
		case PROJTYPE_ELECTRO:
		{
			GetPlayer()->m_InterestPoints += 40;
			vec2 Start = m_Pos;
			
			if (aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon == WEAPON_GUN)
				Start += Direction*30;
			if (aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon == WEAPON_RIFLE)
				Start += Direction*50;
			
			float Reach = aCustomWeapon[m_ActiveCustomWeapon].m_BulletLife;
			
			
			if (aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread == 1)
			{
				float a = GetAngle(Direction);
				
				vec2 To = m_Pos + vec2(cosf(a), sinf(a))*Reach;
				GameServer()->Collision()->IntersectLine(Start, To, 0x0, &To);
				
				// character collision
				vec2 At;
				CCharacter *pHit = GameServer()->m_World.IntersectCharacter(Start, To, 70.0f, At, this);
				if(pHit)
				{
					To = pHit->m_Pos;
					pHit->ElectroShock();
					pHit->TakeDamage(Direction, Damage, GetPlayer()->GetCID(), WEAPON_RIFLE, At);
				}
			
					int A = distance(Start, To) / 100;
					
					if (A > 4)
						A = 4;
					
					if (A < 2)
						A = 2;
			
				new CElectro(GameWorld(), Start, To, vec2(0, 0), A);
			}
			else
			{
				for (int i = -1; i < 2; i += 2)
				{
					
					float a = GetAngle(Direction);
					a += (i + frandom()-frandom()) / 10.0f;
					//a += i / 10.0f;
					vec2 To = m_Pos + vec2(cosf(a), sinf(a))*Reach;
					
					GameServer()->Collision()->IntersectLine(Start, To, 0x0, &To);
					
					// character collision
					vec2 At;
					CCharacter *pHit = GameServer()->m_World.IntersectCharacter(Start, To, 70.0f, At, this);
					if(pHit)
					{
						To = pHit->m_Pos;
						pHit->ElectroShock();
						pHit->TakeDamage(Direction, Damage, GetPlayer()->GetCID(), WEAPON_RIFLE, At);
					}
			
					int A = distance(Start, To) / 100;
					
					if (A > 4)
						A = 4;
					
					if (A < 2)
						A = 2;
			
					new CElectro(GameWorld(), Start, To, vec2(cosf(a+i*1.2f), sinf(a+i*1.2f))*40, A);
				}
			}
			break;
		}
			
			
		case PROJTYPE_LIGHTNING:
		{
			GetPlayer()->m_InterestPoints += 10;
		
			int Desc = 1;
			
			if (aCustomWeapon[m_ActiveCustomWeapon].m_Extra1 == ELECTRIC)
				Desc++;
		
			int ShotSpread = aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread / 2;

			if (ShotSpread == 1)
			{
				float a = GetAngle(Direction);
				a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;

				new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 200, 100, m_pPlayer->GetCID(), Damage, Desc);
			}
			else
			{
			// for lightning shotgun, might not work 100% right
				if (aCustomWeapon[m_ActiveCustomWeapon].m_ShotSpread%2)
				{
					for(int i = -ShotSpread; i <= ShotSpread; ++i)
					{
						float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
						float a = GetAngle(Direction);
						a += Spreading[i+2];
						a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;

						new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 200, 100, m_pPlayer->GetCID(), Damage, Desc);
					}
				}
				else
				{
					for(int i = -ShotSpread; i <= ShotSpread; ++i)
					{
						float Spreading[] = {-0.185f, -0.130f, -0.050f, 0.050f, 0.130f, 0.185f};
						float a = GetAngle(Direction);
						a += Spreading[i+3];
						a += (frandom()-frandom())*aCustomWeapon[m_ActiveCustomWeapon].m_BulletSpread;
						
						new CLightning(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), 200, 100, m_pPlayer->GetCID(), Damage, Desc);
					}
				}
			}
		} break;
	};
	
	

	m_AttackTick = Server()->Tick();

	if(m_aWeapon[m_ActiveCustomWeapon].m_Ammo > 0)
		m_aWeapon[m_ActiveCustomWeapon].m_Ammo--;

	/*
	if(m_ReloadTimer <= 0)
	{
		m_ReloadTimer = aCustomWeapon[m_ActiveCustomWeapon].m_BulletReloadTime * Server()->TickSpeed() / 1000;
	}
	*/
	
}


	
	
void CCharacter::HandleWeapons()
{
	ShowArmor();

	
	if(m_ReloadTimer > 0)
	{
		m_ReloadTimer--;
		return;
	}
	
	// fire Weapon, if wanted
	FireWeapon();

	return;
}



void CCharacter::ShowArmor()
{
	m_Armor = 0;
}







void CCharacter::AutoWeaponChange()
{
	//if (HasAmmo() && frandom()*100 > 4 && m_ActiveCustomWeapon != W_HAMMER && m_ActiveCustomWeapon != W_PISTOL && m_ActiveCustomWeapon != W_TOOL)
	if (HasAmmo() && frandom()*100 > 4 && m_ActiveCustomWeapon != W_TOOL)// && m_ActiveCustomWeapon != W_PISTOL && m_ActiveCustomWeapon != W_TOOL)
		return;
	
	// -1 because smoke grenade shouldn't be included
	int w = rand()%(NUM_CUSTOMWEAPONS);
	
	if (m_aWeapon[w].m_Got && !m_aWeapon[w].m_Disabled)
	{
		if (m_aWeapon[w].m_Ammo > 0 || aCustomWeapon[w].m_MaxAmmo == 0)
		{
			SetCustomWeapon(w);
		}
	}
}




void CCharacter::GiveStartWeapon()
{
	if (g_Config.m_SvRandomWeapons)
	{
		GiveRandomWeapon();
		//GiveRandomWeapon();
	}
	
	//GiveAllWeapons();
}



void CCharacter::GiveRandomWeapon(int WeaponLevel)
{
	int w = 0;
	while (w == W_TOOL || w == W_HAMMER || w == W_PISTOL)
		w = rand()%(NUM_CUSTOMWEAPONS);
	
	GiveCustomWeapon(w);
	SetCustomWeapon(w);
}

void CCharacter::GiveAllWeapons()
{
	for (int w = 0; w < NUM_CUSTOMWEAPONS; w++)
		GiveCustomWeapon(w);
}



bool CCharacter::GiveAmmo(int *CustomWeapon, float AmmoFill)
{
	if(m_aWeapon[*CustomWeapon].m_Got)
	{
		if (m_aWeapon[*CustomWeapon].m_Ammo < aCustomWeapon[*CustomWeapon].m_MaxAmmo)
		{
			m_aWeapon[*CustomWeapon].m_Ammo += aCustomWeapon[*CustomWeapon].m_MaxAmmo * AmmoFill;
			
			if (m_aWeapon[*CustomWeapon].m_Ammo > aCustomWeapon[*CustomWeapon].m_MaxAmmo)
				m_aWeapon[*CustomWeapon].m_Ammo = aCustomWeapon[*CustomWeapon].m_MaxAmmo;
			
			return true;
		}
	}
	
	return false;
}


bool CCharacter::GiveCustomWeapon(int CustomWeapon, float AmmoFill)
{
	if(!m_aWeapon[CustomWeapon].m_Got && !m_aWeapon[CustomWeapon].m_Disabled)
	{	
		m_aWeapon[CustomWeapon].m_Got = true;
		m_aWeapon[CustomWeapon].m_Disabled = false;
		m_aWeapon[CustomWeapon].m_Ready = false;
		m_aWeapon[CustomWeapon].m_Ammo = aCustomWeapon[CustomWeapon].m_MaxAmmo;
		
		//bool SkipAmmoFill = false;
				
		// ammo fill
		m_aWeapon[CustomWeapon].m_Ammo = aCustomWeapon[CustomWeapon].m_MaxAmmo * AmmoFill;
		
		
		ScanWeapons();

		return true;
	}
	return false;
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
	// simulate releasing the fire button
	if((m_Input.m_Fire&1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}




void CCharacter::SelectItem(int Item)
{
	if (m_aItem[Item] <= 0)
		return;
	
	if (Item == PLAYERITEM_HEAL && m_aStatus[STATUS_HEAL] <= 0)
	{
		m_aStatus[STATUS_HEAL] = Server()->TickSpeed() * 0.65f;
		m_aItem[Item]--;
	}

	if (Item == PLAYERITEM_RAGE && m_aStatus[STATUS_RAGE] <= 0)
	{
		m_aStatus[STATUS_RAGE] = Server()->TickSpeed() * 15.0f;
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
		m_aStatus[STATUS_SHIELD] = Server()->TickSpeed() * 15.0f;
		m_ShieldHealth = 70;
		m_ShieldRadius = 16;
		m_aItem[Item]--;
	}
	
	if (Item == PLAYERITEM_INVISIBILITY && m_aStatus[STATUS_INVISIBILITY] <= 0)
	{
		m_aStatus[STATUS_INVISIBILITY] = Server()->TickSpeed() * 15.0f;
		m_aItem[Item]--;
	}
}

void CCharacter::GiveBuff(int Item)
{
	if (Item == PLAYERITEM_HEAL)
		m_aStatus[STATUS_HEAL] = Server()->TickSpeed() * 0.65f;

	if (Item == PLAYERITEM_RAGE)
		m_aStatus[STATUS_RAGE] = Server()->TickSpeed() * 15.0f;
	
	
	if (Item == PLAYERITEM_SHIELD)
	{
		m_aStatus[STATUS_SHIELD] = Server()->TickSpeed() * 15.0f;
		m_ShieldHealth = 70;
		m_ShieldRadius = 16;
	}
	
	if (Item == PLAYERITEM_INVISIBILITY)
		m_aStatus[STATUS_INVISIBILITY] = Server()->TickSpeed() * 15.0f;
}



void CCharacter::UpdateCoreStatus()
{
	m_Core.m_Status = 0;
	
	// end shield effect when needed
	if (m_ShieldHealth <= 0 || m_aStatus[STATUS_SHIELD] <= 0)
	{
		m_ShieldHealth = 0;
		m_aStatus[STATUS_SHIELD] = 0;
		m_ShieldRadius = 0;
	}
	
	if (m_aStatus[STATUS_HEAL] > 0)
	{
		IncreaseHealth(2);
	}
	
	for (int i = 0; i < NUM_STATUSS; i++)
	{
		if (m_aStatus[i] > 0)
		{
			m_Core.m_Status |= 1 << i;
			m_aStatus[i]--;
		}
	}
	
	if(m_LastStatusEffect+Server()->TickSpeed()/3 <= Server()->Tick())
	{
		m_LastStatusEffect = Server()->Tick();
		
		 // flame damage
		if (m_aStatus[STATUS_AFLAME] > 0)
			TakeDamage(vec2(0, 0), 4, m_aStatusFrom[STATUS_AFLAME], m_aStatusWeapon[STATUS_AFLAME], vec2(0, 0), DAMAGETYPE_FLAME);
	}
}


void CCharacter::Tick()
{
	if (m_PainSoundTimer > 0)
		m_PainSoundTimer--;
	
	if (m_SkipPickups > 0)
		m_SkipPickups--;
	
	
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
	
	if (GameServer()->m_FreezeCharacters)
		ResetInput();
	
	m_Core.m_Input = m_Input;

	float RecoilCap = 15.0f;
	if ((m_Core.m_Vel.x < RecoilCap && m_Recoil.x > 0) || (m_Core.m_Vel.x > -RecoilCap && m_Recoil.x < 0))
		m_Core.m_Vel.x += m_Recoil.x*0.7f;
	
	if ((m_Core.m_Vel.y < RecoilCap && m_Recoil.y > 0) || (m_Core.m_Vel.y > -RecoilCap && m_Recoil.y < 0))
		m_Core.m_Vel.y += m_Recoil.y*0.7f;
	//m_Core.m_Vel += m_Recoil*0.7f;
	
	m_Recoil *= 0.5f;
	
	m_Core.Tick(true);
	
	// monster damage
	if (m_Core.m_MonsterDamage)
		TakeDamage(normalize(m_Core.m_Vel), 10, -1, DEATHTYPE_MONSTER, vec2(0, 0));
	
	if (IsGrounded())
		m_SwordReady = true;
	
	
	if (m_CryTimer > 0)
		m_CryTimer--;

	// handle death-tiles and leaving gamelayer
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f-24)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f-24)&CCollision::COLFLAG_DEATH ||
		GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
		GameLayerClipped(m_Pos))
	{
		if (g_Config.m_SvInstaDeathTiles)
			Die(m_pPlayer->GetCID(), DEATHTYPE_SPIKE);
		else
		{
			m_DeathTileTimer = 10;
			TakeDeathtileDamage();
		}
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
	if (!GameServer()->m_FreezeCharacters)
		HandleWeapons();

	// Previnput
	m_PrevInput = m_Input;
	
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
	bool StuckBefore = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.Move();
	bool StuckAfterMove = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	bool StuckAfterQuant = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

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

	int Events = m_Core.m_TriggeredEvents;
	int Mask = CmaskAllExceptOne(m_pPlayer->GetCID());

	if(Events&COREEVENT_GROUND_JUMP) GameServer()->CreateSound(m_Pos, SOUND_PLAYER_JUMP, Mask);

	if(Events&COREEVENT_HOOK_ATTACH_PLAYER) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_PLAYER, CmaskAll());
	if(Events&COREEVENT_HOOK_ATTACH_GROUND) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_GROUND, Mask);
	if(Events&COREEVENT_HOOK_HIT_NOHOOK) GameServer()->CreateSound(m_Pos, SOUND_HOOK_NOATTACH, Mask);


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
		if(m_ReckoningTick+Server()->TickSpeed()*3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
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
	++m_Ninja.m_ActivationTick;
	++m_ReckoningTick;
	if(m_LastAction != -1)
		++m_LastAction;
	if(m_EmoteStop > -1)
		++m_EmoteStop;
}

bool CCharacter::HookGrabbed()
{
	if (m_Core.m_HookState == HOOK_GRABBED)
		return true;
		
	return false;
}

bool CCharacter::HookGrabbedToPlayer()
{
	if (m_Core.m_HookState == HOOK_GRABBED && m_Core.m_HookedPlayer >= 0)
		return true;
		
	return false;
}

void CCharacter::SetHealth(int Health)
{
	m_MaxHealth = Health;
	m_HiddenHealth = Health;
}

bool CCharacter::IncreaseHealth(int Amount)
{
	if(m_HiddenHealth >= m_MaxHealth)
		return false;
	m_HiddenHealth = clamp(m_HiddenHealth+Amount, 0, m_MaxHealth);
	
	//GetPlayer()->m_InterestPoints += 40;
	
	return true;
}



// use armor points as clips
bool CCharacter::AddClip()
{
	if (aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize <= 0 || aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo <= 0)
		return false;

	if (m_aWeapon[m_ActiveCustomWeapon].m_Ammo < aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo)
	{
		m_aWeapon[m_ActiveCustomWeapon].m_Ammo = min(m_aWeapon[m_ActiveCustomWeapon].m_Ammo+aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize, aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo);
		GetPlayer()->m_InterestPoints += 40;
		return true;
	}
	
	return false;
}


// use armor points as clips
bool CCharacter::IncreaseArmor(int Amount)
{
	return AddClip();
	
	/*
	if (aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize <= 0 || aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo <= 0)
		return false;

	if (m_aWeapon[m_ActiveCustomWeapon].m_Ammo <= aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo)
	{
		m_aWeapon[m_ActiveCustomWeapon].m_Ammo = min(m_aWeapon[m_ActiveCustomWeapon].m_Ammo+aCustomWeapon[m_ActiveCustomWeapon].m_PowerupSize, aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo);
		GetPlayer()->m_InterestPoints += 40;
		return true;
	}
	
	return false;
	*/
}



void CCharacter::Die(int Killer, int Weapon, bool SkipKillMessage)
{
	//if (Weapon < 0)
	//	Weapon = 0;
	// we got to wait 0.5 secs before respawning
	//m_pPlayer->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
	m_pPlayer->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*3;
	
	if (Killer == m_pPlayer->GetCID() && (Weapon == WEAPON_HAMMER || Weapon == WEAPON_GAME))
		SkipKillMessage = true;
	
	if (Killer == NEUTRAL_BASE)
		Killer = GetPlayer()->GetCID();
	
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
	
	// a nice sound
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_DIE);

	// this is for auto respawn after 3 secs
	m_pPlayer->m_DieTick = Server()->Tick();

	m_Alive = false;
	GameServer()->m_World.RemoveEntity(this);
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	
	GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID());
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


void CCharacter::Deathray()
{
	if (m_DeathrayTick > 0)
		return;
	
	m_DeathrayTick = Server()->Tick() + Server()->TickSpeed() * 0.2f;
	m_aStatus[STATUS_DEATHRAY] = 10.0f*Server()->TickSpeed();
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


bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon, vec2 Pos, int Type)
{
	// skip everything while spawning
	if (m_aStatus[STATUS_SPAWNING] > 0.0f)
		return false;

	if (m_ShieldHealth <= 0)
		m_Recoil += Force;
	
	// signal AI
	if (Dmg > 0 && GetPlayer()->m_pAI && Weapon >= 0)
		GetPlayer()->m_pAI->ReceiveDamage(From, Dmg);
	
	if(GameServer()->m_pController->IsFriendlyFire(m_pPlayer->GetCID(), From) && !g_Config.m_SvTeamdamage)
		return false;
		
	// m_pPlayer only inflicts half damage on self
	if (Type != DAMAGETYPE_FLAME)
	{
		if(From == m_pPlayer->GetCID())
			Dmg = max(1, Dmg/2);
	}

	m_DamageTaken++;

	// damage / projectile end position
	vec2 DmgPos = m_Pos + vec2(0, -12);
	if (Pos.x != 0 && Pos.y != 0)
		DmgPos = Pos;

	// create healthmod indicator
	if (m_ShieldHealth <= 0)
	{
		if (Type == DAMAGETYPE_NORMAL)
		{
			if(Server()->Tick() < m_DamageTakenTick+25)
				GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), Dmg);
			else
			{
				m_DamageTaken = 0;
				GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), Dmg);
			}
		}
		else
		if (Type == DAMAGETYPE_ELECTRIC)
		{
			//GameServer()->SendEffect(m_pPlayer->GetCID(), EFFECT_ELECTRODAMAGE);
			m_aStatus[STATUS_ELECTRIC] = 1.0f*Server()->TickSpeed();
		}
	}
	
	if(Dmg)
	{
		if (m_ShieldHealth > 0 && Type != DAMAGETYPE_FLAME)
		{
			GameServer()->CreateEffect(FX_SHIELDHIT, DmgPos);
			m_ShieldHealth -= Dmg;
			return false;
		}
		else
		{
			m_HiddenHealth -= Dmg;
			
			if (Type == DAMAGETYPE_NORMAL)
				m_LatestHitVel = Force;
			
			m_Core.m_DamageTick = Server()->Tick();
		}
	}
	
	GetPlayer()->m_ActionTimer = 0;
	GetPlayer()->m_InterestPoints += Dmg * 4;

	m_DamageTakenTick = Server()->Tick();


	// do damage Hit sound
	if(From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
	{
		GameServer()->m_apPlayers[From]->m_InterestPoints += Dmg * 5;
		
		int Mask = CmaskOne(From);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
				Mask |= CmaskOne(i);
		}
		GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
	}

	// check for death
	if(m_HiddenHealth <= 0)
	{
		Die(From, Weapon);

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
		if (Dmg > 10 || frandom()*10 < 3)
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
		else
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
	m_DamageTaken++;
	
	m_Core.m_DamageTick = Server()->Tick();

	GameServer()->CreateDamageInd((m_Pos+SawbladePos) / 2.0f, GetAngle(normalize(vec2(frandom()-0.5f, frandom()-0.5f))), 3);

	m_Core.m_Vel += normalize(m_Pos-SawbladePos)*2.0f;
	
	m_HiddenHealth -= 5;
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
		GameServer()->CreateDamageInd(m_Pos, GetAngle(normalize(vec2(frandom()-0.5f, frandom()-0.5f))), 3);
	}
	else
	{
		GameServer()->CreateDamageInd(m_Pos, GetAngle(normalize(vec2(frandom()-0.5f, frandom()-0.5f))), 3);
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

	pCharacter->m_Emote = m_EmoteType;

	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;

	pCharacter->m_Weapon = aCustomWeapon[m_ActiveCustomWeapon].m_ParentWeapon;
	
	pCharacter->m_WeaponGroup1 = m_aSelectedWeapon[0];
	pCharacter->m_WeaponGroup2 = m_aSelectedWeapon[1];
	
	pCharacter->m_SelectedGroup = m_ActiveWeaponGroup;
	
	pCharacter->m_AttackTick = m_AttackTick;

	pCharacter->m_Direction = m_Input.m_Direction;

	if(m_pPlayer->GetCID() == SnappingClient || SnappingClient == -1 ||
		(!g_Config.m_SvStrictSpectateMode && m_pPlayer->GetCID() == GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID))
	{
		pCharacter->m_Health = int(float(m_HiddenHealth)/float(m_MaxHealth) * 10.0f);
		if (pCharacter->m_Health < 1)
			pCharacter->m_Health = 1;

		pCharacter->m_Armor = m_Armor;
			
		if(m_aWeapon[m_ActiveCustomWeapon].m_Ammo > 0)
		{
			//pCharacter->m_AmmoCount = (m_aWeapon[m_ActiveCustomWeapon].m_Ammo * (10.0f / aCustomWeapon[m_ActiveCustomWeapon].m_MaxAmmo));
			pCharacter->m_AmmoCount = m_aWeapon[m_ActiveCustomWeapon].m_Ammo;
		}
	}

	if(pCharacter->m_Emote == EMOTE_NORMAL)
	{
		if(250 - ((Server()->Tick() - m_LastAction)%(250)) < 5)
			pCharacter->m_Emote = EMOTE_BLINK;
	}

	// fake AI chatter flag
	if (GetPlayer()->m_pAI && GetPlayer()->m_pAI->m_ChatterStartTick < Server()->Tick() && GetPlayer()->m_pAI->m_ChatterEndTick > Server()->Tick())
		pCharacter->m_PlayerFlags = PLAYERFLAG_CHATTING;
	else
		pCharacter->m_PlayerFlags = GetPlayer()->m_PlayerFlags;
}

