#include <engine/shared/config.h>

#include "ai.h"
#include "entities/character.h"
#include "entities/staticlaser.h"
#include "entities/flag.h"
#include "entities/bomb.h"
#include <game/server/player.h>
#include "gamecontext.h"


CAI::CAI(CGameContext *pGameServer, CPlayer *pPlayer)
{
	m_pGameServer = pGameServer;
	m_pPlayer = pPlayer;
	m_pTargetPlayer = 0;
	
	m_pPath = 0;
	m_pVisible = 0;
	
	m_Special = -1;
	ResetEvents();
	
	Reset();
}


void CAI::Reset()
{
	if (m_pPath)
		delete m_pPath;
	
	m_WaypointUpdateNeeded = true;
	m_WayPointUpdateTick = 0;
	m_WayVisibleUpdateTick = 0;
	
	m_ChatterStartTick = 0;
	m_ChatterEndTick = 0;
	
	m_Sleep = 0;
	m_Stun = 0;
	m_ReactionTime = 20;
	m_NextReaction = m_ReactionTime;
	m_InputChanged = true;
	m_Move = 0;
	m_LastMove = 0;
	m_Jump = 0;
	m_LastJump = 0;
	m_LastAttack = 0;
	m_Hook = 0;
	m_LastHook = 0;
	m_LastPos = vec2(-9000, -9000);
	m_Direction = vec2(-1, 0);
	m_DisplayDirection = vec2(-1, 0);
	
	m_HookTick = 0;
	m_HookReleaseTick = 0;
	
	m_UnstuckCount = 0;
	
	m_WayPointUpdateWait = 0;
	m_WayFound = false;
	
	m_AutoWeaponChange = true;
	
	m_TargetTimer = 99;
	m_AttackTimer = 0;
	m_HookTimer = 0;
	m_HookReleaseTimer = 0;
	
	m_pTargetPlayer = 0;
	m_PlayerPos = vec2(0, 0);
	m_TargetPos = vec2(0, 0);
	m_PlayerSpotTimer = 0;
	m_PlayerSpotCount = 0;
	
	m_TurnSpeed = 0.2f;
	
	m_DontMoveTick = 0;
	m_WaypointDir = vec2(0, 0);
	
	m_OldTargetPos = vec2(0, 0);
	ClearEmotions();
	
	m_ItemUseTick = 0;
}


void CAI::OnCharacterSpawn(class CCharacter *pChr)
{
	pChr->SetCustomWeapon(W_HAMMER);
	Reset();
	m_WaypointPos = pChr->m_Pos;
	m_TargetPos = pChr->m_Pos;
}


void CAI::ResetEvents()
{
	for (int i = 0; i < NUM_EVENTS; i++)
	{
		m_Event[i] = false;
		m_EventTriggerTick[i] = 0;
	}
}


void CAI::TickEvents()
{
	for (int i = 0; i < NUM_EVENTS; i++)
	{
		if (m_Event[i] && m_EventTriggerTick[i] <= GameServer()->Server()->Tick())
		{
			OnEvent(i);
			m_Event[i] = false;
		}
	}
}


void CAI::TriggerEvent(int EventNum, float InHowManySeconds)
{
	if (EventNum < 0 || EventNum >= NUM_EVENTS)
		return;
	
	m_Event[EventNum] = true;
	m_EventTriggerTick[EventNum] = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*InHowManySeconds;
	
	if (EventNum >= EVENT_TALK1 && EventNum <= EVENT_TALK19)
	{
		m_ChatterStartTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*InHowManySeconds*0.3f;
		m_ChatterEndTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*InHowManySeconds;
	}
}


void CAI::OnEvent(int EventNum)
{
	if (EventNum == EVENT_DROPWEAPON)
		Player()->GetCharacter()->DropWeapon();
	
	if (EventNum == EVENT_DIE)
		Player()->GetCharacter()->m_DelayedKill = true;
	
	if (EventNum == EVENT_FLEE)
	{
		m_TargetPos = GameServer()->GetFarHumanSpawnPos();
		m_WaypointUpdateNeeded = true;
		m_TargetTimer = 99;
	}
}


