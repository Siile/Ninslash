#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

#include <game/server/player.h>
#include "character.h"

#include "building.h"
#include "projectile.h"
#include "superexplosion.h"

CBuilding::CBuilding(CGameWorld *pGameWorld, vec2 Pos, int Type, int Team)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_BUILDING)
{
	m_SetTimer = 0;
	
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
		m_ProximityRadius = BarrelPhysSize;
		m_Life = 100;
		break;
	
	default:
		m_ProximityRadius = BuildingPhysSize;
		m_Life = 100;
	};
	
	m_Pos = Pos;
	m_Team = Team;
	m_Type = Type;
	m_Center = vec2(0, 0);
	
	if (!GameServer()->m_pController->IsTeamplay())
		m_Team = TEAM_NEUTRAL;
	
	if (m_Team == TEAM_BLUE)
		m_DamageOwner = BLUE_BASE;
	else if (m_Team == TEAM_RED)
		m_DamageOwner = RED_BASE;
	else
		m_DamageOwner = NEUTRAL_BASE;
	
	GameWorld()->InsertEntity(this);
}

void CBuilding::Reset()
{
	//GameServer()->m_World.DestroyEntity(this);
}


void CBuilding::TakeDamage(int Damage, int Owner, int Weapon)
{
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
	}
}


void CBuilding::Destroy()
{
	if (m_Type == BUILDING_MINE1)
	{
		GameServer()->CreateMineExplosion(m_Pos, m_DamageOwner, DEATHTYPE_LANDMINE, false);
		GameServer()->m_World.DestroyEntity(this);
	}
	
	if (m_Type == BUILDING_MINE2)
	{
		GameServer()->CreateElectromineExplosion(m_Pos, m_DamageOwner, DEATHTYPE_ELECTROMINE, false);
		GameServer()->m_World.DestroyEntity(this);
	}
			
	if (m_Type == BUILDING_BARREL)
	{
		//CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, m_Pos, m_DamageOwner, WEAPON_HAMMER, 1);
		//GameServer()->m_World.InsertEntity(S);
		//GameServer()->CreateExplosion(ep, m_DamageOwner, Weapon, false, false);
		//GameServer()->CreateFlameExplosion(m_Pos, m_DamageOwner, WEAPON_HAMMER, false);
		
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		GameServer()->CreateExplosion(m_Pos, m_DamageOwner, DEATHTYPE_BARREL, false, false);
		GameServer()->m_World.DestroyEntity(this);
	}
	
}


void CBuilding::Tick()
{
	if (m_Type == BUILDING_SAWBLADE)
	{
		CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, m_ProximityRadius*1.4f, 0);
		if(pChr && pChr->IsAlive())
			pChr->TakeSawbladeDamage(m_Pos);
	}
	
	 // effect testing on all buildings
	//if (GameServer()->Server()->Tick()%40 == 1)
	//	GameServer()->CreateEffect(FX_BARREL, m_Pos);


	if (m_SetTimer > 0)
		m_SetTimer--;

	
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
		if(pChr && pChr->IsAlive())// && (pChr->GetPlayer()->GetTeam() != m_Team || !GameServer()->m_pController->IsTeamplay()))
		{
			if (pChr->GetPlayer()->GetTeam() != m_Team || !GameServer()->m_pController->IsTeamplay())
			{
				m_DeathTimer = 1;
				m_Life = 0;
			}
		}
	}
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
	pP->m_Type = m_Type;
	pP->m_Team = m_Team;
}
