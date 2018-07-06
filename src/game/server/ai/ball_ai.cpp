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
}


void CAIball::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	m_PowerLevel = g_Config.m_SvBotLevel;
	m_WaypointDir = vec2(0, 0);
	Player()->SetRandomSkin();
}


void CAIball::DoBehavior()
{
	// reset jump and attack
	if (Player()->GetCharacter()->GetCore().m_JetpackPower < 10 || Player()->GetCharacter()->GetCore().m_Jetpack == 0)
		m_Jump = 0;
	
	m_Attack = 0;

	HeadToMovingDirection();

	if (!GameServer()->m_pController->m_pBall)
	{
		m_ReactionTime = 2;
		return;
	}
	
	vec2 BallPos = GameServer()->m_pController->m_pBall->m_Pos;
	
	m_TargetPos = BallPos+vec2(0, -20);
	m_WaypointPos = m_TargetPos;
	MoveTowardsWaypoint(false);
	
	m_ReactionTime = 2;
}