void CAI::Zzz(int Time)
{
	if (!Player()->GetCharacter())
		return;
		
	if (m_Sleep < Time)
	{
		m_Sleep = Time;
		Player()->GetCharacter()->SetEmoteFor(EMOTE_HAPPY, Time*17, Time*17, true);
		GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_ZZZ);
	}
}


void CAI::Stun(int Time)
{
	if (!Player()->GetCharacter())
		return;

	
	if (m_Stun < Time)
	{
		m_Stun = Time;
		Player()->GetCharacter()->SetEmoteFor(EMOTE_SURPRISE, Time*17, Time*17, true);
	}
}


void CAI::Panic()
{
	Player()->GetCharacter()->SetEmoteFor(EMOTE_PAIN, 2, 2, false);
	m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*1;
	
	if (frandom()*10 < 7)
		m_Attack = 1;
	else
		m_Attack = 0;
}



void CAI::StandStill(int Time)
{
	m_Sleep = Time;
}


void CAI::UpdateInput(int *Data)
{
	m_InputChanged = false;
	Data[0] = m_Move;
	Data[1] = m_DisplayDirection.x; Data[2] = m_DisplayDirection.y;
	//Data[1] = m_Direction.x; Data[2] = m_Direction.y;
	
	Data[3] = m_Jump;
	Data[4] = m_Attack;
	Data[5] = m_Hook;
}




void CAI::AirJump()
{
	if (Player()->GetCharacter()->GetVel().y > 0 && m_WaypointPos.y + 80 < m_Pos.y )
	{
		if (!GameServer()->Collision()->FastIntersectLine(m_Pos, m_Pos+vec2(0, 120)) && frandom()*10 < 5)
			m_Jump = 1;
	}
}




void CAI::DoJumping()
{
	// skip at random when on ground
	if (frandom()*10 < 3 && Player()->GetCharacter()->IsGrounded())
	{
		m_Jump = 0;
		return;
	}
	
	if (Player()->GetCharacter()->GetCore().m_JetpackPower > 70)
	{
		//if (abs(m_Pos.x - m_WaypointPos.x) < 50 && m_Pos.y > m_WaypointPos.y)
		if (m_Pos.y > m_WaypointPos.y)
			m_Jump = 1;
	}

	/*
	if (Player()->GetCharacter()->IsGrounded() &&
		(GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 32, m_Pos.y) || GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 96, m_Pos.y)))
		m_Jump = 1;
		*/
		
	if (GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y - 64) && Player()->GetCharacter()->GetCore().m_Jetpack == 1)
		m_Jump = 0;
		
	// release jump
	//if (m_WaypointPos.y - 120 > m_Pos.y && Player()->GetCharacter()->GetCore().m_Jetpack == 1)
	//	m_Jump = 0;
}



