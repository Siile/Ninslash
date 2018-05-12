#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "alien1_ai.h"


CAIalien1::CAIalien1(CGameContext *pGameServer, CPlayer *pPlayer, int Level)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	m_StartPos = vec2(0, 0);
	m_ShockTimer = 0;
	m_Triggered = false;
	m_TriggerLevel = 5 + rand()%5;
	
	m_Level = Level;
	
	m_Skin = SKIN_ALIEN1+min(Level, 2);

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
	
	if (m_Skin == SKIN_ALIEN3)
	{
		pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(4, 4), 2)));
		pChr->SetHealth(60+min((m_Level-1)*7, 300));
		pChr->SetArmor(60+min((m_Level-1)*7, 300));
		m_PowerLevel = 8;
		m_TriggerLevel = 15 + rand()%5;
	}
	if (m_Skin == SKIN_ALIEN2)
	{
		pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_CHAINSAW)));
		pChr->SetHealth(60+min((m_Level-1)*6, 300));
		pChr->SetArmor(60+min((m_Level-1)*6, 300));
		m_PowerLevel = 8;
		m_TriggerLevel = 15 + rand()%5;
	}
	else
	{
		if (frandom() < min(m_Level*0.1f, 1.0f))
			pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 1)));
		else if (frandom() < min(m_Level*0.1f, 1.0f))
			pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 4)));
		
		if (frandom() < 0.6f)
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GUN1)));
		else
			pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GUN2)));
		
		pChr->SetHealth(60+min((m_Level-1)*5, 300));
	}
	
	m_ShockTimer = 10;
		
	if (!m_Triggered)
		m_ReactionTime = 100;
}


void CAIalien1::ReceiveDamage(int CID, int Dmg)
{
	if (CID >= 0 && frandom() < Dmg*0.02f)
		m_Triggered = true;

	if (frandom() < Dmg*0.03f)
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
