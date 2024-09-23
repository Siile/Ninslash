#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/weapons.h>
#include "pickup.h"
#include "weapon.h"
#include "electro.h"
#include "superexplosion.h"


CPickup::CPickup(CGameWorld *pGameWorld, int Type, int SubType, int Ammo)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Type = Type;
	m_Subtype = SubType;
	m_ProximityRadius = PickupPhysSize;
	
	if (Type == POWERUP_COIN)
		m_BoxSize = 14;
	else
		m_BoxSize = 24;

	m_ResetableDropable = false;
	m_pWeapon = NULL;
	
	Reset();

	GameWorld()->InsertEntity(this);
	
	if (g_Config.m_SvSurvivalMode)
	{
		m_SkipAutoRespawn = true;
	}
	else
		m_SkipAutoRespawn = false;
	
	m_Dropable = false;
	m_Life = 0;
	m_Vel = vec2(0, 0);
	m_Ammo = Ammo;
}

void CPickup::Reset()
{
	m_Mirror = false;
	if (!m_SkipAutoRespawn)
	{
		if (g_pData->m_aPickups[m_Type].m_Spawndelay > 0)
			m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * g_pData->m_aPickups[m_Type].m_Spawndelay;
		else
			m_SpawnTick = -1;
			
		m_Flashing = false;
		m_FlashTimer = 0;
		
		m_AngleForce = 0.0f;
		m_Angle = 0.0f;
	
		ClearWeapon();
		
		if (m_Type == POWERUP_WEAPON && GetStaticType(m_Subtype) != SW_BALL)
			SetRandomWeapon();
	}
}

void CPickup::SetRandomWeapon()
{
	if (GetStaticType(m_Subtype) == SW_BALL)
		return;
	
	m_Subtype = GetRandomWeaponType(g_Config.m_SvSurvivalMode ? true : false);
	
	if (GetStaticType(m_Subtype) == SW_UPGRADE)
		return;
	
	
	if (str_comp(g_Config.m_SvGametype, "ball") == 0)
	{
		if (frandom() < 0.7f)
			m_Subtype = GetChargedWeapon(m_Subtype, frandom()*WeaponMaxLevel(m_Subtype));
		else
			m_Subtype = GetChargedWeapon(m_Subtype, WeaponMaxLevel(m_Subtype));
	}
	
	if (WeaponMaxLevel(m_Subtype) > 0 && frandom() < 0.5f)
		m_Subtype = GetChargedWeapon(m_Subtype, frandom()*WeaponMaxLevel(m_Subtype));
	else if (WeaponMaxLevel(m_Subtype) > 0 && frandom() < 0.1f)
		m_Subtype = GetChargedWeapon(m_Subtype, WeaponMaxLevel(m_Subtype));
}

void CPickup::ClearWeapon()
{
	if (m_pWeapon)
	{
		m_pWeapon->Clear();
		m_pWeapon = NULL;
	}
}

bool CPickup::IsWeapon()
{
	if (m_SpawnTick > 0)
		return false;
	
	if (m_Type == POWERUP_WEAPON)
		return true;
	
	return false;
}

