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
	m_PowerLevel = g_Config.m_SvBotLevel;
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
}

bool CAItdm::SeekFriend()
{
	if (SeekClosestFriend())
	{
		m_TargetPos = m_PlayerPos;
		return true;
	}
	
	return false;
}


void CAItdm::DoBehavior()
{
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

	
	
	int f = 800-max(400, m_EnemiesInSight*50);

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
				{
					if (SeekFriend() && m_PlayerDistance < 200)
						SeekRandomWaypoint();
				}
			}
		}
		else
			SeekRandomWaypoint();
	}
	
	if (Player()->GetCharacter()->GetWeaponType() == WEAPON_NONE)
		FindWeapon();
	else
		ShootAtBlocks();

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
	m_ReactionTime = 2;
	//m_ReactionTime = 2 + frandom()*4;
}
