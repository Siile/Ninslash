#include <engine/shared/config.h>
#include <game/weapons.h>

#include "ai.h"
#include "entities/pickup.h"
#include "entities/character.h"
#include "entities/droid.h"
#include "entities/building.h"
#include "entities/staticlaser.h"
#include "entities/flag.h"
#include <game/server/player.h>
#include "gamecontext.h"


CAI::CAI(CGameContext *pGameServer, CPlayer *pPlayer)
{
	m_pGameServer = pGameServer;
	m_pPlayer = pPlayer;
	m_pTargetPlayer = 0;
	
	m_pPath = 0;
	m_pVisible = 0;

	m_TriggerLevel = 0;
	m_Triggered = false;
	
	m_Special = -1;
	ResetEvents();
	
	Reset();
}

CAI::~CAI()
{
	if (m_pPath)
		delete m_pPath;
}

void CAI::Reset()
{
	if (m_pPath)
	{
		delete m_pPath;
		m_pPath = NULL;
	}
	
	m_WaypointUpdateNeeded = true;
	m_WayPointUpdateTick = 0;
	m_WayVisibleUpdateTick = 0;
	
	m_ChatterStartTick = 0;
	m_ChatterEndTick = 0;
	
	m_PowerLevel = 0;
	m_DispersionTick = 0;
	
	m_AttackOnDamage = false;
	m_AttackOnDamageTick = 0;
	
	m_SendMove = 0;
	m_SendJump = 0;
	m_SendTurbo = 0;
	m_SendAttack = 0;
	m_SendDown = 0;
	m_InputUpdateSkip = 0;
	
	m_Sleep = 0;
	m_Stun = 0;
	m_ReactionTime = 20;
	m_NextReaction = m_ReactionTime;
	m_InputChanged = true;
	m_Move = 0;
	m_LastMove = 0;
	m_Jump = 0;
	m_Down = 0;
	m_Attack = 0;
	m_LastJump = 0;
	m_LastAttack = 0;
	m_Hook = 0;
	m_LastHook = 0;
	m_LastPos = vec2(-9000, -9000);
	m_Direction = vec2(-1, 0);
	m_DisplayDirection = vec2(-1, 0);
	
	if (frandom()*10 < 5)
	{
		m_Direction = vec2(1, 0);
		m_DisplayDirection = vec2(1, 0);
	}
	
	m_MoveReactTime = 0;
	m_HookTick = 0;
	m_HookReleaseTick = 0;
	
	m_UnstuckCount = 0;
	m_AirJump = 0;
	
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
	
	m_MoveType = MOVE_IDLE;
	
	m_DontMoveTick = 0;
	m_WaypointDir = vec2(0, 0);
	
	m_OldTargetPos = vec2(0, 0);
	ClearEmotions();
	
	m_ItemUseTick = 0;
	m_ChargeStartTick = 0;
}


void CAI::OnCharacterSpawn(class CCharacter *pChr)
{
	Reset();
	m_WaypointPos = pChr->m_Pos;
	m_TargetPos = pChr->m_Pos;
}

