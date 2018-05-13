#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "pyro1_ai.h"


CAIpyro1::CAIpyro1(CGameContext *pGameServer, CPlayer *pPlayer, int Level)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	m_StartPos = vec2(0, 0);
	m_ShockTimer = 0;
	m_Triggered = false;
	m_TriggerLevel = 20 + rand()%20;
	
	m_Skin = SKIN_PYRO1+min(Level, 2);
	
	Player()->SetCustomSkin(m_Skin);
}


void CAIpyro1::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr); 
	m_TriggerLevel = 20 + rand()%20;
	
	m_WaypointDir = vec2(0, 0);
	
	int Level = g_Config.m_SvMapGenLevel;
	
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	
	if (m_Skin == SKIN_PYRO1)
	{
		if (frandom() < 0.5f)
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_FLAMER)));
		else
			pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(2, 1)));
	}
	else if (m_Skin == SKIN_PYRO2)
	{
		if (frandom() < 0.35f)
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_BAZOOKA)));
		else
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_BOUNCER)));
	}
	else if (m_Skin == SKIN_SKELETON1)
	{
		pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(1, 4), 3)));
	}
	
	
	pChr->SetHealth(90+min(Level*5.0f, 100.0f));
	pChr->SetArmor(80+min(Level*5.0f, 100.0f));
	m_PowerLevel = 8;
	m_TriggerLevel = 15 + rand()%5;
	
	m_ShockTimer = 10;
		
	if (!m_Triggered)
		m_ReactionTime = 100;
}

void CAIpyro1::OnCharacterDeath()
{
}

void CAIpyro1::ReceiveDamage(int CID, int Dmg)
{
	if (CID >= 0 && frandom() < Dmg*0.02f)
		m_Triggered = true;
	
	//m_ShockTimer = 10;
	//m_Attack = 0;
}


void CAIpyro1::DoBehavior()
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
			
			if (WeaponShootRange() - m_PlayerDistance > 200)
			{
				m_TargetPos = normalize(m_Pos - m_PlayerPos) * WeaponShootRange();
				GameServer()->Collision()->IntersectLine(m_Pos, m_TargetPos, 0x0, &m_TargetPos);
				//MoveTowardsWaypoint(true);
				//
				
				int Weapon = Player()->GetCharacter()->GetWeaponType();
				if (GetWeaponFiringType(Weapon) == WFT_NONE)
				{
					Shooting = false;
					m_TargetPos = m_Pos - m_PlayerDirection*3;
				}
			}
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
		ShootAtBlocks();
		
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
