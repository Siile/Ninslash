#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "robot1_ai.h"


CAIrobot1::CAIrobot1(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetCustomSkin(2);
	m_StartPos = vec2(0, 0);
}


void CAIrobot1::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	//Player()->SetRandomSkin();
	
	//Player()->SetCustomSkin(2);
	m_PowerLevel = 10;
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	m_Triggered = false;
	
	
	pChr->GiveCustomWeapon(WEAPON_RIFLE);
}


void CAIrobot1::DoBehavior()
{
	m_Attack = 0;
	
	HeadToMovingDirection();

	SeekClosestEnemyInSight();

	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ReactToPlayer();
		
		if (ShootAtClosestEnemy())
		{
			m_Triggered = true;
			m_TargetPos = m_Pos;
		}
		else
		{
			if (SeekClosestEnemy())
			{
				m_TargetPos = m_PlayerPos;
				
				if (WeaponShootRange() - m_PlayerDistance > 200)
					SeekRandomWaypoint();
			}
		}
	}
	else
		m_TargetPos = m_StartPos;


	if (abs(m_Pos.x - m_TargetPos.x) < 40 && abs(m_Pos.y - m_TargetPos.y) < 40)
	{
		// stand still
		m_Move = 0;
	}
	else
	{
		if (UpdateWaypoint())
		{
			MoveTowardsWaypoint();
		}
		else
		{
			m_WaypointPos = m_TargetPos;
			MoveTowardsWaypoint(true);
		}
	}
	
	Player()->GetCharacter()->m_SkipPickups = 999;
	

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 1 + frandom()*3;
	
}