void CAI::OnCharacterDeath()
{
	
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


void CAI::Trigger(int TriggerLevel)
{
	if (TriggerLevel >= m_TriggerLevel)
	{
		m_Triggered = true;
		m_ReactionTime = 1;
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


void CAI::FindWeapon()
{
	CPickup *apEnts[60];
	float Radius = 1000;
	
	int Num = GameServer()->m_World.FindEntities(m_Pos, Radius, (CEntity**)apEnts, 60, CGameWorld::ENTTYPE_PICKUP);
	for(int i = 0; i < Num; i++)
	{
		if (apEnts[i]->IsWeapon())
		{
			vec2 p = apEnts[i]->m_Pos;
			if (!GameServer()->Collision()->IsInFluid(p.x, p.y))
				m_TargetPos = p;
		}
	}
}



void CAI::StandStill(int Time)
{
	m_Sleep = Time;
}


void CAI::UpdateInput(int *Data)
{
	m_InputChanged = false;
	Data[0] = m_SendMove;
	Data[1] = m_DisplayDirection.x; Data[2] = m_DisplayDirection.y;
	//Data[1] = m_Direction.x; Data[2] = m_Direction.y;
	
	Data[3] = m_SendJump;
	Data[4] = m_SendAttack;
	Data[5] = m_SendTurbo;
	Data[7] = m_SendDown;
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

	if (Player()->GetCharacter()->IsGrounded() && GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 32, m_Pos.y))
		m_Jump = 1;
	
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
	//new CStaticlaser(&GameServer()->m_World, m_Pos, m_WaypointPos, 10);
	
	if (m_WayPointUpdateTick + GameServer()->Server()->TickSpeed()*(4+frandom()*4) < GameServer()->Server()->Tick())
		m_WaypointUpdateNeeded = true;
		
	if (distance(m_Pos, m_TargetPos) < 1200 && !GameServer()->Collision()->FastIntersectLine(m_Pos, m_TargetPos))
	{
		m_WaypointPos = m_TargetPos;
		m_WaypointDir = m_WaypointPos - m_Pos;
		return true;
	}
	
	if (m_WaypointUpdateNeeded)
	{
		m_WaypointUpdateNeeded = false;
		m_WayPointUpdateTick = GameServer()->Server()->Tick();

		//if (GameServer()->Collision()->AStar(m_Pos + vec2(0, -16), m_TargetPos))
		if (GameServer()->Collision()->AStar(m_TargetPos, m_Pos + vec2(0, -16)))
		{	
			if (m_pPath)
				delete m_pPath;
			
			m_pPath = GameServer()->Collision()->GetPath();
			GameServer()->Collision()->ForgetAboutThePath();
			
			if (!m_pPath)
				return false;
			
			m_pVisible = m_pPath->GetVisible(GameServer(), m_Pos-vec2(0, 16));
			
			//for (int i = 0; i < 20; i++)
			//	new CStaticlaser(&GameServer()->m_World, GameServer()->Collision()->m_aPath[i], GameServer()->Collision()->m_aPath[i+1], 5+i*2);
			
			if (m_pVisible)
			{
				//m_WaypointPos = m_pPath->m_Pos;
				m_WaypointPos = m_pVisible->m_Pos;
				m_WaypointDir = m_WaypointPos - m_Pos;
				return true;
			}
			else
				return false;
			
			//new CStaticlaser(&GameServer()->m_World, m_Pos, m_TargetPos, 20);
			
		}
	}
	
	if (m_pVisible && abs(m_WaypointPos.x - m_pVisible->m_Pos.x) < 70 && abs(m_WaypointPos.y - m_pVisible->m_Pos.y) < 70) // m_TargetTimer++ > 10
	{
		m_TargetTimer = 0;
		m_pVisible = m_pVisible->GetVisible(GameServer(), m_Pos-vec2(0, 16));
		m_WaypointPos = m_pVisible->m_Pos;
		m_WaypointDir = m_WaypointPos - m_Pos;
		
		return true;
	}
	
	return false;
	
	
	
	
	
	
	
	
	//if (distance(m_WaypointPos, m_LastPos) < 100) // || m_TargetTimer++ > 30)// && m_WayPointUpdateWait > 10)
	if (m_TargetTimer++ > 80 && (!m_pVisible || m_WaypointUpdateNeeded))
	{
		m_TargetTimer = 0;
		
		m_WayFound = false;
		
		vec2 To = m_TargetPos-m_Pos;
		
		if (length(To) > 200)
			To = normalize(m_TargetPos-m_Pos)*200;
		
		To += m_Pos;
		To.y -= 70;
		
		GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To);
		
		if (GameServer()->Collision()->AStar(To, m_TargetPos))
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
			
			m_WaypointUpdateNeeded = false;
			m_WayPointUpdateTick = GameServer()->Server()->Tick();
		}
		
		if (m_WayFound)
		{
			if (distance(GameServer()->Collision()->m_aPath[GameServer()->Collision()->m_PathLen], m_TargetPos) > distance(m_Pos, m_TargetPos))
			{
				m_WayFound = false;
				m_WaypointPos = m_TargetPos;
			}
		}
	}
	
	
	m_WayPointUpdateWait++;
	
	
	if (m_pVisible)
	{
		m_WaypointPos = m_pVisible->m_Pos;
		m_WaypointDir = m_WaypointPos - m_Pos;
		
		m_pVisible = m_pVisible->GetVisible(GameServer(), m_Pos-vec2(0, 16));
		
		m_WayVisibleUpdateTick = GameServer()->Server()->Tick();
	}

	
	// check target
	
	if (distance(m_Pos, m_TargetPos) < 600 && !GameServer()->Collision()->FastIntersectLine(m_Pos, m_TargetPos))
	{
		m_WaypointPos = m_TargetPos;
		m_WaypointDir = m_WaypointPos - m_Pos;
		return true;
	}

	return m_WayFound;
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





void CAI::HeadToMovingDirection()
{
	if (abs(m_WaypointPos.x - m_Pos.x) + abs(m_WaypointPos.y - m_Pos.y) > 4)
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
	/*
	if (Player()->GetCharacter()->GetVel().y > 0 && GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y+32)&CCollision::COLFLAG_DEATH)
	{
		m_Jump = 1;
	}
	*/
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





bool CAI::MoveTowardsWaypoint(bool Freestyle)
{
	//m_Jump = 0;
	//m_Hook = 0;

	bool OnWall = Player()->GetCharacter()->GetCore().m_OnWall;
	
	// handle basic movements
	int LastMoveType = -1;
	int i = 0;
	
	while (LastMoveType != m_MoveType && i < 10)
	{
		LastMoveType = m_MoveType;
		i++;
		
		switch (m_MoveType)
		{
			case MOVE_IDLE:
			{
				m_Move = 0;
				
				/*
				if (rand()%29 == 2)
					m_Move = -1;
				else if (rand()%29 == 2)
					m_Move = 1;
				*/
			}
			break;
				
			case MOVE_LEFT:
			{
				m_Move = -1;
				m_Down = 0;
				
				if (frandom() < 0.05f)
					m_Down = 1;
				
				// drop down from platforms
				/*
				if (m_WaypointPos.y > m_Pos.y && abs(Player()->GetCharacter()->GetCore().m_Vel.x) < 2.0f && GameServer()->Collision()->IsPlatform(m_Pos.x, m_Pos.y+48))
				{
					m_Down = 1;
				}
				
				if (Player()->GetCharacter()->IsGrounded())
				{
					m_AirJump = 0;
					
					if (GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 24, m_Pos.y+30))
						m_Jump = 1;
					else if (GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 32, m_Pos.y - 64))
						m_Down = 1;
				}
				
				if (!OnWall)
				{
					if (m_EnemiesInSight > 0 && Player()->GetCharacter()->GetCore().m_JetpackPower > 150 && abs(Player()->GetCharacter()->GetCore().m_Vel.x) < 12)
					{
						m_Hook = 1;
						m_Direction = m_WaypointDir;
					}
				}
				else
				{
					if (m_WaypointPos.y < m_Pos.y)
						m_Jump = 1;
					else
						m_Down = 1;
				}
				
				if (Player()->GetCharacter()->IsGrounded() && m_EnemyInLine && abs(Player()->GetCharacter()->GetCore().m_Vel.x) > 8)
					m_Down = 1;
				*/
			}
			break;
				
			case MOVE_RIGHT:
			{
				m_Move = 1;
				m_Down = 0;
				
				if (frandom() < 0.05f)
					m_Down = 1;
				
				/*
				// drop down from platforms
				if (m_WaypointPos.y > m_Pos.y && abs(Player()->GetCharacter()->GetCore().m_Vel.x) < 2.0f && GameServer()->Collision()->IsPlatform(m_Pos.x, m_Pos.y+48))
				{
					m_Down = 1;
				}
				
				if (Player()->GetCharacter()->IsGrounded() )
				{
					m_AirJump = 0;
					
					if (GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 24, m_Pos.y+30))
						m_Jump = 1;
					else if (GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 32, m_Pos.y - 64))
						m_Down = 1;
				}
				
				if (!OnWall)
				{
					if (m_EnemiesInSight > 0 && Player()->GetCharacter()->GetCore().m_JetpackPower > 150 && abs(Player()->GetCharacter()->GetCore().m_Vel.x) < 12)
					{
						m_Hook = 1;
						m_Direction = m_WaypointDir;
					}
				}
				else
				{
					if (m_WaypointPos.y < m_Pos.y)
						m_Jump = 1;
					else
						m_Down = 1;
				}
				
				if (Player()->GetCharacter()->IsGrounded() && m_EnemyInLine && abs(Player()->GetCharacter()->GetCore().m_Vel.x) > 8)
					m_Down = 1;
				*/
			}
			break;
				
			
			case MOVE_UPLEFT:
			{
				m_Jump = 1;
				m_Move = -1;
				
				/*
				if (Player()->GetCharacter()->Wallrun())
				{
					m_Move = 0;
					m_Jump = 0;
					m_Hook = 0;
					
					if (!GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y-48))
						break;
				}
				
				m_Jump = 1;
				m_Move = -1;
				
				int Treshold = 100;
				if (Player()->GetCharacter()->IsGrounded())
				{
					Treshold += 50;
					
					// load jetpack for a while if needed
					if (Player()->GetCharacter()->GetCore().m_JetpackPower < 125)
					{
						m_Move = 0;
						m_Hook = 0;
						m_Jump = 0;
						Treshold = 500;
					}
				}
				else
				{
					//if (!m_AirJump && m_PowerLevel >= 8 && Player()->GetCharacter()->GetCore().m_Vel.y > 0.0f && !GameServer()->Collision()->IsTileSolid(m_Pos.x-24, m_Pos.y-64))
					//	m_AirJump++;
					
					if (Player()->GetCharacter()->GetCore().m_JetpackPower > 20)
						m_Jump = 1;
				}
				
				if (Player()->GetCharacter()->GetCore().m_JetpackPower > Treshold)
				{
					m_Hook = 1;
					m_Direction = vec2(-1, -0.75f);
				}
				*/
			};
			break;
				
			case MOVE_UPRIGHT:
			{
				m_Jump = 1;
				m_Move = 1;
				
				/*
				if (Player()->GetCharacter()->Wallrun())
				{
					m_Move = 0;
					m_Jump = 0;
					m_Hook = 0;
					
					if (!GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y-48))
						break;
				}
				
				m_Jump = 1;
				m_Move = 1;
				
				int Treshold = 100;
				if (Player()->GetCharacter()->IsGrounded())
				{
					Treshold += 50;
					
					// load jetpack for a while if needed
					if (Player()->GetCharacter()->GetCore().m_JetpackPower < 100)
					{
						m_Move = 0;
						m_Hook = 0;
						m_Jump = 0;
						Treshold = 500;
					}
				}
				else
				{
					//if (!m_AirJump && m_PowerLevel >= 8 && Player()->GetCharacter()->GetCore().m_Vel.y > 0.0f && !GameServer()->Collision()->IsTileSolid(m_Pos.x+24, m_Pos.y-64))
					//	m_AirJump++;
					
					if (Player()->GetCharacter()->GetCore().m_JetpackPower > 20)
						m_Jump = 1;
				}
				
				if (Player()->GetCharacter()->GetCore().m_JetpackPower > Treshold)
				{
					m_Hook = 1;
					m_Direction = vec2(1, -0.75f);
				}
				*/
			};
			break;
			
				
			case MOVE_DOWN:
			{
				m_Down = 1;
				m_Jump = 0;
				m_Hook = 0;
				m_Move = 0;
				
				/*
				if (GameServer()->Collision()->IsPlatform(m_Pos.x, m_Pos.y+48))
				{
					if (rand()%9 < 3)
						m_Move = 0;
				}
				else
				if (GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y+48))
				{
					if (rand()%9 < 4)
						m_Move = -1;
					else if (rand()%9 < 4)
						m_Move = 1;
				}
				*/
			}
			break;
				
			case MOVE_UP:
			{
				m_Jump = 1;
				m_Move = 0;
				
				if (frandom() < 0.02f)
					m_Move = 1;
				if (frandom() < 0.02f)
					m_Move = -1;
				
				/*
				if (Player()->GetCharacter()->Wallrun())
				{
					m_Move = 0;
					m_Jump = 0;
					m_Hook = 0;
					
					if (!GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y-48))
						break;
				}
				
				if (Player()->GetCharacter()->IsGrounded())
				{
					if (Player()->GetCharacter()->GetCore().m_JetpackPower > 150)
					{
						m_Jump = 1;
						if (rand()%6 == 2)
							m_Move = -1;
						else if (rand()%6 == 2)
							m_Move = 1;
					}
					else
					// load jetpack for a while if needed
					{
						m_Move = 0;
						m_Hook = 0;
					}
				}
				else
				{
					if (Player()->GetCharacter()->GetCore().m_JetpackPower > 50)
					{
						m_Hook = 1;
						m_Direction = vec2(0, -1);
					}

					if (Player()->GetCharacter()->GetCore().m_JetpackPower > 20)
						m_Jump = 1;
					
					if (Player()->GetCharacter()->GetCore().m_OnWall)
					{
						m_Jump = 1;
						//m_Move *= -1;
					}
					
					if (GameServer()->Collision()->IsTileSolid(m_Pos.x + 48, m_Pos.y-32))
					{
						//m_MoveType = MOVE_UPRIGHT;
						m_Move = 1;
					}
						
					else if (GameServer()->Collision()->IsTileSolid(m_Pos.x - 48, m_Pos.y-32))
					{
						//m_MoveType = MOVE_UPLEFT;
						m_Move = -1;
					}
						
					
					//if (GameServer()->Collision()->FastIntersectLine(m_Pos+vec2(0, -32), m_Pos+vec2(0, -64)))
					//	Freestyle = true;
					
					if (GameServer()->Collision()->FastIntersectLine(m_Pos+vec2(-24, -32), m_Pos+vec2(-24, -48)))
						m_MoveType = MOVE_UPRIGHT;
					else if (GameServer()->Collision()->FastIntersectLine(m_Pos+vec2(+24, -32), m_Pos+vec2(+24, -48)))
						m_MoveType = MOVE_UPLEFT;
				}
			*/
			}
			break;
			
			default:
				break;
		};
		
		//if (length(normalize(m_DisplayDirection)-normalize(m_TargetPos)) < 0.3f && GameServer()->Collision()->IntersectLine(m_Pos, m_Pos+normalize(m_DisplayDirection)*400, 0x0, 0x0))
		if (m_DisplayDirection.y < 0 && m_MoveType != MOVE_IDLE && m_MoveType != MOVE_DOWN && !GameServer()->Collision()->IntersectLine(m_Pos, m_Pos+normalize(m_DisplayDirection)*150, 0x0, 0x0) && GameServer()->Collision()->IntersectLine(m_Pos+normalize(m_DisplayDirection)*150, m_Pos+normalize(m_DisplayDirection)*350, 0x0, 0x0))
		{
			m_Hook = 1;
			m_HookTimer = 10+rand()%20;
		}
		
		if (frandom() < 0.02f)
			m_Jump = 1;
		
		if ((m_HookTimer && --m_HookTimer == 0) || m_HookTimer <= 0)
			m_Hook = 0;
		
		if (Freestyle)
		{
			if (GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 32, m_Pos.y - 64) &&
				!GameServer()->Collision()->IsTileSolid(m_Pos.x + m_Move * 32, m_Pos.y - 24))
				m_Down = 1;
			
			if ((m_MoveType == MOVE_LEFT || m_MoveType == MOVE_UPLEFT) &&
				GameServer()->Collision()->IsTileSolid(m_Pos.x - 24, m_Pos.y-14))
			{
				m_Move = 1;
				if (OnWall)
					m_Jump = 1;
			}
			if ((m_MoveType == MOVE_RIGHT || m_MoveType == MOVE_UPRIGHT) &&
				GameServer()->Collision()->IsTileSolid(m_Pos.x + 24, m_Pos.y-14))
			{
				m_Move = -1;
				if (OnWall)
					m_Jump = 1;
			}
		}
	}
	
	
	// teach some basic moves to override shitty waypoint logic
	float Vel = Player()->GetCharacter()->GetCore().m_Vel.x;
	
	// abuse the bug / use the feature that lets the character keep sliding if coming from tunnel, don't fix it
	/*
	if (Player()->GetCharacter()->GetCore().m_Slide > 0)
	{
		if (!Player()->GetCharacter()->GetCore().IsGrounded())
		{
			if (Vel < -3)
			{
				m_Move = -1;
				m_Hook = 1;
				m_Direction = vec2(-50.0f, -100.0f);
			}
			else
			if (Vel > 3)
			{
				m_Move = 1;
				m_Hook = 1;
				m_Direction = vec2(50.0f, -100.0f);
			}
		}
		else
		{
			// save some fuel for the possible flight
			if (Player()->GetCharacter()->GetCore().m_JetpackPower < 60)
				m_Hook = 0;
			
			m_Move = Vel < 0.0f ? -1 : 1;
		}
	}
	*/
	
	
	// avoid acid pools
	if (GameServer()->Collision()->IsInFluid(m_Pos.x, m_Pos.y + 32) ||
		GameServer()->Collision()->IsInFluid(m_Pos.x, m_Pos.y + 64))
	{
		m_Jump = 1;
		
		if (Vel < -2.0f)
			m_Move = -1;
		if (Vel > 2.0f)
			m_Move = 1;
	}
	
	
	// jump over other characters sometimes
	if (abs(Vel) > 1 && Player()->GetCharacter()->PlayerCollision() && frandom()*10 < 2)
		m_Jump = true;
	
	vec2 Pos = Player()->GetCharacter()->GetCore().m_Pos;
	
	// avoid sawblades
	for (int x = 1; x < 6; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			if ((m_Move == -1 && GameServer()->Collision()->IsSawblade(Pos + vec2(-x, y-1)*32.0f)) ||
				(m_Move == 1 && GameServer()->Collision()->IsSawblade(Pos + vec2(x, y-1)*32.0f)))
			{
				m_Move *= -1;
				
				// load fuel
				if (Player()->GetCharacter()->GetCore().IsGrounded())
				{
					if (Player()->GetCharacter()->GetCore().m_JetpackPower < 50)
					{
						m_Jump = 0;
						m_Hook = 0;
					}
					
					if (Player()->GetCharacter()->GetCore().m_JetpackPower > 60)
						m_Jump = 1;
				}
			}
		}
	}
	
	if (frandom() < 0.1f)
		m_Jump = 0;
	
	if (Freestyle)
	{
		if (m_Move < 0 && (m_MoveType == MOVE_RIGHT || m_MoveType == MOVE_UPRIGHT))
			m_MoveType = MOVE_LEFT;
		if (m_Move > 0 && (m_MoveType == MOVE_LEFT || m_MoveType == MOVE_UPLEFT))
			m_MoveType = MOVE_RIGHT;
		
		if (m_MoveType == MOVE_IDLE)
		{
			if (m_Pos.x > m_TargetPos.x)
				m_MoveType = MOVE_LEFT;
			else
				m_MoveType = MOVE_RIGHT;
		}
		return true;
	}
	
	// movement logic
	if (distance(m_LastPos, m_WaypointPos) < 40)
	{
		m_MoveType = MOVE_IDLE;
		return true;
	}
	
	if (abs(m_LastPos.x - m_WaypointPos.x)*2.0f < abs(m_LastPos.y - m_WaypointPos.y))
	{
		if (m_LastPos.y > m_WaypointPos.y)
			m_MoveType = MOVE_UP;
		else
			m_MoveType = MOVE_DOWN;
	}
	else
	//if (abs(m_LastPos.x - m_WaypointPos.x) > 32)
	{
		if (m_LastPos.x < m_WaypointPos.x)
		{
			m_MoveType = MOVE_RIGHT;
			if (m_LastPos.y > m_WaypointPos.y + 200 || (m_LastPos.y > m_WaypointPos.y + 64 && abs(m_LastPos.x - m_WaypointPos.x) < 400))
				m_MoveType = MOVE_UPRIGHT;
		}
			
		if (m_LastPos.x > m_WaypointPos.x)
		{
			m_MoveType = MOVE_LEFT;
			if (m_LastPos.y > m_WaypointPos.y + 200 || (m_LastPos.y > m_WaypointPos.y + 64 && abs(m_LastPos.x - m_WaypointPos.x) < 400))
				m_MoveType = MOVE_UPLEFT;
		}
	}
	/*
	else
	{
		if (m_LastPos.y > m_WaypointPos.y)
			m_MoveType = MOVE_UP;
		else
			m_MoveType = MOVE_IDLE;
	}
	*/

	/*
	bool Floor = false;
	for (int y = 1; y < 30; y++)
		if (GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y + y*32))
		{
			Floor = true;
			break;
		}
	
	if (!Floor && (m_MoveType != MOVE_DOWN || m_MoveType != MOVE_UP || m_MoveType != MOVE_UPLEFT || m_MoveType != MOVE_UPRIGHT))
		m_MoveType = MOVE_UP;
	*/
	
	if (m_MoveType == MOVE_DOWN && GameServer()->Collision()->IsTileSolid(m_Pos.x, m_Pos.y + 40))
	{
		if (m_Pos.x > m_TargetPos.x)
			m_MoveType = MOVE_LEFT;
		else if (m_Pos.x < m_TargetPos.x)
			m_MoveType = MOVE_LEFT;
	}
	
	return false;
}



