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
	m_Skin = 2;
	
	int Level = g_Config.m_SvMapGenLevel;
	
	if (frandom() < min(0.4f, Level*0.006f))
		m_Skin = 3;
	
	// "boss"
	if (frandom() < 0.5f && g_Config.m_SvInvBosses > 0)
	{
		m_Skin = 8;
		g_Config.m_SvInvBosses--;
	}
	
	pPlayer->SetCustomSkin(m_Skin);
	m_StartPos = vec2(0, 0);
	
	m_Triggered = false;
	m_TriggerLevel = 10 + rand()%5 + m_Skin;
}


void CAIrobot1::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	m_WaypointDir = vec2(0, 0);
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	
	
	if (m_Skin == 2)
	{
		m_PowerLevel = 6;
		pChr->SetHealth(80);
		pChr->GiveCustomWeapon(WEAPON_RIFLE);
		pChr->SetCustomWeapon(WEAPON_RIFLE);
	}
	else if (m_Skin == 3)
	{
		m_PowerLevel = 2;
		pChr->SetHealth(100);
		pChr->GiveCustomWeapon(WEAPON_CHAINSAW);
		pChr->SetCustomWeapon(WEAPON_CHAINSAW);
	}
	else if (m_Skin == 8)
	{
		m_PowerLevel = 8;
		pChr->SetHealth(140);
		pChr->GiveCustomWeapon(WEAPON_ELECTRIC, 1, 1);
		pChr->SetCustomWeapon(WEAPON_ELECTRIC);
	}
	
	if (!m_Triggered)
		m_ReactionTime = 150;
}


void CAIrobot1::ReceiveDamage(int CID, int Dmg)
{
	
}


void CAIrobot1::DoBehavior()
{
	m_Attack = 0;
	
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
				
				if (WeaponShootRange() - m_PlayerDistance > 300)
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
		// triggered, but no enemies in sight
		ShootAtClosestBuilding();
		
		if (SeekClosestEnemy())
			m_TargetPos = m_PlayerPos;
	}


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
	m_ReactionTime = 1 + rand()%3;
}
