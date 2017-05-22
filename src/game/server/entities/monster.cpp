#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "monster.h"


CMonster::CMonster(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_MONSTER)
{
	m_ProximityRadius = MonsterPhysSize;

	m_StartPos = Pos;
	
	Reset();
	GameWorld()->InsertEntity(this);

}

void CMonster::Reset()
{
	m_Center = vec2(0, -50);
	m_Health = 100;
	m_Pos = m_StartPos;
	m_Status = MONSTERSTATUS_IDLE;
	m_Dir = -1;
	m_DeathTick = 0;
	SetState(CMonster::IDLE);
	m_TargetIndex = -1;
	m_ReloadTimer = 0;
	m_AttackTick = 0;
	m_TargetTimer = 0;
	m_Target = vec2(0, 0);
	m_NewTarget = vec2(0, 0);
	m_Vel = vec2(0, 0);
	m_FlyTargetTick = 0;
	m_Mode = CMonster::WALKER;
	m_ProximityRadius = MonsterPhysSize;
	m_FireDelay = 0;
	m_FireCount = 0;
	m_AttackTimer = 0;
}



void CMonster::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Type)
{
	// skip everything while spawning
	//if (m_aStatus[STATUS_SPAWNING] > 0.0f)
	//	return false;
	
	if (g_Config.m_SvOneHitKill)
		Dmg = 1000;

	vec2 DmgPos = m_Pos + m_Center;
	
	if (m_TargetIndex < 0 && frandom() < 0.3f)
	{
		SetState(CMonster::TURN);
		
		if (m_AttackTimer > 0)
			m_AttackTimer = 20;
		else
			m_AttackTimer--;
	}
	
	// create healthmod indicator
	if (Type == DAMAGETYPE_NORMAL)
	{
		if (Pos.x != 0 && Pos.y != 0)
			DmgPos = Pos;
		//	DmgPos = (DmgPos + Pos) / 2.0f;
		
		GameServer()->CreateBuildingHit(DmgPos);

		m_Status = MONSTERSTATUS_HURT;
	}
	else
	if (Type == DAMAGETYPE_ELECTRIC)
	{
		//GameServer()->SendEffect(m_pPlayer->GetCID(), EFFECT_ELECTRODAMAGE);
		m_Status = MONSTERSTATUS_ELECTRIC;
	}
	else if (Type == DAMAGETYPE_FLAME)
		m_Status = MONSTERSTATUS_HURT;
	
	GameServer()->CreateDamageInd(DmgPos, GetAngle(-Force), -Dmg, -1);
	
	m_Vel += Force*0.75f;
	
	m_Health -= Dmg;
	
	
	// check for death
	if(m_Health <= 0)
	{
		// set attacker's face to happy (taunt!)
		if (From >= 0 && GameServer()->m_apPlayers[From])
		{
			CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
			if (pChr)
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
		}

		GameServer()->CreateExplosion(m_Pos+m_Center, NEUTRAL_BASE, WEAPON_HAMMER, 0, false, false);
		GameServer()->CreateSound(m_Pos+m_Center, SOUND_GRENADE_EXPLODE);
		m_DeathTick = Server()->Tick();
		
		// random pickup drop
		if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_ARMOR, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		else if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_HEALTH, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		else if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_MINE, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);			
		else
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_KIT, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);			
		
		return;
	}

	m_DamageTakenTick = Server()->Tick();
}