void CAI::Build()
{
	CBuilding *apEnts[3];
	int Num = GameServer()->m_World.FindEntities(m_Pos, 20, (CEntity**)apEnts, 3, CGameWorld::ENTTYPE_BUILDING);
	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pBuilding = apEnts[i];

		// found it
		if (pBuilding->m_Type == BUILDING_STAND || (pBuilding->m_Type == BUILDING_TURRET && pBuilding->m_aStatus[BSTATUS_NOPE] == 1))
		{
			Player()->GetCharacter()->DropWeapon();
			break;
		}
	}
	
	//if (frandom()*20 < 2 && Player()->GetCharacter()->IsGrounded() && Player()->GetCharacter()->m_Kits > 1)
	//	Player()->UseKit(KIT_TURRET, Player()->GetCharacter()->m_Pos);
}




void CAI::ReceiveDamage(int CID, int Dmg)
{
	// mainly for witch
	/*
	if (!m_EventTriggerTick[EVENT_STARTLE])
		TriggerEvent(EVENT_STARTLE);
	
	if (frandom()*20 < 2)
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
	*/
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
	int Weapon = Player()->GetCharacter()->GetWeaponType();
	return AIAttackRange(Weapon);
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


void CAI::ShootAtBlocks()
{
	if (GameServer()->Collision()->CheckBlocks(m_Pos + normalize(m_Direction)*30) || GameServer()->Collision()->CheckBlocks(m_Pos + normalize(m_Direction)*60) || GameServer()->Collision()->CheckBlocks(m_Pos + normalize(m_Direction)*120))
		m_Attack = 1;
	
	return;
	
	/*
	float Range = WeaponShootRange();
	
	if (Range < 40)
		return;
	
	if (Range > 1000)
		Range = 1000;
	
	vec2 To = m_Pos + vec2(0, -6) + m_Direction * Range;
	if (GameServer()->Collision()->IntersectBlocks(m_Pos, To))
	{
		m_Attack = 1;
		
		//if (frandom()*20 < 2 && Range > 200)
		//	m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom());
	}
	*/
}


void CAI::ShootAtClosestHuman()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;	
	int Weapon = Player()->GetCharacter()->GetWeaponType();
	
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
		if (Distance < AIAttackRange(Weapon) && 
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
		if (m_AttackTimer++ > max(0, 20-m_PowerLevel))
		{
			if (ClosestDistance < WeaponShootRange() && abs(atan2(m_Direction.x, m_Direction.y) - atan2(AttackDirection.x, AttackDirection.y)) < PI / 4.0f)
			{
				m_Attack = 1;
				if (frandom()*30 < 2 && WeaponShootRange() > 200)
					m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom());
			}
		}
	}
		
	//if (m_AutoWeaponChange)
	//	Player()->GetCharacter()->AutoWeaponChange();
}



