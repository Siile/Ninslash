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
	m_ReactorPos = vec2(0.0f, 0.0f);
	m_NextTargetTick = 0;
	m_Role = rand()%99;
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
	
	bool SeekWaypoint = true;
	
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
			int ShopTime = 80;
			
			if (Player()->GetCharacter() && Player()->GetCharacter()->IsBombCarrier())
				ShopTime *= 1.75f;
			
			// shop
			if (!m_Shopped && m_StateTimer > ShopTime/2)
				m_Shopped = GameServer()->Shop(Player(), m_StateTimer%4, true);
			
			if (m_StateTimer++ > ShopTime)
			{
				m_StateTimer = 0;
				m_State = AISTATE_ATTACK;
			}
		}
	}
	else if (m_State == AISTATE_ATTACK)
	{
		// bomb carrier => find reactor
		if (Player()->GetCharacter() && Player()->GetCharacter()->IsBombCarrier())
		{
			if (GameServer()->m_pController->InBombArea(m_Pos))
			{
				m_TargetPos = m_Pos;
				m_WaypointPos = m_TargetPos;
				
				if (GetStaticType(Player()->GetCharacter()->GetWeaponType()) == SW_BOMB)
				{
					m_Attack = 1;
					m_Move = 0;
					m_Hook = 0;
					m_Jump = 0;
					SeekWaypoint = false;
				}
				else
					Player()->GetCharacter()->RandomizeInventory();
			}
			else
			{
				if (m_ReactorPos.x == 0.0f)
					m_ReactorPos = GameServer()->m_pController->GetAttackPos();
				else
					m_TargetPos = m_ReactorPos;
			}
		}
		else
		{
			bool TrySeekEnemy = true;
			bool TrySeekFriend = true;
			
			if (Player()->GetTeam() == TEAM_RED)
			{
				if (m_Role%5 < 3 && GameServer()->m_pController->m_BombStatus != BOMB_DISARMED)
				{
					if (GameServer()->m_pController->m_BombStatus != BOMB_ARMED ||  distance(m_Pos, GameServer()->m_pController->m_BombPos) > 40+frandom()*400)
					{
						TrySeekEnemy = false;
						m_TargetPos = GameServer()->m_pController->m_BombPos;
					}
					
					if (GameServer()->m_pController->m_BombStatus == BOMB_ARMED)
						TrySeekFriend = false;
				}
			}
			// blue team
			else
			{
				if (GameServer()->m_pController->m_BombStatus == BOMB_ARMED)
				{
					TrySeekEnemy = false;
					m_TargetPos = GameServer()->m_pController->m_BombPos;
				}
				else
				{
					if (m_NextTargetTick && m_NextTargetTick > GameServer()->Server()->Tick() && m_ReactorPos.x != 0.0f)
					{
						m_TargetPos = m_ReactorPos;
						TrySeekEnemy = false;
						
						if (GameServer()->m_pController->InBombArea(m_Pos))
							m_NextTargetTick = 0;
					}
					else
					{
						if (m_NextTargetTick < GameServer()->Server()->Tick())
						{
							m_NextTargetTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(5.0f + frandom()*10.0f);
							
							if (frandom() < 0.6f)
								m_ReactorPos = GameServer()->m_pController->GetAttackPos();
							else
								m_ReactorPos = vec2(0.0f, 0.0f);
						}
					}
				}
			}
			
			if (TrySeekEnemy)
			{
				// find ally or enemy
				int f = 800-max(400, m_EnemiesInSight*50);

				bool SeekEnemy = false;
				
				if (TrySeekFriend && SeekClosestFriend())
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
			}
		}
		
		if (Player()->GetCharacter()->GetWeaponType() == WEAPON_NONE)
			FindWeapon();
		else
			ShootAtBlocks();
	}

	if (SeekWaypoint)
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
	

	//Build();
	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2;
	//m_ReactionTime = 2 + frandom()*4;
}
