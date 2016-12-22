#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "pickup.h"
#include "electro.h"
#include "superexplosion.h"


CPickup::CPickup(CGameWorld *pGameWorld, int Type, int SubType, int Ammo)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Type = Type;
	m_Subtype = SubType;
	m_ProximityRadius = PickupPhysSize;

	m_ResetableDropable = false;

	Reset();

	GameWorld()->InsertEntity(this);
	m_SkipAutoRespawn = false;

	m_Dropable = false;
	m_Life = 0;
	m_Vel = vec2(0, 0);
	m_Ammo = Ammo;
}

void CPickup::Reset()
{
	if (!m_SkipAutoRespawn)
	{
		if (g_pData->m_aPickups[m_Type].m_Spawndelay > 0)
			m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * g_pData->m_aPickups[m_Type].m_Spawndelay;
		else
			m_SpawnTick = -1;

		m_Flashing = false;
		m_FlashTimer = 0;
	}
}

void CPickup::Tick()
{
	if (!m_Dropable && GameServer()->Collision()->IsForceTile(m_Pos.x, m_Pos.y+24) != 0)
	{
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

	// wait for respawn
	//if(m_SpawnTick > 0) - 12.5.
	if(m_SpawnTick > 0 && (!m_Dropable || m_ResetableDropable) && !m_Flashing)
	{
		if(Server()->Tick() > m_SpawnTick && !m_SkipAutoRespawn)
		{
			// respawn
			m_SpawnTick = -1;

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
				m_SpawnTick = 999;
			return;
		}
	}

	// a small visual effect before pickup disappears
	if (m_Flashing)
	{
		m_FlashTimer--;

		if (m_FlashTimer <= 0)
			m_FlashTimer = 20;

		if (m_FlashTimer > 10)
			m_SpawnTick = 999;
		else
			m_SpawnTick = -1;
	}



	// physics
	if (m_Dropable && !m_Treasure)
	{
		m_Vel.y += 0.5f;

		bool Grounded = false;
		if(GameServer()->Collision()->CheckPoint(m_Pos.x+12, m_Pos.y+12+5))
			Grounded = true;
		if(GameServer()->Collision()->CheckPoint(m_Pos.x-12, m_Pos.y+12+5))
			Grounded = true;

		int OnForceTile = GameServer()->Collision()->IsForceTile(m_Pos.x-12, m_Pos.y+12+5);
		if (OnForceTile == 0)
			OnForceTile = GameServer()->Collision()->IsForceTile(m_Pos.x+12, m_Pos.y+12+5);

		if (Grounded)
			m_Vel.x = (m_Vel.x + OnForceTile) * 0.8f;
			//m_Vel.x *= 0.8f;
		else
			m_Vel.x *= 0.99f;


		//if (OnForceTile == -1)
		//	m_Vel.x -= 0.3f;

		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(24.0f, 24.0f), 0.4f);
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

			case POWERUP_ARMOR:
				if(pChr->IncreaseArmor(1))
				{
					GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
					RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					m_Life = 0;
					m_Flashing = false;
				}
				break;

			case POWERUP_MINE:
				if(pChr->AddMine())
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

			// todo: clean and remove parent weapon type
			case POWERUP_WEAPON:
				if(m_Subtype >= 0 && m_Subtype < NUM_CUSTOMWEAPONS)
				{
					/*
					if (Parent < 0 || Parent >= NUM_WEAPONS)
					{
						RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
						m_Life = 0;
						m_Flashing = false;
						break;
					}
					*/

					float AmmoFill = 1.0f;
					if (m_Dropable)
						AmmoFill = 0.3f + frandom()*0.3f;

					if (m_Ammo >= 0.0f)
						AmmoFill = m_Ammo;

					if (pChr->GiveCustomWeapon(m_Subtype, AmmoFill))
					{
						if(m_Subtype == WEAPON_GRENADE)
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE);
						else
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);

						if(pChr->GetPlayer())
						{
							//GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), Parent);

							GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);

							//char aBuf[256]; str_format(aBuf, sizeof(aBuf), "Picked up %s", aCustomWeapon[m_Subtype].m_Name);
							//GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(), aBuf);
						}

						RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
						m_Life = 0;
						m_Flashing = false;
					}
					else
					{
						if (pChr->GiveAmmo(&m_Subtype, AmmoFill))
						{
							if(m_Subtype == WEAPON_GRENADE)
								GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE);
							else
								GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);

							//char aBuf[256]; str_format(aBuf, sizeof(aBuf), "Picked up ammo for %s", aCustomWeapon[m_Subtype].m_Name);
							//GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(), aBuf);

							if(pChr->GetPlayer())
								GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);

							RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
							m_Life = 0;
							m_Flashing = false;
						}
					}

					/*if(pChr->GiveWeapon(m_Subtype, 10)) // !pChr->m_WeaponPicked &&
					{
						RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

						//pChr->m_WeaponPicked = true;

						if(m_Subtype == WEAPON_GRENADE)
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE);
						else if(m_Subtype == WEAPON_SHOTGUN)
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);
						else if(m_Subtype == WEAPON_RIFLE)
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN);

						if(pChr->GetPlayer())
							GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
					}*/
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
		}
	}
}

void CPickup::TickPaused()
{
	if(m_SpawnTick != -1)
		++m_SpawnTick;
}

void CPickup::Snap(int SnappingClient)
{
	if(m_SpawnTick != -1 || NetworkClipped(SnappingClient) ||
			(m_Type == POWERUP_WEAPON && !GameServer()->m_pController->CanSeePickup(SnappingClient, m_Type, m_Subtype))) // for gungame
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;

	if (m_Type == POWERUP_WEAPON && m_Subtype >= 0 && m_Subtype < NUM_CUSTOMWEAPONS)
	{
		//pP->m_Subtype = aCustomWeapon[m_Subtype].m_ParentWeapon;
		pP->m_Subtype = m_Subtype;
	}
	else
		pP->m_Subtype = m_Subtype;
}