void CMonster::Tick()
{
	if (m_Health <= 0)
	{
		// respawn
		/*
		if (m_DeathTick < Server()->Tick() - Server()->TickSpeed() * 30)
		{
			Reset();
			GameServer()->CreateEffect(FX_MONSTERSPAWN, m_Pos);
		}
		*/
		return;
	}
	
	
	
	bool WillFire = false;
	
	
	if (m_AttackTimer > 0 && Target())
	{
		WillFire = true;
		if (m_TargetTimer-- < 0)
		{
			FindTarget();
			m_TargetTimer = 30 + frandom()*30;
		}
	}
	else
	// idle
	{
		if (!FindTarget())
		{
			if (m_TargetTimer-- < 0)
			{
				m_TargetTimer = 50 + frandom()*100;
				//m_Target = vec2((frandom()-frandom())*150, frandom()*50-frandom()*100);
				//m_Target = m_OriginalDirection;
				if (m_Mode == CMonster::WALKER)
					m_NewTarget = vec2(m_Dir * 50, 0);
			}
		}
	}
	
	//m_Target = (m_NewTarget+m_Target)/2;
	
	if (WillFire)
	{
		m_Target += (m_NewTarget-m_Target) / 6.0f;
		if (--m_FireDelay < 0)
		{
			m_FireDelay = 0;
			Fire();
		}
		
		m_FlyTargetTick = Server()->Tick() + Server()->TickSpeed() * 1.5f;
	}
	else
	{
		m_FireDelay = min(m_FireDelay+2, 20);
		
		m_Target += (vec2(m_Dir * 50, 0)-m_Target) / 6.0f;
		
		// takeoff
		/*
		if (m_Mode == CMonster::WALKER && m_FlyTargetTick > 0 && m_FlyTargetTick < Server()->Tick())
		{
			m_Mode = CMonster::DRONE;
			m_Pos.y -= 54;
			m_Vel = vec2(0, -8);
			m_Center = vec2(0, 0);
			GameServer()->CreateEffect(FX_TAKEOFF, m_Pos+vec2(0, -4));
			m_ProximityRadius = MonsterPhysSize*0.7f;
		}
		*/
	}
	
	
	// state change
	if (m_State != m_NextState && Server()->Tick() > m_StateChangeTick)
		SetState(m_NextState);
	
	// get moving
	if (m_State == CMonster::IDLE && m_State == m_NextState && m_StateChangeTick < Server()->Tick())
		SetState(CMonster::MOVE);
	
	
	m_Anim = 0;
	
	// fly
	if (m_Mode == CMonster::DRONE)
	{
		m_Anim = 3;

		vec2 TargetPos = vec2(m_NewTarget.x, m_NewTarget.y);
		
		if (WillFire)
		{
			TargetPos = vec2(m_NewTarget.x + 300*m_Dir, m_NewTarget.y + 100);
			m_FlyTargetTick = Server()->Tick() + Server()->TickSpeed() * 3.0f;
		}
		else
		{
			if (m_FlyTargetTick < Server()->Tick())
			{
				for (int i = 0; i < 10; i++)
				{
					m_NewTarget = vec2(frandom()-frandom(), frandom()-frandom()) * 300;
					
					if (!GameServer()->Collision()->FastIntersectLine(m_Pos, m_Pos-m_NewTarget))
						break;
				}
				
				m_FlyTargetTick = Server()->Tick() + Server()->TickSpeed() * 3.0f;
			}
			
			m_Dir = m_Vel.x < 0 ? -1 : 1;
			
			if (abs(m_Vel.x) + abs(m_Vel.y) < 1.0f)
				m_FlyTargetTick = Server()->Tick();
		}
		
		m_Vel -= normalize(TargetPos) * 0.75f;
		m_Vel *= 0.9f;
		
		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(78.0f, 64.0f), 0, false);
	}
	
	
	// move
	if (m_State == CMonster::MOVE && m_Mode == CMonster::WALKER)
	{
		m_Anim = 1;
		
		float Speed = 6.0f;
		
		if (m_AttackTimer-- < -40)
		{
			m_AttackTimer = 10+rand()%20;
			m_State = CMonster::IDLE;
			m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (2 + frandom());
		}
		else if (m_Dir == -1)
		{
			m_Pos.x -= Speed;
			
			// wall
			if (GameServer()->Collision()->IsTileSolid(m_Pos.x-46, m_Pos.y-8))
			{
				SetState(CMonster::TURN);
			}
			// pit
			else if (!GameServer()->Collision()->IsTileSolid(m_Pos.x-55, m_Pos.y+18))
			{
				m_State = CMonster::IDLE;
				m_NextState = CMonster::TURN;
				m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (1 + frandom());
			}
		}
		else if (m_Dir == 1)
		{
			m_Pos.x += Speed;
			
			// wall
			if (GameServer()->Collision()->IsTileSolid(m_Pos.x+46, m_Pos.y-8))
			{
				SetState(CMonster::TURN);
			}
			// pit
			else if (!GameServer()->Collision()->IsTileSolid(m_Pos.x+55, m_Pos.y+18))
			{
				m_State = CMonster::IDLE;
				m_NextState = CMonster::TURN;
				m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (1 + frandom());
			}
		}
	}
	
	// turn
	if (m_State == CMonster::TURN)
	{
		m_Dir *= -1;
		m_State = CMonster::MOVE;
		m_NextState = CMonster::MOVE;
	}
	
	
	if(Server()->Tick() > m_DamageTakenTick+15)
		m_Status = MONSTERSTATUS_IDLE;
	
	GameServer()->m_World.m_Core.AddMonster(m_Pos);
}



