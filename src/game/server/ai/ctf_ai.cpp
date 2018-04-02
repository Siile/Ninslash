#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "ctf_ai.h"


CAIctf::CAIctf(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	pPlayer->SetRandomSkin();
}


void CAIctf::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
}


void CAIctf::DoBehavior()
{
	// power level
	m_PowerLevel = 8;
	
	if (g_Config.m_SvGodBots)
		m_PowerLevel = 20;
	
	// reset jump and attack
	if (Player()->GetCharacter()->GetCore().m_JetpackPower < 10 || Player()->GetCharacter()->GetCore().m_Jetpack == 0)
		m_Jump = 0;
	
	m_Attack = 0;

	
	HeadToMovingDirection();

	SeekClosestEnemyInSight();
	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		if (!ShootAtClosestEnemy())
			if (!ShootAtClosestBuilding())
				ShootAtClosestMonster();
		ReactToPlayer();
	}
	else
	{
		if (!ShootAtClosestBuilding())
			ShootAtClosestMonster();
	}

	
	bool SeekEnemy = false;
	
	int EnemyTeam = TEAM_RED;
	if (Player()->GetTeam() == TEAM_RED)
		EnemyTeam = TEAM_BLUE;
	
	
	// carrying flag
	if (GameServer()->m_pController->GetFlagState(EnemyTeam) == Player()->GetCID())
		m_TargetPos = GameServer()->m_pController->GetFlagPos(Player()->GetTeam());
	else
	{
		// easy access
		vec2 TeamFlagPos = GameServer()->m_pController->GetFlagPos(Player()->GetTeam());
		vec2 EnemyFlagPos = GameServer()->m_pController->GetFlagPos(EnemyTeam);
			
		if (GameServer()->m_pController->GetFlagState(Player()->GetTeam()) != FLAG_ATSTAND)
		{
			// check distance to flags, choose closer
			if (distance(TeamFlagPos, m_Pos) < distance(EnemyFlagPos, m_Pos)*1.2f)
				m_TargetPos = TeamFlagPos;
			else
				m_TargetPos = EnemyFlagPos;
			
			if (GameServer()->m_pController->GetFlagState(EnemyTeam) >= 0)
			{
				m_TargetPos = TeamFlagPos;
			}
		}
		else
		{
			m_TargetPos = GameServer()->m_pController->GetFlagPos(EnemyTeam);
			
			if (GameServer()->m_pController->GetFlagState(EnemyTeam) != FLAG_ATSTAND && distance(TeamFlagPos, m_Pos) < 500)
				SeekEnemy = true;
		}
	}
	

	if (SeekEnemy)
	{
		if (SeekClosestEnemy())
		{
			m_TargetPos = m_PlayerPos;
							
			if (m_EnemiesInSight > 0)
			{
				if (WeaponShootRange() - m_PlayerDistance > 200)
					SeekRandomWaypoint();
			}
		}
	}
	

	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint();
	}
	else
	{
		m_WaypointPos = m_TargetPos;
		MoveTowardsWaypoint(true);
	}
	
	Build();
	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}
