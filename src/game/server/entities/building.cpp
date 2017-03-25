#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

#include <game/server/player.h>
#include "character.h"
#include "monster.h"

#include "building.h"
#include "projectile.h"
#include "superexplosion.h"

CBuilding::CBuilding(CGameWorld *pGameWorld, vec2 Pos, int Type, int Team)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_BUILDING)
{
	m_SetTimer = 0;
	m_Center = vec2(0, 0);
	m_Collision = true;
	
	m_Status = 0;
	for (int i = 0; i < NUM_BSTATUS; i++)
		m_aStatus[i] = 0;
	
	switch (Type)
	{
	case BUILDING_SAWBLADE:
		m_ProximityRadius = SawbladePhysSize;
		m_Life = 100;
		break;
		
	case BUILDING_MINE1:
	case BUILDING_MINE2:
		m_ProximityRadius = MinePhysSize;
		m_Life = 10+frandom()*10;
		m_SetTimer = GameServer()->Server()->TickSpeed()*1.5f;
		break;
		
	case BUILDING_BARREL:
		m_ProximityRadius = BarrelPhysSize;
		m_Life = 15+frandom()*10;
		break;
		
	case BUILDING_LAZER:
		m_ProximityRadius = LazerPhysSize;
		m_Life = 100;
		break;
	
	case BUILDING_BASE:
		m_ProximityRadius = BasePhysSize;
		m_Life = 60;
		break;
		
	case BUILDING_STAND:
		m_ProximityRadius = StandPhysSize;
		m_Life = 60;
		m_Center = vec2(0, -10);
		break;
		
	case BUILDING_SWITCH:
		m_ProximityRadius = SwitchPhysSize;
		m_Life = 9000;
		m_Center = vec2(0, -34);
		break;
		
	case BUILDING_DOOR1:
		m_ProximityRadius = DoorPhysSize;
		m_Life = 9000;
		m_Center = vec2(0, -20);
		m_Collision = false;
		break;
		
	case BUILDING_JUMPPAD:
		m_ProximityRadius = JumppadPhysSize;
		m_Life = 9000;
		Pos += vec2(48, 16);
		m_Center = vec2(0, 0);
		m_Collision = false;
		break;
		
	case BUILDING_FLAMETRAP:
		m_ProximityRadius = FlametrapPhysSize;
		m_Life = 60;
		break;
	
	default:
		m_ProximityRadius = BuildingPhysSize;
		m_Life = 100;
	};
	
	m_Pos = Pos;
	m_Team = Team;
	m_Type = Type;
	m_MaxLife = m_Life;
	
	if (!GameServer()->m_pController->IsTeamplay())
		m_Team = TEAM_NEUTRAL;
	
	if (m_Team == TEAM_BLUE)
		m_DamageOwner = BLUE_BASE;
	else if (m_Team == TEAM_RED)
		m_DamageOwner = RED_BASE;
	else
		m_DamageOwner = NEUTRAL_BASE;
	
	m_TriggerTimer = 0;
	
	GameWorld()->InsertEntity(this);
}

void CBuilding::Reset()
{
	//GameServer()->m_World.DestroyEntity(this);
}


