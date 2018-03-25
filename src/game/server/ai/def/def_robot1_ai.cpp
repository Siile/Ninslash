#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "def_robot1_ai.h"


CAIdefrobot1::CAIdefrobot1(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	Player()->SetCustomSkin(2);
	m_StartPos = vec2(0, 0);
	m_ShockTimer = 0;
}


void CAIdefrobot1::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	
	m_WaypointDir = vec2(0, 0);
	//Player()->SetRandomSkin();
	
	m_PowerLevel = 10;
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	m_Triggered = false;
	
	pChr->SetHealth(110);
	
	m_ShockTimer = 10;
}


void CAIdefrobot1::ReceiveDamage(int CID, int Dmg)
{
	//m_ShockTimer = 10;
	m_Attack = 0;
}


void CAIdefrobot1::DoBehavior()
{
	m_Attack = 0;	
	
	if (m_ShockTimer > 0 && m_ShockTimer--)
	{
		m_ReactionTime = 1 + frandom()*3;
		return;
	}
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();

	bool Jump = false;
	bool Shooting = false;
	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ReactToPlayer();
		
		if (ShootAtClosestEnemy())
		{
			Shooting = true;
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
	{
		//if (Player()->GetCharacter()->PlayerCollision() && frandom()*10 < 3)
		//	Jump = true;
			
		
		ShootAtClosestBuilding();
		
		SeekClosestReactor();
		
		/*
		if (SeekClosestEnemy())
			m_TargetPos = m_PlayerPos;
		else if(SeekRandomEnemy())
		{
			m_TargetPos = m_PlayerPos;
				
			if (WeaponShootRange() - m_PlayerDistance > 200)
				SeekRandomWaypoint();
		}
		*/
	}


	if ((Shooting && Player()->GetCharacter()->IsGrounded()) || (abs(m_Pos.x - m_TargetPos.x) < 40 && abs(m_Pos.y - m_TargetPos.y) < 40))
	{
		// stand still
		m_Move = 0;
		m_Jump = 0;
		m_Hook = 0;
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
	
	if (Jump)
		m_Jump = 1;
	
	Player()->GetCharacter()->m_SkipPickups = 999;

	RandomlyStopShooting();
	
	if (m_ShockTimer > 0 && m_ShockTimer--)
	{
		m_Attack = 0;
		m_Move = 0;
		m_Hook = 0;
		m_Jump = 0;
	}
	
	// next reaction in
	m_ReactionTime = 1 + frandom()*3;
	
}