void CMonster::Fire()
{
	if (m_ReloadTimer-- < 0)
	{
		m_FireCount++;
		m_ReloadTimer = 80 * Server()->TickSpeed() / 1000;
		
		vec2 TurretPos = m_Pos+vec2(m_Dir * 16, m_Center.y);
		//float Angle = (m_Angle + 90) / (180/pi);

		//vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		GameServer()->CreateSound(m_Pos, aCustomWeapon[W_WALKER].m_Sound);
		
		GameServer()->CreateProjectile(NEUTRAL_BASE, W_WALKER, 0, TurretPos+normalize(m_Target*-1)*32.0f, m_Target*-1);
		GameServer()->CreateProjectile(NEUTRAL_BASE, W_WALKER, 0, TurretPos+normalize(m_Target*-1)*32.0f+vec2(m_Dir * 4, -8), m_Target*-1);
		
		m_AttackTick = Server()->Tick();
		
		if (m_Mode == CMonster::DRONE)
			m_Vel += normalize(m_Target)*4.0f;
	}
	
	if (m_FireCount > 4)
	{
		m_FireCount = 0;
		m_FireDelay = 20;
	}
}



bool CMonster::Target()
{
	vec2 TurretPos = m_Pos+m_Center;
	
	if (m_TargetIndex >= 0 && m_TargetIndex < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_TargetIndex];
		if(!pPlayer)
			return false;
			
		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			return false;
		
		if (!pCharacter->IsAlive())
			return false;
		
		if ((m_Dir < 0 && pCharacter->m_Pos.x > m_Pos.x) || 
			(m_Dir > 0 && pCharacter->m_Pos.x < m_Pos.x))
		{
			m_Dir *= -1;
			m_State = CMonster::IDLE;
			m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (1 + frandom());
		}

		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 700 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos+vec2(0, -24), TurretPos))
		{
			vec2 r = vec2(sin(Server()->Tick()*0.075f), cos(Server()->Tick()*0.075f))*Distance*0.3f;
			m_NewTarget = r + TurretPos - ((pCharacter->m_Pos+vec2(0, -24)) + pCharacter->GetCore().m_Vel * 2.0f);
			return true;
		}
		else
			return false;
	}
	
	return false;
}


bool CMonster::FindTarget()
{
	m_TargetIndex = -1;
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	vec2 TurretPos = m_Pos+vec2(0, -67);
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		//if (pPlayer->GetTeam() == m_Team && GameServer()->m_pController->IsTeamplay())
		//	continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
		
		if (GameServer()->m_pController->IsCoop() && pCharacter->m_IsBot)
			continue;
			
		if ((m_Dir < 0 && pCharacter->m_Pos.x > m_Pos.x) || 
			(m_Dir > 0 && pCharacter->m_Pos.x < m_Pos.x))
			continue;
			
		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 800 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos+vec2(0, -24), TurretPos))
		{
			if (!pClosestCharacter || Distance < ClosestDistance)
			{
				pClosestCharacter = pCharacter;
				ClosestDistance = Distance;
				m_TargetIndex = i;
			}
		}
	}
	
	if (pClosestCharacter)
		return true;
	
	return false;
}


void CMonster::SetState(int State)
{
	m_State = State;
	m_NextState = State;
	m_StateChangeTick = Server()->Tick();
	
}

void CMonster::TickPaused()
{
	//if(m_SpawnTick != -1)
	//	++m_SpawnTick;
}

void CMonster::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) || m_Health <= 0)
		return;

	CNetObj_Monster *pP = static_cast<CNetObj_Monster *>(Server()->SnapNewItem(NETOBJTYPE_MONSTER, m_ID, sizeof(CNetObj_Monster)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Status = m_Status;
	pP->m_AttackTick = m_AttackTick;
	pP->m_Anim = m_Anim;
	pP->m_Dir = m_Dir;
	pP->m_Angle = GetAngle(vec2(abs(m_Target.x), m_Target.y*-1)) * (180/pi);
}