bool CBuilding::Jumppad()
{
	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameServer()->m_World.FindEntities(m_Pos, m_ProximityRadius*10.0f, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	bool ret = false;
														
	for (int i = 0; i < Num; ++i)
	{
		CCharacter *pTarget = apEnts[i];

		if (abs(pTarget->m_Pos.x - m_Pos.x) < 64 && abs(pTarget->m_Pos.y - m_Pos.y) < 16)
		{
			pTarget->Jumppad();
			m_aStatus[BSTATUS_ON] = 1;
			m_TriggerTimer = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 0.05f;
			//GameServer()->CreateSound(m_Pos, SOUND_DOOR1);
			ret = true;
		}
	}
	
	return ret;
}


bool CBuilding::Repair(int Amount)
{
	if (m_Life >= m_MaxLife)
		return false;
	
	m_Life += Amount;
	if (m_Life > m_MaxLife)
		m_Life = m_MaxLife;
	
	return true;
}

void CBuilding::UpdateStatus()
{
	if (m_Mirror)
		m_aStatus[BSTATUS_MIRROR] = 1;
	
	m_Status = 0;
	
	for (int i = 0; i < NUM_BSTATUS; i++)
	{
		if (m_aStatus[i] > 0)
			m_Status |= 1 << i;
	}
}



void CBuilding::Trigger()
{
	if (m_Type == BUILDING_DOOR1 && !m_aStatus[BSTATUS_EVENT])
	{
		m_aStatus[BSTATUS_EVENT] = 1;
		m_TriggerTimer = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 0.5f;
		GameServer()->CreateSound(m_Pos, SOUND_DOOR1);
	}
}

void CBuilding::TakeDamage(int Damage, int Owner, int Weapon)
{
	if (m_Type == BUILDING_SWITCH)
	{
		m_aStatus[BSTATUS_ON] = 1;
		GameServer()->m_pController->TriggerSwitch(m_Pos);
		return;
	}
	
	if (m_Life >= 5000)
		return;
	
	if (m_Type == BUILDING_TURRET && GameServer()->m_pController->IsCoop())
	{
		if (Owner >= 0 && Owner < MAX_CLIENTS)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[Owner];
				
			if(pPlayer && !pPlayer->m_IsBot)
				return;
		}
	}
	
	if (g_Config.m_SvOneHitKill)
		Damage = 1000;
	
	if (m_DeathTimer > 0 || m_Life <= 0)
		return;
	
	if (m_Type == BUILDING_SAWBLADE || m_Type == BUILDING_LAZER || m_Type == BUILDING_POWERUPPER)
		return;
	
	// todo
	m_Life -= Damage / 2;
	if (m_Life <= 0)
	{
		m_DeathTimer = 5;
		/* turret
		vec2 ep = m_Pos + vec2(0, -70);
		GameServer()->CreateExplosion(ep, m_DamageOwner, Weapon, false, false);
		GameServer()->CreateSound(ep, SOUND_GRENADE_EXPLODE);
		*/
		//Destroy();
		//GameServer()->m_World.DestroyEntity(this);
		
		if (m_Type == BUILDING_BARREL)
			m_DamageOwner = Owner;
	}
}


void CBuilding::Destroy()
{
	if (m_Type == BUILDING_MINE1)
	{
		GameServer()->CreateMineExplosion(m_Pos, m_DamageOwner, DEATHTYPE_LANDMINE, false);
		GameServer()->m_World.DestroyEntity(this);
	}
	else if (m_Type == BUILDING_MINE2)
	{
		GameServer()->CreateElectromineExplosion(m_Pos, m_DamageOwner, DEATHTYPE_ELECTROMINE, false);
		GameServer()->m_World.DestroyEntity(this);
	}
	else if (m_Type == BUILDING_BARREL)
	{
		//CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, m_Pos, m_DamageOwner, WEAPON_HAMMER, 1);
		//GameServer()->m_World.InsertEntity(S);
		//GameServer()->CreateExplosion(ep, m_DamageOwner, Weapon, false, false);
		//GameServer()->CreateFlameExplosion(m_Pos, m_DamageOwner, WEAPON_HAMMER, false);
		
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		GameServer()->CreateExplosion(m_Pos, m_DamageOwner, DEATHTYPE_BARREL, false, false);
		GameServer()->m_World.DestroyEntity(this);
	}
	else if (m_Type == BUILDING_FLAMETRAP)
	{
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		GameServer()->CreateExplosion(m_Pos, m_DamageOwner, DEATHTYPE_FLAMETRAP, false, false);
		GameServer()->m_World.DestroyEntity(this);
	}
	else
		GameServer()->m_World.DestroyEntity(this);
}