bool CAI::ShootAtClosestEnemy()
{
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	int Weapon = Player()->GetCharacter()->GetWeaponType();
	
	m_EnemyInLine = false;
	
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
		
		if (GameServer()->m_pController->IsCoop() && pCharacter->m_IsBot)
			continue;
		
		
		int Distance = distance(pCharacter->m_Pos, m_LastPos);
		if (Distance < AIAttackRange(Weapon) && 
			!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, m_LastPos))
		{
			if (abs(pCharacter->m_Pos.x - m_LastPos.x) < 96 && abs(pCharacter->m_Pos.y - m_LastPos.y) < 22)
				m_EnemyInLine = true;
			
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				
				float t = m_DispersionTick*0.1f;
				vec2 Dispersion = vec2( 11*cos(t)-6*cos(11.0f/6 * t),
										11*sin(t)-6*sin(11.0f/6 * t));
				
				// range 64 - 0, power level range 20
				Dispersion *= 3.765f - m_PowerLevel*0.188f;
				Dispersion *= 2.0f;
				
				Dispersion *= Distance*0.005f;
				
				m_PlayerDirection = pCharacter->m_Pos - m_LastPos + Dispersion;
				m_PlayerPos = pCharacter->m_Pos + Dispersion;
			}
		}
	}
	
	//if (GetWeaponFiringType(Weapon) != WFT_NONE)
	{
		if (pClosestCharacter && ClosestDistance < WeaponShootRange()*1.2f)
		{
			vec2 AttackDirection = vec2(m_PlayerDirection.x+ClosestDistance*(frandom()*0.2f-frandom()*0.2f), m_PlayerDirection.y+ClosestDistance*(frandom()*0.2f-frandom()*0.2f));

			m_Direction = AttackDirection;
			m_Hook = 0;
			
			// shooting part
			if (m_AttackTimer++ > max(0, 20-m_PowerLevel))
			{
				if (ClosestDistance < WeaponShootRange() && abs(atan2(m_Direction.x, m_Direction.y) - atan2(AttackDirection.x, AttackDirection.y)) < PI / 4.0f)
				{
					m_Attack = 1;
					
					if (m_PowerLevel < 14)
						if (frandom()*30 < 4 && WeaponShootRange() > 200 && Player()->GetCharacter()->IsGrounded())
							m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom()-m_PowerLevel*0.1f);
				}
				
				/*
				if (WeaponShootRange() < 300 && m_PowerLevel >= 10)
					m_Hook = 1;
				*/
			}
			
			return true;
		}
	}
	/*
	else
	{
		//m_TargetPos = m_Pos + m_PlayerDirection * 20;
		//GameServer()->Collision()->IntersectLine(m_Pos, m_TargetPos, 0x0, &m_TargetPos);
		return true;
	}
	*/
	return false;
}