bool CAI::UpdateWaypoint()
{
	if (m_WayPointUpdateTick + GameServer()->Server()->TickSpeed()*3 < GameServer()->Server()->Tick())
		m_WaypointUpdateNeeded = true;
	
	
	//if (distance(m_WaypointPos, m_LastPos) < 100) // || m_TargetTimer++ > 30)// && m_WayPointUpdateWait > 10)
	if (m_TargetTimer++ > 40 && (!m_pVisible || m_WaypointUpdateNeeded))
	{
		m_TargetTimer = 0;
		
		m_WayFound = false;
		
		// old inefficient path finding
		// prepare waypoints for path finding
		//GameServer()->Collision()->SetWaypointCenter(m_Pos);		
		//if (GameServer()->Collision()->FindWaypointPath(m_TargetPos))
			
		
		if (GameServer()->Collision()->AStar(m_Pos, m_TargetPos))
		{	
			if (m_pPath)
			{
				delete m_pPath;
				m_pVisible = 0;
			}
			m_pPath = GameServer()->Collision()->GetPath();
			GameServer()->Collision()->ForgetAboutThePath();
			
			if (!m_pPath)
				return false;
			
			m_pVisible = m_pPath->GetVisible(GameServer(), m_Pos-vec2(0, 16));
			
			m_WayPointUpdateWait = 0;
			m_WayFound = true;
			
			m_WaypointPos = m_pPath->m_Pos;
			m_WaypointDir = m_WaypointPos - m_Pos;
			
			//char aBuf[128]; str_format(aBuf, sizeof(aBuf), "Path found, lenght: %d", m_pPath->Length());
			//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "path", aBuf);
			
			vec2 LastPos = m_Pos;
			CWaypointPath *Next = m_pPath;
			
			m_WaypointUpdateNeeded = false;
			m_WayPointUpdateTick = GameServer()->Server()->Tick();
		}
		
		
		if (GameServer()->m_ShowWaypoints)
		{
			for (int i = 0; i < 10; i++)
				new CStaticlaser(&GameServer()->m_World, GameServer()->Collision()->m_aPath[i], GameServer()->Collision()->m_aPath[i+1], 10);
			
			new CStaticlaser(&GameServer()->m_World, m_Pos, m_WaypointPos, 20);
		}
	}
	
	
	m_WayPointUpdateWait++;
	
	
	if (m_pVisible)
	{
		m_WaypointPos = m_pVisible->m_Pos;
		m_WaypointDir = m_WaypointPos - m_Pos;
		
		m_pVisible = m_pVisible->GetVisible(GameServer(), m_Pos-vec2(0, 16));
		
		m_WayVisibleUpdateTick = GameServer()->Server()->Tick();
		
		//if (GameServer()->m_ShowWaypoints)
		//	new CStaticlaser(&GameServer()->m_World, m_Pos, m_WaypointPos, 10);
	}
	
	
	// left or right
	if (abs(m_WaypointDir.y)*2 < abs(m_WaypointDir.x))
	{
		if (Player()->GetCharacter()->IsGrounded() && abs(m_WaypointDir.x) > 128)
		{
			int Dir = m_WaypointDir.x < 0 ? -1 : 1;
			
			// simple pits
			if (!GameServer()->Collision()->IsTileSolid(m_Pos.x + Dir * 32, m_Pos.y + 16) ||
				!GameServer()->Collision()->IsTileSolid(m_Pos.x + Dir * 64, m_Pos.y + 16))
			{
				if (GameServer()->Collision()->IsTileSolid(m_Pos.x + Dir * 128, m_Pos.y + 16) ||
					GameServer()->Collision()->IsTileSolid(m_Pos.x + Dir * 196, m_Pos.y + 16))
					m_Jump = 1;
			}
				
		}
	}

	
	// check target
	if (distance(m_Pos, m_TargetPos) < 800)
	{
		if (!GameServer()->Collision()->FastIntersectLine(m_Pos, m_TargetPos))
		{
			m_WaypointPos = m_TargetPos;
			m_WaypointDir = m_WaypointPos - m_Pos;
		}
	}
	
	if (!m_WayFound && !m_pVisible)
	{
		return false;
		//m_WaypointPos = m_TargetPos;
		//m_WaypointDir = m_WaypointPos - m_Pos;
	}
	
	return true;
}