void CBuilding::Tick()
{
	if (m_Type == BUILDING_DOOR1)
	{
		if (m_TriggerTimer > 0 && m_TriggerTimer < GameServer()->Server()->Tick())
		{
			m_aStatus[BSTATUS_ON] = 1;
		}
		
		if (m_TriggerTimer > 0 && m_TriggerTimer < GameServer()->Server()->Tick() - Server()->TickSpeed()*0.6f)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, m_ProximityRadius/2.0f, 0);
					
			if(pChr && pChr->IsAlive() && !pChr->m_IsBot)
			{
				GameServer()->m_pController->NextLevel(pChr->GetPlayer()->GetCID());
			}
		}
	}
	
	if (m_Type == BUILDING_JUMPPAD)
	{
		//if (m_TriggerTimer < GameServer()->Server()->Tick())
		{
			Jumppad();
			//if (!Jumppad())
			//	m_aStatus[BSTATUS_ON] = 0;
		}
		if (m_TriggerTimer < GameServer()->Server()->Tick())
			m_aStatus[BSTATUS_ON] = 0;
	}
	
	if (m_Type == BUILDING_SAWBLADE)
	{
		CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, m_ProximityRadius*1.4f, 0);
		if(pChr && pChr->IsAlive())
			pChr->TakeSawbladeDamage(m_Pos);
		
		// walker / drone collision
		CMonster *apEnts[MAX_CLIENTS];
		int Num = GameServer()->m_World.FindEntities(m_Pos, m_ProximityRadius*1.5f, (CEntity**)apEnts,
													MAX_CLIENTS, CGameWorld::ENTTYPE_MONSTER);

		for (int i = 0; i < Num; ++i)
		{
			CMonster *pTarget = apEnts[i];

			if (pTarget->m_Health <= 0)
				continue;

			vec2 Dir;
			if (length(pTarget->m_Pos - m_Pos) > 0.0f)
				Dir = normalize(pTarget->m_Pos - m_Pos);
			else
				Dir = vec2(0.f, 0.f);

			pTarget->TakeDamage(Dir * 10.0f, 5, -1, vec2(0, 0));
		}
	}
	
	 // effect testing on all buildings
	//if (GameServer()->Server()->Tick()%40 == 1)
	//	GameServer()->CreateEffect(FX_BARREL, m_Pos);


	if (m_SetTimer > 0)
		m_SetTimer--;
	
	if (m_Type == BUILDING_FLAMETRAP)
	{
		// on / off
		if (m_TriggerTimer < GameServer()->Server()->Tick() - GameServer()->Server()->TickSpeed() * 3.0f)
			m_TriggerTimer = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 2.0f;
		
		// if on
		if (m_TriggerTimer > GameServer()->Server()->Tick())
		{
			m_aStatus[BSTATUS_FIRE] = 1;
			
			// small delay before settings players to aflame
			if (m_TriggerTimer > GameServer()->Server()->Tick() && m_TriggerTimer < GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 1.7f)
			{
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos + vec2((m_Mirror ? -50 : 50), -4), m_ProximityRadius, 0);
				
					if(pChr && pChr->IsAlive())
					{
						pChr->TakeDamage(vec2(0, 0), 2, NEUTRAL_BASE, DEATHTYPE_FLAMETRAP, vec2(0, 0), DAMAGETYPE_FLAME);
						pChr->SetAflame(1.0f, NEUTRAL_BASE, DEATHTYPE_FLAMETRAP);
					}
				}
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos + vec2((m_Mirror ? -102 : 102), -4), m_ProximityRadius*1.3f, 0);
				
					if(pChr && pChr->IsAlive())
					{
						pChr->TakeDamage(vec2(0, 0), 2, NEUTRAL_BASE, DEATHTYPE_FLAMETRAP, vec2(0, 0), DAMAGETYPE_FLAME);
						pChr->SetAflame(1.0f, NEUTRAL_BASE, DEATHTYPE_FLAMETRAP);
					}
				}
			}
		}
		else
			m_aStatus[BSTATUS_FIRE] = 0;
	}

	
	// destruction
	if (m_DeathTimer > 0)
	{
		m_DeathTimer--;

		if (m_Life <= 0 && m_DeathTimer <= 0)
		{
			// turret?
			Destroy();
			//GameServer()->m_World.DestroyEntity(this);
		}
	}
	else
	if (m_SetTimer <= 0 && (m_Type == BUILDING_MINE1 || m_Type == BUILDING_MINE2))
	{
		CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, m_ProximityRadius*1.7f, 0);
		if(pChr && pChr->IsAlive() && !pChr->IsSliding())// && (pChr->GetPlayer()->GetTeam() != m_Team || !GameServer()->m_pController->IsTeamplay()))
		{
			// co-op, ignore real players if mine set by a real player
			if (GameServer()->m_pController->IsCoop() && !pChr->m_IsBot && m_DamageOwner >= 0)
			{
				// do nothing	
			}
			else
			{
				if (pChr->GetPlayer()->GetTeam() != m_Team || !GameServer()->m_pController->IsTeamplay())
				{
					m_DeathTimer = 1;
					m_Life = 0;
				}
			}
		}
	}
	
	UpdateStatus();
}




void CBuilding::TickPaused()
{
}

void CBuilding::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Building *pP = static_cast<CNetObj_Building *>(Server()->SnapNewItem(NETOBJTYPE_BUILDING, m_ID, sizeof(CNetObj_Building)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Status = m_Status;
	pP->m_Type = m_Type;
	pP->m_Team = m_Team;
}