bool CAI::ShootAtClosestMonster()
{
	CDroid *pClosestMonster = NULL;
	int ClosestDistance = 0;
	int Weapon = Player()->GetCharacter()->GetWeaponType();
	
	vec2 MonsterDir;
	
	m_EnemyInLine = false;
	
	// FIRST_BOT_ID, fix
	CDroid *apEnts[3];
	int Num = GameServer()->m_World.FindEntities(m_LastPos, 600, (CEntity**)apEnts, 3, CGameWorld::ENTTYPE_DROID);

	for (int i = 0; i < Num; ++i)
	{
		//pClosestMonster
		CDroid *pMonster = apEnts[i];

		if (pMonster->m_Health <= 0)
			continue;

		vec2 Dir;
		if (length(pMonster->m_Pos - m_LastPos) > 0.0f)
			Dir = normalize(pMonster->m_Pos - m_LastPos);

		int Distance = distance(pMonster->m_Pos, m_LastPos);
		if (Distance < AIAttackRange(Weapon) && 
			!GameServer()->Collision()->FastIntersectLine(pMonster->m_Pos + vec2(0, -20), m_LastPos))
		{
			//if (abs(pMonster->m_Pos.x - m_LastPos.x) < 96 && abs(pMonster->m_Pos.y - m_LastPos.y) < 24)
			//	m_EnemyInLine = true;
			
			if (!pClosestMonster || Distance < ClosestDistance)
			{
				pClosestMonster = pMonster;
				ClosestDistance = Distance;
				MonsterDir = pMonster->m_Pos - m_LastPos;
			}
		}
	}
	
	if (pClosestMonster && ClosestDistance < WeaponShootRange()*1.2f)
	{
		vec2 AttackDirection = vec2(MonsterDir.x+ClosestDistance*(frandom()*0.3f-frandom()*0.3f), MonsterDir.y+ClosestDistance*(frandom()*0.3f-frandom()*0.3f));

		m_Direction = AttackDirection;
		m_Hook = 0;
		
		// shooting part
		if (m_AttackTimer++ > max(0, 20-m_PowerLevel*2))
		{
			if (ClosestDistance < WeaponShootRange() && abs(atan2(m_Direction.x, m_Direction.y) - atan2(AttackDirection.x, AttackDirection.y)) < PI / 4.0f)
			{
				m_Attack = 1;
				if (frandom()*30 < 2 && WeaponShootRange() > 200)
					m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom());
			}
		}
		
		return true;
	}
	
	return false;
}