void CAI::WallRun()
{
	if (m_WaypointPos.y < m_Pos.y)
	{
		if (Player()->GetCharacter()->GetCore().m_Wallrun > 0 &&  m_WaypointPos.x < m_Pos.x)
		{
			if (GameServer()->Collision()->FastIntersectLine(m_Pos, m_Pos + vec2(0, -64)))
			{
				m_Jump = 1;
			}
			else
			{
				m_Jump = 0;
				m_Move = 0;
			}
		}
		else if (Player()->GetCharacter()->GetCore().m_Wallrun < 0 &&  m_WaypointPos.x > m_Pos.x)
		{
			if (GameServer()->Collision()->FastIntersectLine(m_Pos, m_Pos + vec2(0, -64)))
			{
				m_Jump = 1;
			}
			else
			{
				m_Jump = 0;
				m_Move = 0;
			}
		}
		/*
		else
		// jetpack
		{
			if (Player()->GetCharacter()->GetCore().m_JetpackPower > 30)
				m_Jump = 1;
		}
		*/
	}
}


void CAI::HookMove()
{
	
}




void CAI::JumpIfPlayerIsAbove()
{
		if (abs(m_PlayerPos.x - m_Pos.x) < 100 && m_Pos.y > m_PlayerPos.y + 100)
		{
			if (frandom() * 10 < 4)
				m_Jump = 1;
		}
}


void CAI::HeadToMovingDirection()
{
	m_Direction = m_WaypointPos - m_Pos;
	
	//if (m_Move != 0)
	//	m_Direction = vec2(m_Move, 0);
}

void CAI::Unstuck()
{
	if (abs(m_Pos.x - m_StuckPos.x) < 20)
	{
		if (++m_UnstuckCount > 10)
		{
			if (frandom() * 10 < 5)
				m_Move = -1;
			else
				m_Move = 1;
			
			/*
			if (frandom() * 10 < 4)
				m_Jump = 1;
			*/
		}
		
		/*
		if (m_UnstuckCount > 4)
		{
			if (frandom() * 10 < 4)
				m_Jump = 1;
		}
		*/
	}
	else
	{
		m_UnstuckCount = 0;
		m_StuckPos = m_Pos;
	}
	
	// death tile check
	if (Player()->GetCharacter()->GetVel().y > 0 && GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y+32)&CCollision::COLFLAG_DEATH)
	{
		m_Jump = 1;
	}
}



bool CAI::SeekBombArea()
{
	CFlag *BombArea = GameServer()->m_pController->GetClosestBase(m_LastPos);
	
	if (BombArea)
	{
		m_TargetPos = BombArea->m_Pos;
		return true;
	}
	
	return false;
}

	
bool CAI::SeekBomb()
{
	CBomb *Bomb = GameServer()->m_pController->GetBomb();
	
	if (Bomb)
	{
		m_TargetPos = Bomb->m_Pos;
		return true;
	}
	
	return false;
}


bool CAI::MoveTowardsPlayer(int Dist)
{
	if (abs(m_LastPos.x - m_PlayerPos.x) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_PlayerPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_PlayerPos.x)
		m_Move = -1;
		
	return false;
}


bool CAI::MoveTowardsTarget(int Dist)
{

	if (abs(m_LastPos.x - m_TargetPos.x) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_TargetPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_TargetPos.x)
		m_Move = -1;
		
	return false;
}


bool CAI::MoveTowardsWaypoint(int Dist)
{

	if (distance(m_LastPos, m_WaypointPos) < Dist)
	{
		m_Move = 0;
		return true;
	}
		
	if (m_LastPos.x < m_WaypointPos.x)
		m_Move = 1;
		
	if (m_LastPos.x > m_WaypointPos.x)
		m_Move = -1;
		
	return false;
}


void CAI::ReceiveDamage(int CID, int Dmg)
{
	// mainly for witch
	if (!m_EventTriggerTick[EVENT_STARTLE])
		TriggerEvent(EVENT_STARTLE);
	
	if (frandom()*10 < 5)
		m_Jump = 1;
	
	if (CID >= 0 && CID < 16)
	{
		m_aAnger[CID] += Dmg;
		m_aAnger[CID] *= 1.1f;
		
		m_aAttachment[CID] *= 0.9f;
		
		//if (frandom()*25 < 2 && m_EnemiesInSight > 1)
		//	m_PanicTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(2+frandom()*2);
	}
	else
	{
		// world damage
		for (int i = 0; i < 16; i++)
		{
			m_aAnger[i] += Dmg/2;
			m_aAnger[i] *= 1.1f;
		}
	}
}




