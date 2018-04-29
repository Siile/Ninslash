#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "building.h"
#include "lightning.h"
#include "teslacoil.h"

CTeslacoil::CTeslacoil(CGameWorld *pGameWorld, vec2 Pos, int Team, int OwnerPlayer)
: CBuilding(pGameWorld, Pos, BUILDING_TESLACOIL, Team)
{
	m_ProximityRadius = TeslacoilPhysSize;
	m_Life = 80;
	m_MaxLife = 80;
	
	m_AttachOnFall = true;
	m_Bounciness = 0.0f;
	m_BoxSize = vec2(24.0f, 34.0f);
	m_CanMove = true;
	m_Moving = false;
	
	m_OwnerPlayer = OwnerPlayer;
	m_AttackTick = Server()->Tick() + Server()->TickSpeed()*frandom();
	
	if (!GameServer()->m_pController->IsTeamplay())
		m_Team = m_OwnerPlayer;
	else
		m_Team = Team;
	
	m_Center = vec2(0, -55);
	m_FlipY = 1;
	
	if (GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 40))
	{
		m_Mirror = true;
		m_Center = vec2(0, +55);
		m_FlipY = -1;
	}
}





void CTeslacoil::Tick()
{
	if (m_Life < 40)
		m_aStatus[BSTATUS_REPAIR] = 1;
	else
		m_aStatus[BSTATUS_REPAIR] = 0;
	
	UpdateStatus();

	if (Server()->Tick() > m_AttackTick + Server()->TickSpeed()*(0.3f + frandom()*0.3f))
		Fire();
	
	Move();
	
	// destroy
	if (m_Life <= 0)
	{
		//GameServer()->CreateExplosion(m_Pos + vec2(0, -50*m_FlipY), m_DamageOwner, GetBuildingWeapon(BUILDING_TESLACOIL), 0, false, false);
		GameServer()->CreateExplosion(m_Pos, m_DamageOwner, GetBuildingWeapon(m_Type));
		//GameServer()->CreateSound(m_Pos + vec2(0, -50*m_FlipY), SOUND_GRENADE_EXPLODE);
		GameServer()->m_World.DestroyEntity(this);
	}
}


void CTeslacoil::Fire()
{
	m_AttackTick = Server()->Tick();

	vec2 TurretPos = m_Pos+vec2(0, -67*m_FlipY);
	
	bool Sound = false;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->GetTeam() == m_Team && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if ((!pCharacter->IsAlive() || pCharacter->GetPlayer()->GetCID() == m_OwnerPlayer) && !GameServer()->m_pController->IsTeamplay())
			continue;
		
		if (GameServer()->m_pController->IsCoop())
		{
			if (!pCharacter->m_IsBot && m_Team >= 0)
				continue;
			
			if (pCharacter->m_IsBot && m_Team < 0)
				continue;
		}
		
		if (pCharacter->Invisible())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 700 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
		{
			new CLightning(GameWorld(), TurretPos, pCharacter->m_Pos);
			pCharacter->TakeDamage(m_OwnerPlayer, GetBuildingWeapon(BUILDING_TESLACOIL), 5, vec2(0, 0), vec2(0, 0));
			//pCharacter->TakeDamage(vec2(0, 0), 5, m_DamageOwner, DEATHTYPE_TESLACOIL, vec2(0, 0), DAMAGETYPE_ELECTRIC, false);
			Sound = true;
		}
	}
	
	if (Sound)
		GameServer()->CreateSound(m_Pos + vec2(0, -50*m_FlipY), SOUND_TESLACOIL_FIRE);
}


void CTeslacoil::TickPaused()
{
}

void CTeslacoil::Snap(int SnappingClient)
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

	if (GameServer()->m_pController->IsTeamplay())
		pP->m_Team = m_Team;
	else
	{
		if (SnappingClient == m_OwnerPlayer)
			pP->m_Team = TEAM_RED;
		else
			pP->m_Team = -1;
	}
}
