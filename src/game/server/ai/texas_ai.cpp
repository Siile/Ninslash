#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "texas_ai.h"


CAItexas::CAItexas(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
}


void CAItexas::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
	
	//if (g_Config.m_SvRandomWeapons)
	//	pChr->GiveRandomWeapon();

}


void CAItexas::DoBehavior()

{
	// power level
	if (Player()->GetTeam() == TEAM_RED)
		m_PowerLevel = GameServer()->m_pController->CountPlayers(TEAM_BLUE)*4;

	if (Player()->GetTeam() == TEAM_BLUE)
		m_PowerLevel = 14 - GameServer()->m_pController->CountPlayers(TEAM_BLUE)*2;
		
		
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
			ShootAtClosestMonster();
		ReactToPlayer();
	}
	else
	{
		if (!ShootAtClosestMonster())
			;
			//m_AttackTimer = 0;
	}

	// main logic
	if (Player()->GetTeam() == TEAM_RED)
	{
		if (SeekClosestFriend(true))
		{
			m_TargetPos = m_PlayerPos;
			
			if (m_PlayerDistance < 1400-m_EnemiesInSight*400-GameServer()->m_pController->CountPlayers(TEAM_BLUE)*60)
				SeekRandomWaypoint();
		}
		else if (SeekClosestFriend())
		{
			m_TargetPos = m_PlayerPos;
			
			if (m_PlayerDistance < 1400-m_EnemiesInSight*400-GameServer()->m_pController->CountPlayers(TEAM_BLUE)*60)
				SeekRandomWaypoint();
		}
		else
			SeekRandomWaypoint();
	}
	else if (Player()->GetTeam() == TEAM_BLUE)
	{
		if (SeekClosestEnemy())
			m_TargetPos = m_PlayerPos;
		else
			SeekRandomWaypoint();
	}
	
	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint(20);
	}
	else
	{
		m_Hook = 0;
	}
	
	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 1 + frandom()*2;
	
}