void CAI::HandleEmotions()
{
	m_TotalAnger = 0.0f;
	
	for (int i = 0; i < 16; i++)
	{
		m_aAnger[i] *= 0.97f;
		m_aAttachment[i] *= 0.97f;
		
		m_TotalAnger += m_aAnger[i];
	}
	
	if (m_TotalAnger > 35.0f)
		Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 40, 40, false);
	
	if (m_PanicTick > GameServer()->Server()->Tick())
		Panic();
}




void CAI::ClearEmotions()
{
	m_PanicTick = 0;
	
	for (int i = 0; i < 16; i++)
	{
		m_aAnger[i] = 0.0f;
		m_aAttachment[i] = 0.0f;
	}
}



int CAI::WeaponShootRange()
{
	int Weapon = Player()->GetCharacter()->m_ActiveCustomWeapon;
	int Range = 40;
		
	if (Weapon >= 0 && Weapon < NUM_CUSTOMWEAPONS)
		Range = BotAttackRange[Weapon];
	
	return Range;
}



void CAI::ReactToPlayer()
{
	// angry face
	//if (m_PlayerSpotCount > 20)
	//	Player()->GetCharacter()->SetEmoteFor(EMOTE_ANGRY, 0, 1200);
	
	if (m_PlayerSpotCount == 20 && m_TotalAnger > 35.0f)
	{
		switch (rand() % 3)
		{
		case 0: GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_SPLATTEE); break;
		case 1: GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_EXCLAMATION); break;
		default: /*__*/;
		}
	}
			
	if (m_PlayerSpotCount == 80)
	{
		switch (rand() % 3)
		{
		case 0: GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_ZOMG); break;
		case 1: GameServer()->SendEmoticon(Player()->GetCID(), EMOTICON_WTF); break;
		default: /*__*/;
		}
	}
}



void CAI::ShootAtClosestHuman()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->m_IsBot)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 800 && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (pClosestCharacter && ClosestDistance < WeaponShootRange()*1.2f)
	{
		vec2 AttackDirection = vec2(m_PlayerDirection.x+ClosestDistance*(frandom()*0.3f-frandom()*0.3f), m_PlayerDirection.y+ClosestDistance*(frandom()*0.3f-frandom()*0.3f));
		
		if (m_HookTick < GameServer()->Server()->Tick() - 20)
			m_Direction = AttackDirection;
		
		// shooting part
		if (m_AttackTimer++ > g_Config.m_SvBotReactTime)
		{
			if (ClosestDistance < WeaponShootRange() && abs(atan2(m_Direction.x, m_Direction.y) - atan2(AttackDirection.x, AttackDirection.y)) < PI / 4.0f)
			{
				m_Attack = 1;
				if (frandom()*30 < 2 && !Player()->GetCharacter()->UsingMeleeWeapon())
					m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom());
			}
		}
	}
		
	if (m_AutoWeaponChange)
		Player()->GetCharacter()->AutoWeaponChange();
}



void CAI::ShootAtClosestEnemy()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam() && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 800 && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (pClosestCharacter && ClosestDistance < WeaponShootRange()*1.2f)
	{
		vec2 AttackDirection = vec2(m_PlayerDirection.x+ClosestDistance*(frandom()*0.3f-frandom()*0.3f), m_PlayerDirection.y+ClosestDistance*(frandom()*0.3f-frandom()*0.3f));
		
		if (m_HookTick < GameServer()->Server()->Tick() - 20)
			m_Direction = AttackDirection;
		
		// shooting part
		if (m_AttackTimer++ > g_Config.m_SvBotReactTime)
		{
			if (ClosestDistance < WeaponShootRange() && abs(atan2(m_Direction.x, m_Direction.y) - atan2(AttackDirection.x, AttackDirection.y)) < PI / 4.0f)
			{
				m_Attack = 1;
				if (frandom()*30 < 2 && !Player()->GetCharacter()->UsingMeleeWeapon())
					m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom());
			}
		}
	}
	
	if (m_AutoWeaponChange)
		Player()->GetCharacter()->AutoWeaponChange();
}


