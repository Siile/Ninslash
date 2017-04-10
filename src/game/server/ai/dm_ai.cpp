#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "dm_ai.h"


CAIdm::CAIdm(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
}


void CAIdm::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_PowerLevel = 4;
	
	if (g_Config.m_SvGodBots)
		m_PowerLevel = 20;
	
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
	
	//pChr->GiveRandomWeapon();
}


void CAIdm::DoBehavior()
{
	// power level
	//m_PowerLevel = 20 - GameServer()->m_pController->CountPlayers()*1.5f;
	
	// reset jump and attack
	/*
	if (frandom()*10 < 2)
		m_Jump = 0;
	*/

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


	//if (SeekClosestEnemy())
	if (SeekRandomEnemy())
	{
		m_TargetPos = m_PlayerPos;
				
		if (m_EnemiesInSight > 0)
		{
			if (WeaponShootRange() - m_PlayerDistance > 200)
				SeekRandomWaypoint();
			
			/*
			// distance to the player
			if (m_PlayerPos.x < m_Pos.x)
				m_TargetPos.x = m_PlayerPos.x + WeaponShootRange()/2*(0.5f+frandom()*1.0f);
			else
				m_TargetPos.x = m_PlayerPos.x - WeaponShootRange()/2*(0.5f+frandom()*1.0f);
			*/
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
	
	
	//DoJumping();
	//Unstuck();

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 1 + frandom()*2;
	
}
