#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "robot1_ai.h"


CAIrobot1::CAIrobot1(CGameContext *pGameServer, CPlayer *pPlayer, int Level)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	
	m_Skin = SKIN_ROBO1+min(Level, 4);
	
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
	
	int Level = g_Config.m_SvMapGenLevel;
	
	if (frandom() < 0.4f)
		pChr->GetPlayer()->IncreaseGold(frandom()*4);
	
	if (m_Skin == SKIN_ROBO1)
	{
		m_PowerLevel = 6;
		pChr->SetHealth(50+min(Level*3.0f, 100.0f));
		pChr->SetArmor(50+min(Level*3.0f, 100.0f));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(3, 4)));
	}
	else if (m_Skin == SKIN_ROBO2)
	{
		m_PowerLevel = 2;
		pChr->SetHealth(100+min(Level*3.0f, 200.0f));
		pChr->SetArmor(100+min(Level*3.0f, 200.0f));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_CHAINSAW)));
	}
	else if (m_Skin == SKIN_ROBO3)
	{
		m_PowerLevel = 6;
		pChr->SetHealth(100+min(Level*4.0f, 300.0f));
		pChr->SetArmor(100+min(Level*4.0f, 300.0f));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetModularWeapon(1, 2)));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1)));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetStaticWeapon(SW_GRENADE1)));
		pChr->m_Kits = 1;
	}
	else if (m_Skin == SKIN_ROBO4)
	{
		m_PowerLevel = 10;
		pChr->SetHealth(200+min(Level*5.0f, 400.0f));
		pChr->SetArmor(200+min(Level*5.0f, 400.0f));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(3, 1+rand()%4), 2)));
		pChr->m_Kits = 1;
	}
	else if (m_Skin == SKIN_ROBO5)
	{
		m_PowerLevel = 14;
		pChr->SetHealth(200+min(Level*5.0f, 200.0f));
		pChr->SetArmor(200+min(Level*5.0f, 400.0f));
		pChr->GiveWeapon(GameServer()->NewWeapon(GetChargedWeapon(GetModularWeapon(6, 9), 2)));
		m_AttackOnDamage = true;
	}
	
	if (!m_Triggered)
		m_ReactionTime = 100;
}


void CAIrobot1::ReceiveDamage(int CID, int Dmg)
{
	if (CID >= 0 && frandom() < Dmg*0.01f)
		m_Triggered = true;
	
	if (m_AttackOnDamage)
	{
		m_Attack = 1;
		m_InputChanged = true;
		m_AttackOnDamageTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed();
	}
}


void CAIrobot1::DoBehavior()
{
	m_Attack = 0;
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();
	//bool Shooting = false;
	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ReactToPlayer();
		//m_Triggered = true;
		
		if (!m_MoveReactTime)
			m_MoveReactTime++;
		
		if (ShootAtClosestEnemy())
		{
			//Shooting = true;
			
			if (m_Skin == 1 && WeaponShootRange() - m_PlayerDistance > 100)
			{
				m_TargetPos = normalize(m_Pos - m_PlayerPos) * WeaponShootRange();
				GameServer()->Collision()->IntersectLine(m_Pos, m_TargetPos, 0x0, &m_TargetPos);
			}
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
		ShootAtBlocks();
		
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
	
	if (m_AttackOnDamageTick > GameServer()->Server()->Tick())
		m_Attack = 1;
	
	// next reaction in
	m_ReactionTime = 1 + rand()%3;
}
