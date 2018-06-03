#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/entities/building.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "def_ai.h"


CAIdef::CAIdef(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
	m_State = AISTATE_IDLE;
}


void CAIdef::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	m_PowerLevel = g_Config.m_SvBotLevel;
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
	m_State = AISTATE_FINDSHOP;
	m_StateTimer = 0;
	m_Shopped = false;
}

bool CAIdef::SeekFriend()
{
	if (SeekClosestFriend())
	{
		m_TargetPos = m_PlayerPos;
		return true;
	}
	
	return false;
}


bool CAIdef::FindShop()
{

	CBuilding *pClosestBuilding = NULL;
	int ClosestDistance = 99999;
	
	CBuilding *apEnts[128];
	int Num = GameServer()->m_World.FindEntities(m_LastPos, 4000, (CEntity**)apEnts, 128, CGameWorld::ENTTYPE_BUILDING);

	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pBuilding = apEnts[i];

		if (pBuilding->m_Type != BUILDING_SHOP)
			continue;
		
		int Distance = distance(pBuilding->m_Pos, m_LastPos);
		
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			pClosestBuilding = pBuilding;
		}
	}
	
	if (pClosestBuilding)
	{
		m_ShopPos = pClosestBuilding->m_Pos;
		return true;
	}
	
	return false;
}
	
	

void CAIdef::DoBehavior()
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
		ShootAtClosestBuilding();
	
	
	if (m_State == AISTATE_FINDSHOP)
	{
		if (FindShop())
			m_State = AISTATE_SHOP;
	}
	else if (m_State == AISTATE_SHOP)
	{
		// navigate to shop
		if (abs(m_ShopPos.x - m_Pos.x) > 100 || abs(m_ShopPos.y - m_Pos.y) > 120)
			m_TargetPos = m_ShopPos;
		else
		{
			// shop
			if (!m_Shopped && m_StateTimer > 40)
				m_Shopped = GameServer()->Shop(Player(), m_StateTimer%4, true);
			
			if (m_StateTimer++ > 80)
			{
				m_StateTimer = 0;
				m_State = AISTATE_ATTACK;
			}
		}
	}
	else if (m_State == AISTATE_ATTACK)
	{
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
	}

	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint();
	}
	else
	{
		m_WaypointPos = m_TargetPos;
		MoveTowardsWaypoint(true);
	}
	

	//Build();
	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2;
	//m_ReactionTime = 2 + frandom()*4;
}