void CPickup::Tick()
{
	// store
	if (!m_Dropable && m_SnapTick && m_SnapTick < Server()->Tick()-Server()->TickSpeed()*5.0f)
	{
		if (GameServer()->StoreEntity(m_ObjType, m_Type, m_Subtype, m_Pos.x, m_Pos.y))
		{
			ClearWeapon();
			GameServer()->m_World.DestroyEntity(this);
			return;
		}
	}
	
	if (m_Dropable && !m_ResetableDropable && m_Life <= 0)
	{
		ClearWeapon();
		GameServer()->m_World.DestroyEntity(this);
		return;
	}
	
	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "debug", "pickup tick");
	if (!m_Dropable && GameServer()->Collision()->IsForceTile(m_Pos.x, m_Pos.y+m_BoxSize) != 0)
	{
		m_SkipAutoRespawn = false;
		m_ResetableDropable = true;
		m_SpawnPos = m_Pos;
		m_Life = 280;
		m_Flashing = false;
		m_FlashTimer = 0;
		m_Dropable = true;
		m_Treasure = false;
		m_SpawnTick = -1;
		m_Ammo = 1.0f;
	}
	
	if (m_Life > 0 && m_Type == POWERUP_WEAPON)
	{
		if (GetStaticType(m_Subtype) == SW_BOMB)
		{
			m_Life = 9999;
			
			GameServer()->m_pController->m_BombPos = m_Pos;
			GameServer()->m_pController->m_BombStatus = BOMB_IDLE;
		}
		else if (GetStaticType(m_Subtype) == SW_BALL)
			m_Life = 9999;
	}
	
	// wait for respawn
	//if(m_SpawnTick > 0) - 12.5.
	if(m_SpawnTick > 0 && (!m_Dropable || m_ResetableDropable) && !m_Flashing)
	{
		if(Server()->Tick() > m_SpawnTick && !m_SkipAutoRespawn)
		{
			// respawn
			m_SpawnTick = -1;
			
			
			if (m_Type == POWERUP_WEAPON)
				SetRandomWeapon();
			
			ClearWeapon();
			
			if (m_ResetableDropable)
			{
				m_Pos = m_SpawnPos;
				m_Dropable = false;
			}

			if(m_Type == POWERUP_WEAPON)
				GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN);
		}
		else
			return;
	}
	
	// item drops from enemies
	if (m_Dropable && !m_Treasure)
	{
		if (m_Life < 100)
			m_Flashing = true;
		
		if (m_Life > 0)
			m_Life--;
		else
		{
			if (m_ResetableDropable)
			{
				m_Flashing = false;
				m_Dropable = false;
				m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * (4.0f+frandom()*6);
			}
			else
			{
				ClearWeapon();
				GameServer()->m_World.DestroyEntity(this);
				return;
			}
			
			ClearWeapon();
			return;
		}
	}
	
	// a small visual effect before pickup disappears
	if (m_Flashing)
	{
		m_FlashTimer--;
		
		if (m_FlashTimer <= 0)
			m_FlashTimer = 16;
			
		if (m_FlashTimer > 8)
			m_SpawnTick = 999;
		else
			m_SpawnTick = -1;
	}
	
	// physics
	if (m_Dropable && !m_Treasure)
	{
		m_Vel.y += 0.5f;
		
		bool Down = m_Vel.y < 0.0f;
	
		bool Grounded = false;
		if(GameServer()->Collision()->CheckPoint(m_Pos.x+12, m_Pos.y+m_BoxSize/2+5, false, Down))
			Grounded = true;
		if(GameServer()->Collision()->CheckPoint(m_Pos.x-12, m_Pos.y+m_BoxSize/2+5, false, Down))
			Grounded = true;
		
		int OnForceTile = GameServer()->Collision()->IsForceTile(m_Pos.x-12, m_Pos.y+m_BoxSize/2+5);
		if (OnForceTile == 0)
			OnForceTile = GameServer()->Collision()->IsForceTile(m_Pos.x+12, m_Pos.y+m_BoxSize/2+5);
		

		if (Grounded)
		{
			m_Vel.x = (m_Vel.x + OnForceTile*0.37f) * 0.925f;
			//m_Vel.x *= 0.8f;
			//m_Vel.x = (m_Vel.x + OnForceTile) * 0.8f;
			m_AngleForce += (m_Vel.x - OnForceTile*0.74f*6.0f - m_AngleForce) / 2.0f;
		}
		else
		{
			m_Vel.x *= 0.99f;
			m_Vel.y *= 0.99f;
		}
		
		if (m_Vel.x > 0.1f)
			m_Mirror = false;
		else if (m_Vel.x < -0.1f)
			m_Mirror = true;
		
		/*
		if (Grounded)
			m_Vel.x = (m_Vel.x + OnForceTile) * 0.8f;
			//m_Vel.x *= 0.8f;
		else
			m_Vel.x *= 0.99f;
		*/
		
		//if (OnForceTile == -1)
		//	m_Vel.x -= 0.3f;
		
		vec2 OldVel = m_Vel;
		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(m_BoxSize, m_BoxSize), 0.5f);
		
		if (m_pWeapon)
			if ((((OldVel.x < 0 && m_Vel.x > 0) || (OldVel.x > 0 && m_Vel.x < 0)) && abs(m_Vel.x) > 3.0f) ||
				(((OldVel.y < 0 && m_Vel.y > 0) || (OldVel.y > 0 && m_Vel.y < 0)) && abs(m_Vel.y) > 3.0f))
				GameServer()->CreateSound(m_Pos, SOUND_SFX_BOUNCE1);

		m_Angle += clamp(m_AngleForce*0.04f, -0.6f, 0.6f);
	}
	
	// Check if a player intersected us
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	if(pChr && pChr->IsAlive() && pChr->m_SkipPickups <= 0 && (!g_Config.m_SvBotsSkipPickups || !pChr->GetPlayer()->m_IsBot)) // && !pChr->GetPlayer()->m_pAI)
	{
		// player picked us up, is someone was hooking us, let them go
		int RespawnTime = -1;
		
		switch (m_Type)
		{
			case POWERUP_HEALTH:
				if(pChr->IncreaseHealth(10))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
				}
				break;

			case POWERUP_AMMO:
				if(pChr->IncreaseAmmo(1))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
				}
				break;
				
			case POWERUP_ARMOR:
				if(pChr->IncreaseArmor(10))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
				}
				break;
				
			case POWERUP_COIN:
				if(pChr->GetPlayer()->IncreaseGold(1))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
				}
				break;
				
			case POWERUP_KIT:
				if(pChr->AddKit())
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
				}
				break;

			case POWERUP_WEAPON:
				if (!m_pWeapon)
					m_pWeapon = GameServer()->NewWeapon(m_Subtype);
					
				if (pChr->PickWeapon(m_pWeapon))
				{
					if(m_Subtype == WEAPON_GRENADE)
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE);
					else
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
						
					m_pWeapon = NULL;
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
						
					if(pChr->GetPlayer())
						GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), pChr->m_PickedWeaponSlot);
				}
				break;
				
			default:
				break;
		};
		
		if(RespawnTime >= 0)
		{
			// force 10 sec on factory boxes
			if (m_ResetableDropable)
				m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * (4.0f+frandom()*6);
			
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "pickup player='%d:%s' item=%d/%d",
				pChr->GetPlayer()->GetCID(), Server()->ClientName(pChr->GetPlayer()->GetCID()), m_Type, m_Subtype);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
			m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * RespawnTime;
			ClearWeapon();
		}
	}
	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "debug", "pickup tick end");
}

	
void CPickup::AddForce(vec2 Force)
{
	if (m_Dropable && !m_Treasure)
		m_Vel += Force;
	
	// limit speed
	if (length(m_Vel) > 30.0f)
		m_Vel = normalize(m_Vel)*30.0f;
}


void CPickup::SurvivalReset()
{
	if (!m_Dropable || m_ResetableDropable)
	{
		m_SpawnTick = -1;
		m_Flashing = false;
		m_FlashTimer = 0;
		m_Life = 0;
			
		if (m_ResetableDropable)
		{
			m_Pos = m_SpawnPos;
			m_Dropable = false;
		}
	}
		
	ClearWeapon();
	SetRandomWeapon();

	if (m_Dropable)
	{
		ClearWeapon();
		GameServer()->m_World.DestroyEntity(this);
	}
}


void CPickup::TickPaused()
{
	if(m_SpawnTick != -1)
		++m_SpawnTick;
}

void CPickup::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient))
		return;

	m_SnapTick = Server()->Tick();
	
	if (m_SpawnTick != -1)
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if (!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Angle = (int)(m_Angle*256.0f);
	pP->m_Mirror = m_Mirror;
	pP->m_Type = m_Type;
	pP->m_Subtype = m_Subtype;
}
