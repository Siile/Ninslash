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
	m_StartPos = vec2(0, 0);
	m_ShockTimer = 0;
	m_Triggered = false;
	m_TriggerLevel = 5 + rand()%5;
	
	// "boss"
	if (frandom() < 0.5f && g_Config.m_SvInvBosses > 0)
	{
		m_Skin = 11;
		g_Config.m_SvInvBosses--;
	}
	else
		m_Skin = 4;

	Player()->SetCustomSkin(m_Skin);
}


void CAIalien1::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	//Player()->SetRandomSkin();
	
	m_PowerLevel = 2;
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	
	if (m_Skin == 11)
	{
		pChr->GiveCustomWeapon(WEAPON_CHAINSAW, 1, 2);
		pChr->SetCustomWeapon(WEAPON_CHAINSAW);
		pChr->SetHealth(80);
		m_PowerLevel = 8;
		m_TriggerLevel = 15 + rand()%5;
	}
	else
	{
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
	
	
	m_ShockTimer = 10;
		
	if (!m_Triggered)
		m_ReactionTime = 150;
}


void CAIalien1::ReceiveDamage(int CID, int Dmg)
{
	//if (CID >= 0)
	//	m_Triggered = true;

	m_ShockTimer = 2 + Dmg/2;
	m_Attack = 0;
}


void CAIalien1::DoBehavior()
{
	m_Attack = 0;
	
	if (m_ShockTimer > 0 && m_ShockTimer--)
	{
		m_ReactionTime = 1 + frandom()*3;
		return;
	}
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();
	bool Shooting = false;
	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ReactToPlayer();
		//m_Triggered = true;
		
		if (!m_MoveReactTime)
			m_MoveReactTime++;
		
		if (ShootAtClosestEnemy())
		{
			Shooting = true;
		}
		else
		{
			if (SeekClosestEnemy())
			{
				m_TargetPos = m_PlayerPos;
				
				//if (WeaponShootRange() - m_PlayerDistance > 200)
				//	SeekRandomWaypoint();
			}
		}
	}
	else if (!m_Triggered)
	{
		m_TargetPos = m_StartPos;
	}
	else
	{
		// triggered, but no enemies in sight
		ShootAtClosestBuilding();
		
		if (SeekClosestEnemy())
			m_TargetPos = m_PlayerPos;
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
	m_ReactionTime = 1 + rand()%3;
}
