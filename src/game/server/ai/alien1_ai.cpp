#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "alien1_ai.h"


CAIalien1::CAIalien1(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	Player()->SetCustomSkin(4);
	m_StartPos = vec2(0, 0);
}


void CAIalien1::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	
	m_WaypointDir = vec2(0, 0);
	//Player()->SetRandomSkin();
	
	m_PowerLevel = 2;
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	m_Triggered = false;
	
	if (frandom() < 0.4f)
		pChr->GiveCustomWeapon(WEAPON_RIFLE);
	else if (frandom() < 0.4f)
		pChr->GiveCustomWeapon(WEAPON_CHAINSAW);
	else if (frandom() < 0.5f)
		pChr->GiveCustomWeapon(WEAPON_LASER);
	else
		pChr->GiveCustomWeapon(WEAPON_ELECTRIC);
	
	pChr->SetHealth(60);
}


void CAIalien1::ReceiveDamage(int CID, int Dmg)
{
	m_Triggered = true;
}


void CAIalien1::DoBehavior()
{
	m_Attack = 0;
	
	HeadToMovingDirection();

	SeekClosestEnemyInSight();

	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ReactToPlayer();
		
		m_Triggered = true;
		
		if (!m_MoveReactTime)
			m_MoveReactTime++;
		
		if (ShootAtClosestEnemy())
		{
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
	else if (!m_Triggered)
	{
		m_TargetPos = m_StartPos;
	}
	else
	{
		// seen enemy, but enemies in sight
		
		ShootAtClosestBuilding();
		
		if (SeekClosestEnemy())
			m_TargetPos = m_PlayerPos;
		else
		{
			//m_Triggered = false;
			//m_TargetPos = m_StartPos;
		}
	}


	if (abs(m_Pos.x - m_TargetPos.x) < 40 && abs(m_Pos.y - m_TargetPos.y) < 40)
	{
		// stand still
		m_Move = 0;
	}
	else
	{
		if (!m_MoveReactTime || m_MoveReactTime++ > 9)
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
	}
	
	Player()->GetCharacter()->m_SkipPickups = 999;

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 1 + frandom()*3;
	
}