void CAI::RandomlyStopShooting()
{
	if (frandom()*20 < 4 && m_Attack == 1)
	{
		m_Attack = 0;
		
		m_AttackTimer = g_Config.m_SvBotReactTime*0.5f;
	}
}



bool CAI::SeekRandomHuman()
{
	if (m_pTargetPlayer && m_pTargetPlayer->GetCharacter() && m_pTargetPlayer->GetCharacter()->IsAlive())
	{
		m_PlayerDirection = m_pTargetPlayer->GetCharacter()->m_Pos - m_Pos;
		m_PlayerPos = m_pTargetPlayer->GetCharacter()->m_Pos;
		m_PlayerDistance = distance(m_pTargetPlayer->GetCharacter()->m_Pos, m_Pos);
		return true;
	}
	
	int i = 0;
	while (i++ < 9)
	{
		int p = rand()%MAX_CLIENTS;
		
		CPlayer *pPlayer = GameServer()->m_apPlayers[p];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->m_IsBot)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
		
		m_pTargetPlayer = pPlayer;
		m_PlayerDirection = m_pTargetPlayer->GetCharacter()->m_Pos - m_Pos;
		m_PlayerPos = m_pTargetPlayer->GetCharacter()->m_Pos;
		m_PlayerDistance = distance(m_pTargetPlayer->GetCharacter()->m_Pos, m_Pos);
		return true;
	}
	
	return false;
}


bool CAI::SeekRandomEnemy()
{
	if (m_pTargetPlayer && m_pTargetPlayer->GetCharacter() && m_pTargetPlayer->GetCharacter()->IsAlive())
	{
		m_PlayerDirection = m_pTargetPlayer->GetCharacter()->m_Pos - m_Pos;
		m_PlayerPos = m_pTargetPlayer->GetCharacter()->m_Pos;
		m_PlayerDistance = distance(m_pTargetPlayer->GetCharacter()->m_Pos, m_Pos);
		return true;
	}
	
	int i = 0;
	while (i++ < 9)
	{
		int p = rand()%MAX_CLIENTS;
		
		CPlayer *pPlayer = GameServer()->m_apPlayers[p];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam() && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
		
		m_pTargetPlayer = pPlayer;
		m_PlayerDirection = m_pTargetPlayer->GetCharacter()->m_Pos - m_Pos;
		m_PlayerPos = m_pTargetPlayer->GetCharacter()->m_Pos;
		m_PlayerDistance = distance(m_pTargetPlayer->GetCharacter()->m_Pos, m_Pos);
		return true;
	}
	
	return false;
}



bool CAI::SeekClosestFriend()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->GetTeam() != Player()->GetTeam() || !GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if ((!pClosestCharacter || Distance < ClosestDistance))
		{
			pClosestCharacter = pCharacter;
			ClosestDistance = Distance;
			m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
			m_PlayerPos = pCharacter->m_Pos;
		}
	}
	
	if (pClosestCharacter)
	{
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	return false;
}


bool CAI::SeekClosestHuman()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player() || GameServer()->IsBot(i))
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam() && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if ((!pClosestCharacter || Distance < ClosestDistance))
		{
			pClosestCharacter = pCharacter;
			ClosestDistance = Distance;
			m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
			m_PlayerPos = pCharacter->m_Pos;
		}
	}
	
	if (pClosestCharacter)
	{
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	return false;
}