bool CAI::ShootAtClosestBuilding()
{
	CBuilding *pClosestBuilding = NULL;
	int ClosestDistance = 0;
	
	vec2 BuildingDir;
	
	CBuilding *apEnts[16];
	int Num = GameServer()->m_World.FindEntities(m_LastPos, WeaponShootRange()*1.5f, (CEntity**)apEnts, 16, CGameWorld::ENTTYPE_BUILDING);

	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pBuilding = apEnts[i];

		if (pBuilding->m_Type == BUILDING_TURRET || pBuilding->m_Type == BUILDING_TESLACOIL || pBuilding->m_Type == BUILDING_MINE1 || pBuilding->m_Type == BUILDING_MINE2 ||
			(str_comp(g_Config.m_SvGametype, "base") == 0 && (pBuilding->m_Type == BUILDING_REACTOR || pBuilding->m_Type == BUILDING_TESLACOIL || pBuilding->m_Type == BUILDING_FLAMETRAP || pBuilding->m_Type == BUILDING_LIGHTNINGWALL || pBuilding->m_Type == BUILDING_LIGHTNINGWALL2)))
		{
		
			if (GameServer()->m_pController->IsTeamplay() && pBuilding->m_Team == Player()->GetTeam())
				continue;
			
			int FlipY = 1;
			
			if ((pBuilding->m_Type == BUILDING_TURRET || pBuilding->m_Type == BUILDING_TESLACOIL) && pBuilding->m_Mirror)
				FlipY = -1;
			
			vec2 Dir;
			if (length(pBuilding->m_Pos - m_LastPos) > 0.0f)
				Dir = normalize(pBuilding->m_Pos - m_LastPos);

			int Distance = distance(pBuilding->m_Pos, m_LastPos);
			if (Distance < 800 && 
				!GameServer()->Collision()->FastIntersectLine(pBuilding->m_Pos + vec2(0, -30*FlipY), m_LastPos))
			{
				if (!pClosestBuilding || Distance < ClosestDistance)
				{
					pClosestBuilding = pBuilding;
					ClosestDistance = Distance;
					BuildingDir = (pBuilding->m_Pos+vec2(0, -30)) - m_LastPos;
				}
			}
		}
	}
	
	if (pClosestBuilding && ClosestDistance < WeaponShootRange()*1.2f)
	{
		vec2 AttackDirection = vec2(BuildingDir.x+ClosestDistance*(frandom()*0.3f-frandom()*0.3f), BuildingDir.y+ClosestDistance*(frandom()*0.3f-frandom()*0.3f));

		m_Direction = AttackDirection;
		m_Hook = 0;
		
		// shooting part
		if (m_AttackTimer++ > max(0, 20-m_PowerLevel*2))
		{
			if (ClosestDistance < WeaponShootRange() && abs(atan2(m_Direction.x, m_Direction.y) - atan2(AttackDirection.x, AttackDirection.y)) < PI / 4.0f)
			{
				m_Attack = 1;
				if (frandom()*30 < 2 && WeaponShootRange() > 200)
					m_DontMoveTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(1+frandom());
			}
		}
		
		return true;
	}
	
	return false;
}


