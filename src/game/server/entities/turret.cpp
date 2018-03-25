#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include <game/weapons.h>
#include "building.h"
#include "turret.h"
#include "projectile.h"
#include "laser.h"
#include "weapon.h"

CTurret::CTurret(CGameWorld *pGameWorld, vec2 Pos, int Team, class CWeapon *pWeapon)
: CBuilding(pGameWorld, Pos, BUILDING_TURRET, Team)
{
	m_ProximityRadius = TurretPhysSize;
	m_Life = 60;
	m_MaxLife = 60;
	m_Angle = 0;
	m_TargetTimer = 0;
	m_TargetIndex = -1;
	m_ReloadTimer = 0;
	m_AttackTick = 0;
	
	m_pWeapon = pWeapon;
	m_pWeapon->SetTurret();
	
	m_OwnerPlayer = m_pWeapon->GetOwner();
	
	if (!GameServer()->m_pController->IsTeamplay())
		m_Team = m_OwnerPlayer;
	else
		m_Team = Team;

	m_Center = vec2(0, -40);
	m_FlipY = 1;
	
	// upside down
	if (GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 45))
	{
		m_Mirror = true;
		m_Center = vec2(0, +40);
		m_FlipY = -1;
	}
	
	SetAngle(vec2(frandom()-frandom(), frandom()*m_FlipY));
}



void CTurret::SetAngle(vec2 Direction)
{
	m_Angle = GetAngle(Direction) * (180/pi) + 90;
	
	m_OriginalDirection = Direction;
	m_Target = Direction;
}



void CTurret::Tick()
{
	if (m_Life < 30)
		m_aStatus[BSTATUS_REPAIR] = 1;
	else
		m_aStatus[BSTATUS_REPAIR] = 0;
	
	
	if (m_DeathTimer > 0)
	{
		m_DeathTimer--;
		if (m_Life <= 0 && m_DeathTimer <= 0)
		{
			GameServer()->CreateExplosion(m_Pos+vec2(0, -50*m_FlipY), m_DamageOwner, GetBuildingWeapon(m_Type));
			GameServer()->CreateSound(m_Pos+vec2(0, -50*m_FlipY), SOUND_GRENADE_EXPLODE);
			GameServer()->m_World.DestroyEntity(this);
		}
		return;
	}

	bool WillFire = false;
	
	if (Target())
	{
		WillFire = true;
		if (m_TargetTimer-- < 0)
		{
			FindTarget();
			m_TargetTimer = 30 + frandom()*30;
		}
	}
	else
	// idle
	{
		if (!FindTarget())
		{
			if (m_TargetTimer-- < 0)
			{
				m_TargetTimer = 50 + frandom()*100;
				//m_Target = vec2((frandom()-frandom())*150, frandom()*50-frandom()*100);
				m_Target = m_OriginalDirection;
			}
		}
	}
	
	int TargetAngle = int(atan2(m_Target.y, m_Target.x) * 180/pi + 90);
	if (TargetAngle < 0)
		TargetAngle += 360;
	
	if (m_Angle < 90 && TargetAngle > 270)
	{
		m_Angle -= 2;
	}
	else if (m_Angle > 270 && TargetAngle < 90)
	{
		m_Angle += 2;
	}
	else
	{
		if (m_Angle < TargetAngle+1)
			m_Angle += 2;
		else if (m_Angle > TargetAngle-1)
			m_Angle -= 2;
	}
	
	if (m_Angle < 0)
		m_Angle += 360;
	if (m_Angle > 360)
		m_Angle -= 360;
	
	if (WillFire && abs(m_Angle - TargetAngle) < 30)
		Fire();
	
	
	UpdateStatus();
}


bool CTurret::Fire()
{
	vec2 TurretPos = m_Pos+vec2(0, 10 -45.0f*m_FlipY);
	float Angle = (m_Angle + 90) / (180/pi);
		
	vec2 Dir = vec2(cosf(Angle), sinf(Angle));
	
	m_pWeapon->SetPos(TurretPos, vec2(0, 0), Dir, 28);
	
	if (m_pWeapon->Fire(NULL))
	{
		m_AttackTick = Server()->Tick();
		return true;
	}
	
	return false;
}


bool CTurret::Target()
{
	vec2 TurretPos = m_Pos+vec2(0, -50*m_FlipY);
	
	if (m_TargetIndex >= 0 && m_TargetIndex < MAX_CLIENTS)
	{
		CCharacter *pCharacter = GameServer()->GetPlayerChar(m_TargetIndex);
		if (!pCharacter)
			return false;
		
		if (!pCharacter->IsAlive())
			return false;

		int Distance = distance(pCharacter->m_Pos, TurretPos);
		//if (Distance < aCustomWeapon[iw].m_AiAttackRange && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
		if (Distance < 900 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
		{
			m_Target = TurretPos - ((pCharacter->m_Pos+vec2(0, -64)) + pCharacter->GetCore().m_Vel * 2.0f);
			return true;
		}
		else
			return false;
	}
	
	return false;
}


bool CTurret::FindTarget()
{
	m_TargetIndex = -1;
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	vec2 TurretPos = m_Pos+vec2(0, -50*m_FlipY);
	
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
		if (Distance < AIAttackRange(m_pWeapon->GetWeaponType()) && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_TargetIndex = i;
			}
		}
	}
	
	if (pClosestCharacter)
		return true;
	
	return false;
}



void CTurret::TickPaused()
{
}

void CTurret::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Turret *pP = static_cast<CNetObj_Turret *>(Server()->SnapNewItem(NETOBJTYPE_TURRET, m_ID, sizeof(CNetObj_Turret)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Angle = m_Angle;
	
	/*
	if (!GameServer()->m_pController->IsTeamplay() && SnappingClient == m_OwnerPlayer)
		pP->m_Team = TEAM_BLUE;
	else
		pP->m_Team = m_Team;
	*/
	

	if (GameServer()->m_pController->IsTeamplay())
		pP->m_Team = m_Team;
	else
	{
		if (SnappingClient == m_OwnerPlayer)
			pP->m_Team = TEAM_RED;
		else
			pP->m_Team = -1;
	}
	
	pP->m_Status = m_Status;
	pP->m_Weapon = m_pWeapon->GetWeaponType();
	pP->m_AttackTick = m_AttackTick;
}