bool CAI::SeekClosestEnemy()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam() && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if ((!pClosestCharacter || Distance < ClosestDistance))
		{
			pClosestCharacter = pCharacter;
			ClosestDistance = Distance;
			m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
			m_PlayerPos = pCharacter->m_Pos;
		}
	}
	
	if (pClosestCharacter)
	{
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	return false;
}


bool CAI::SeekClosestHumanInSight()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	m_EnemiesInSight = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->m_IsBot)
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 900 && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
			//!GameServer()->Collision()->IntersectLine(pCharacter->m_Pos, m_LastPos, NULL, NULL))
		{
			m_EnemiesInSight++;
			
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (m_EnemiesInSight == 0)
		m_DontMoveTick = 0;
	
	if (pClosestCharacter)
	{
		m_PlayerSpotCount++;
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	m_PlayerSpotCount = 0;
	return false;
}


bool CAI::SeekClosestEnemyInSight()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	m_EnemiesInSight = 0;
	
	// FIRST_BOT_ID, fix
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		
		if (pPlayer == Player())
			continue;
		
		if (pPlayer->GetTeam() == Player()->GetTeam() && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < 900 && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
			//!GameServer()->Collision()->IntersectLine(pCharacter->m_Pos, m_LastPos, NULL, NULL))
		{
			m_EnemiesInSight++;
			
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos;
				m_PlayerPos = pCharacter->m_Pos;
			}
		}
	}
	
	if (m_EnemiesInSight == 0)
		m_DontMoveTick = 0;
	
	if (pClosestCharacter)
	{
		m_PlayerSpotCount++;
		m_PlayerDistance = ClosestDistance;
		return true;
	}

	m_PlayerSpotCount = 0;
	return false;
}





void CAI::UseItems()
{
	/*
	if (m_ItemUseTick < GameServer()->Server()->Tick() - GameServer()->Server()->TickSpeed() * 10)
	{
		m_ItemUseTick = GameServer()->Server()->Tick();
		m_pPlayer->SelectItem(rand()%NUM_PLAYERITEMS);
	}
	*/
}


void CAI::Tick()
{
	m_NextReaction--;
	
	// character check & position update
	if (m_pPlayer->GetCharacter())
		m_Pos = m_pPlayer->GetCharacter()->m_Pos;
	else
		return;
	
	// skip if sleeping or stunned
	if (m_Sleep > 0 || m_Stun > 0)
	{
		if (m_Sleep > 0)
			m_Sleep--;
			
		if (m_Stun > 0)
			m_Stun--;
		
		m_Move = 0;
		m_Jump = 0;
		m_Hook = 0;
		m_Attack = 0;
		m_InputChanged = true;
		
		return;
	}
	
	TickEvents();
	HandleEmotions();
	
	// stupid AI can't even react to events every frame
	if (m_NextReaction <= 0)
	{
		m_NextReaction = m_ReactionTime;
	
		DoBehavior();
		UseItems();
		
		if (m_DontMoveTick > GameServer()->Server()->Tick())
		{
			m_Move = 0;
			m_Hook = m_LastHook;
			m_Jump = 0;
		}
		
		if (m_LastJump == 1 && Player()->GetCharacter()->GetCore().m_Jetpack == 0)
			m_Jump = 0;
		
		if (m_pPlayer->GetCharacter())
			m_LastPos = m_pPlayer->GetCharacter()->m_Pos;
		m_LastMove = m_Move;
		m_LastJump = m_Jump;
		m_LastAttack = m_Attack;
		m_LastHook = m_Hook;
		
		if (m_OldTargetPos != m_TargetPos)
		{
			m_OldTargetPos = m_TargetPos;
			m_WaypointUpdateNeeded = true;
		}
			
		m_InputChanged = true;
	}
	else
	{
		m_Attack = 0;
	}
	m_InputChanged = true;
	

	
	m_DisplayDirection.x += (m_Direction.x - m_DisplayDirection.x) / 4.0f;
	m_DisplayDirection.y += (m_Direction.y - m_DisplayDirection.y) / 4.0f;
}



