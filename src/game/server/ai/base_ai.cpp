#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "base_ai.h"


CAIbase::CAIbase(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
}


void CAIbase::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	m_PowerLevel = g_Config.m_SvBotLevel;
	m_WaypointDir = vec2(0, 0);
	pChr->m_SkipPickups = 999;
}


void CAIbase::ReceiveDamage(int CID, int Dmg)
{
	if (frandom() > Dmg*0.02f)
		return;
	
	if (CID < 0 || CID >= MAX_CLIENTS)
		return;
	
	int p = CID;
		
	CPlayer *pPlayer = GameServer()->m_apPlayers[p];
	if(!pPlayer)
		return;
		
	if (pPlayer == Player())
		return;

	CCharacter *pCharacter = pPlayer->GetCharacter();
	if (!pCharacter)
		return;
		
	if (!pCharacter->IsAlive())
		return;
		
	m_pTargetPlayer = pPlayer;
	m_PlayerDirection = m_pTargetPlayer->GetCharacter()->m_Pos - m_Pos;
	m_PlayerPos = m_pTargetPlayer->GetCharacter()->m_Pos;
	m_PlayerDistance = distance(m_pTargetPlayer->GetCharacter()->m_Pos, m_Pos);
}


void CAIbase::DoBehavior()
{
	m_Attack = 0;
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();

	
	// if we see a player
	
	if (m_EnemiesInSight > 0)
	{
		if (!ShootAtClosestEnemy())
		{
			if (!ShootAtClosestBuilding())
				ShootAtClosestMonster();
		}
		
		ReactToPlayer();
	}
	else
	{
		ShootAtClosestBuilding();
	}
	
	SeekClosestReactor();
	ShootAtBlocks();
	
	if (UpdateWaypoint())
	{
		MoveTowardsWaypoint();
	}
	else
	{
		SeekRandomWaypoint();
		m_TargetPos = m_WaypointPos;
		MoveTowardsWaypoint();
	}
	
	/*else
	{
		m_WaypointPos = m_TargetPos;
		MoveTowardsWaypoint(true);
	}*/
	
	
	//DoJumping();
	//Unstuck();

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 2;
	
}
