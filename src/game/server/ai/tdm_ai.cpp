#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "tdm_ai.h"


CAItdm::CAItdm(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
}


void CAItdm::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
	
	//if (g_Config.m_SvRandomWeapons)
	//	pChr->GiveRandomWeapon();
}


void CAItdm::DoBehavior()
{
	// power level
	m_PowerLevel = 20 - GameServer()->m_pController->CountPlayers(Player()->GetTeam())*1.5f;
	
	// reset jump and attack
	if (Player()->GetCharacter()->GetCore().m_JetpackPower < 10 || Player()->GetCharacter()->GetCore().m_Jetpack == 0)
		m_Jump = 0;
	
	m_Attack = 0;

	
	HeadToMovingDirection();

	SeekClosestEnemyInSight();
	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ShootAtClosestEnemy();
		ReactToPlayer();
	}
	else
		m_AttackTimer = 0;

	int f = 1000+m_EnemiesInSight*100;

	bool SeekEnemy = false;
	
	if (SeekClosestFriend())
	{
		m_TargetPos = m_PlayerPos;
		
		if (m_PlayerDistance < f)
			SeekEnemy = true;
	}
	else
		SeekEnemy = true;
	
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
		else
			SeekRandomWaypoint();
	}
	

	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint(20);
		//HookMove();
		//AirJump();
		
		// jump if waypoint is above us
		//if (abs(m_WaypointPos.x - m_Pos.x) < 60 && m_WaypointPos.y < m_Pos.y - 100 && frandom()*20 < 4)
		//	m_Jump = 1;
		
		//WallRun();
	}
	else
	{
		m_Hook = 0;
	}
	
	
	//DoJumping();
	//Unstuck();

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2 + frandom()*4;
	
}