void CAI::RandomlyStopShooting()
{
	/*
	if (Player()->GetCharacter()->m_ActiveWeapon == W_SCYTHE)
		return;
	*/
	
	if (frandom()*20 < 4 && m_Attack == 1)
	{
		m_Attack = 0;
		m_AttackTimer = max(0, 20-m_PowerLevel)/2;
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
		
		if (pPlayer->m_IsBot && GameServer()->m_pController->IsCoop())
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



bool CAI::SeekRandomWaypoint()
{
	m_TargetPos = GameServer()->Collision()->GetRandomWaypointPos();
	//m_WaypointUpdateNeeded = true;
	
	return true;
}



bool CAI::SeekClosestFriend(bool OnlyUnharmed)
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
		
		if (OnlyUnharmed)
		{
			if (pCharacter->m_DamageTakenTick > GameServer()->Server()->Tick() - GameServer()->Server()->TickSpeed()*5 ||
				pCharacter->m_AttackTick > GameServer()->Server()->Tick() - GameServer()->Server()->TickSpeed()*5)
				continue;
		}
			
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


bool CAI::SeekClosestReactor()
{
	CBuilding *pClosestBuilding = NULL;
	int ClosestDistance = 0;
	
	CBuilding *apEnts[9999];
	int Num = GameServer()->m_World.FindEntities(m_LastPos, 0, (CEntity**)apEnts, 9999, CGameWorld::ENTTYPE_BUILDING);

	for (int i = 0; i < Num; ++i)
	{
		CBuilding *pBuilding = apEnts[i];

		if (pBuilding->m_Type != BUILDING_REACTOR)
			continue;
	
		int Distance = distance(pBuilding->m_Pos, m_LastPos);
		if (!pClosestBuilding || Distance < ClosestDistance)
		{
			pClosestBuilding = pBuilding;
			ClosestDistance = Distance;
		}
	}
	
	if (pClosestBuilding)
	{
		m_TargetPos = pClosestBuilding->m_Pos;
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
		
		if (GameServer()->m_pController->IsCoop() && pCharacter->m_IsBot)
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
	
	//if (m_EnemiesInSight == 0)
	//	m_DontMoveTick = 0;
	
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
			
		if (GameServer()->m_pController->IsCoop() && pCharacter->m_IsBot)
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
	if (m_ItemUseTick < GameServer()->Server()->Tick())
	{
		m_ItemUseTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 5 * frandom();
		//m_pPlayer->SelectItem(rand()%NUM_PLAYERITEMS);
		
		if (m_pPlayer->GetCharacter())
			m_pPlayer->GetCharacter()->RandomizeInventory();
	}
	
	if (m_pPlayer->GetCharacter() && !m_pPlayer->GetCharacter()->GetWeaponType())
		m_ItemUseTick = min(0.0f + m_ItemUseTick, GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed() * 0.5f);
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
		m_Down = 0;
		m_Hook = 0;
		m_Attack = 0;
		m_InputChanged = true;
		
		return;
	}
	
	TickEvents();
	HandleEmotions();
	m_DispersionTick++;
	
	bool HoldAttack = false;
	bool Charge = AIWeaponCharge(Player()->GetCharacter()->GetWeaponType());
	
	if (m_Attack && Charge)
	{
		HoldAttack = true;
		
		if (!m_ChargeStartTick)
			m_ChargeStartTick = GameServer()->Server()->Tick() + GameServer()->Server()->TickSpeed()*(0.2f+frandom()*2.5);
	}
	
	// main logic
	if (m_NextReaction <= 0)
	{
		m_NextReaction = m_ReactionTime;
	
		m_EnemyInLine = false;
		DoBehavior();
		UseItems();
		
		if (m_AutoWeaponChange)
			Player()->GetCharacter()->AutoWeaponChange();
		
		if (m_DontMoveTick > GameServer()->Server()->Tick())
		{
			m_Move = 0;
			m_Hook = m_LastHook;
			m_Jump = 0;
		}
		
		if (m_LastJump == 1 && Player()->GetCharacter()->GetCore().m_Jetpack == 0)
			m_Jump = 0;
		

		if (m_AirJump && m_AirJump++ < 4)
		{
			m_Jump = 1;
			m_Down = 1;
		}
		
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
	
	if (HoldAttack)
		m_Attack = 1;
	
	if (m_ChargeStartTick && m_ChargeStartTick < GameServer()->Server()->Tick())
	{
		m_Attack = 0;
		m_ChargeStartTick = 0;
	}
	
	if (frandom()*100 < 0.1f)
		m_Hook = 0;
	
	m_InputChanged = false;
	if (m_InputUpdateSkip <= 0)
	{
		if (m_SendMove != m_Move || m_SendJump != m_Jump || m_SendTurbo != m_Hook || m_SendAttack != m_Attack || m_SendDown != m_Down)
		{
			m_SendMove = m_Move;
			m_SendJump = m_Jump;
			m_SendTurbo = m_Hook;
			m_SendAttack = m_Attack;
			m_SendDown = m_Down;
			m_InputUpdateSkip = 6;
			m_InputChanged = true;
		}
	}
	else m_InputUpdateSkip--;
			m_InputChanged = true;

	m_DisplayDirection.x += (m_Direction.x - m_DisplayDirection.x) / max(1.0f, 14.0f - m_PowerLevel*0.75f);
	m_DisplayDirection.y += (m_Direction.y - m_DisplayDirection.y) / max(1.0f, 14.0f - m_PowerLevel*0.75f);
}



