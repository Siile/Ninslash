#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/ball.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "ball_ai.h"


CAIball::CAIball(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	pPlayer->SetRandomSkin();
	m_aGoalPos[TEAM_RED] = GameServer()->m_pController->GetGoalArea(TEAM_RED);
	m_aGoalPos[TEAM_BLUE] = GameServer()->m_pController->GetGoalArea(TEAM_BLUE);
}


void CAIball::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	m_PowerLevel = g_Config.m_SvBotLevel;
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
	
	m_Team = Player()->GetTeam();
	m_Role = rand()%3;
}


void CAIball::DoBehavior()
{
	// reset jump and attack
	if (Player()->GetCharacter()->GetCore().m_JetpackPower < 10 || Player()->GetCharacter()->GetCore().m_Jetpack == 0)
		m_Jump = 0;
	
	m_Attack = 0;

	HeadToMovingDirection();
	
	if (Player()->GetCharacter()->GetWeaponType() != WEAPON_NONE)
	{
		SeekClosestEnemyInSight();
		
		// if we see a player
		if (m_EnemiesInSight > 0)
			ShootAtClosestEnemy();
	}
	
	
	if (!GameServer()->m_pController->m_pBall)
	{
		m_ReactionTime = 2;
		return;
	}
	
	vec2 BallPos = GameServer()->m_pController->m_pBall->m_Pos+vec2(0, -20);
	
	if (m_Role == 0)
	{
		if (distance(m_Pos, m_aGoalPos[m_Team]) < 150)
			m_TargetPos = BallPos;
		else
			m_TargetPos = (BallPos+m_aGoalPos[m_Team])/2;
	}
	else if (m_Role == 1)
		m_TargetPos = BallPos - normalize(m_aGoalPos[!m_Team]-BallPos)*frandom()*300;
	else
		m_TargetPos = BallPos;
	
	
	if (Player()->GetCharacter()->GetWeaponType() == WEAPON_NONE)
		FindWeapon();
	
	m_WaypointPos = m_TargetPos;
	MoveTowardsWaypoint(false);
	
	if (frandom()*100.0f < 0.3f)
		m_Role = rand()%3;
	
	m_ReactionTime = 2;
}
