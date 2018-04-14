#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "droid_walker.h"


CWalker::CWalker(CGameWorld *pGameWorld, vec2 Pos)
: CDroid(pGameWorld, Pos, DROIDTYPE_WALKER)
{
	m_ProximityRadius = DroidPhysSize;

	m_StartPos = Pos;
	
	Reset();
	GameWorld()->InsertEntity(this);

}

void CWalker::Reset()
{
	m_Center = vec2(0, -50);
	m_Health = 100;
	m_Pos = m_StartPos;
	m_Status = DROIDSTATUS_IDLE;
	m_Dir = -1;
	m_DeathTick = 0;
	SetState(CWalker::IDLE);
	m_TargetIndex = -1;
	m_ReloadTimer = 0;
	m_AttackTick = 0;
	m_TargetTimer = 0;
	m_Target = vec2(0, 0);
	m_NewTarget = vec2(0, 0);
	m_Vel = vec2(0, 0);
	m_FlyTargetTick = 0;
	m_Mode = CWalker::WALKER;
	m_ProximityRadius = DroidPhysSize;
	m_FireDelay = 0;
	m_FireCount = 0;
	m_AttackTimer = 0;
}



void CWalker::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Weapon)
{
	// skip everything while spawning
	//if (m_aStatus[STATUS_SPAWNING] > 0.0f)
	//	return false;
	
	if (g_Config.m_SvOneHitKill)
		Dmg = 1000;

	vec2 DmgPos = m_Pos + m_Center;
	
	if (m_TargetIndex < 0 && frandom() < 0.3f)
	{
		SetState(CWalker::TURN);
		
		if (m_AttackTimer > 0)
			m_AttackTimer = 20;
		else
			m_AttackTimer--;
	}
	
	// create damage indicator
	if (WeaponElectroAmount(Weapon) > 0.0f)
		m_Status = DROIDSTATUS_ELECTRIC;
	else if (WeaponFlameAmount(Weapon) > 0.0f)
		m_Status = DROIDSTATUS_HURT;
	else
	{
		if (Pos.x != 0 && Pos.y != 0)
			DmgPos = Pos;
		
		GameServer()->CreateBuildingHit(DmgPos);
		m_Status = DROIDSTATUS_HURT;
	}
	
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

		GameServer()->CreateExplosion(m_Pos+m_Center, TEAM_NEUTRAL, GetDroidWeapon(m_Type, true));
		m_DeathTick = Server()->Tick();
		
		// random pickup drop
		for (int i = 0; i < 3; i++)
		{
			if (frandom() < 0.3f)
				GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_AMMO, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
			else if (frandom() < 0.3f)
				GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_HEALTH, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
			else if (frandom() < 0.3f)
				GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_ARMOR, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
			else
				GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_KIT, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);			
		}
		
		m_DeathTick = Server()->Tick();
		GameServer()->m_World.DestroyEntity(this);
				
		return;
	}

	m_DamageTakenTick = Server()->Tick();
}



void CWalker::Tick()
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
	
	if (GameServer()->Collision()->IsInFluid(m_Pos.x, m_Pos.y))
		TakeDamage(vec2(0, 0), 2, -1, vec2(0, 0), DAMAGETYPE_FLUID);
	
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
				if (m_Mode == CWalker::WALKER)
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
		if (m_Mode == CWalker::WALKER && m_FlyTargetTick > 0 && m_FlyTargetTick < Server()->Tick())
		{
			m_Mode = CWalker::DRONE;
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
	if (m_State == CWalker::IDLE && m_State == m_NextState && m_StateChangeTick < Server()->Tick())
		SetState(CWalker::MOVE);
	
	
	m_Anim = 0;
	
	// fly
	if (m_Mode == CWalker::DRONE)
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
	if (m_State == CWalker::MOVE && m_Mode == CWalker::WALKER)
	{
		m_Anim = 1;
		
		float Speed = 6.0f;
		
		if (m_AttackTimer-- < -40)
		{
			m_AttackTimer = 10+rand()%20;
			m_State = CWalker::IDLE;
			m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (2 + frandom());
		}
		else if (m_Dir == -1)
		{
			m_Pos.x -= Speed;
			
			// wall
			if (GameServer()->Collision()->IsTileSolid(m_Pos.x-46, m_Pos.y-8))
			{
				SetState(CWalker::TURN);
			}
			// pit
			else if (!GameServer()->Collision()->IsTileSolid(m_Pos.x-55, m_Pos.y+18))
			{
				m_State = CWalker::IDLE;
				m_NextState = CWalker::TURN;
				m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (1 + frandom());
			}
		}
		else if (m_Dir == 1)
		{
			m_Pos.x += Speed;
			
			// wall
			if (GameServer()->Collision()->IsTileSolid(m_Pos.x+46, m_Pos.y-8))
			{
				SetState(CWalker::TURN);
			}
			// pit
			else if (!GameServer()->Collision()->IsTileSolid(m_Pos.x+55, m_Pos.y+18))
			{
				m_State = CWalker::IDLE;
				m_NextState = CWalker::TURN;
				m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (1 + frandom());
			}
		}
	}
	
	// turn
	if (m_State == CWalker::TURN)
	{
		m_Dir *= -1;
		m_State = CWalker::MOVE;
		m_NextState = CWalker::MOVE;
	}
	
	
	if(Server()->Tick() > m_DamageTakenTick+15)
		m_Status = DROIDSTATUS_IDLE;
	
	GameServer()->m_World.m_Core.AddMonster(m_Pos);
}



void CWalker::Fire()
{
	if (m_ReloadTimer-- < 0)
	{
		m_FireCount++;
		m_ReloadTimer = 80 * Server()->TickSpeed() / 1000;
		
		vec2 TurretPos = m_Pos+vec2(m_Dir * 16, m_Center.y);
		//float Angle = (m_Angle + 90) / (180/pi);

		//vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		GameServer()->CreateSound(m_Pos, SOUND_WALKER_FIRE);
		
		GameServer()->CreateProjectile(NEUTRAL_BASE, GetDroidWeapon(m_Type), 0, TurretPos+normalize(m_Target*-1)*32.0f, m_Target*-1);
		GameServer()->CreateProjectile(NEUTRAL_BASE, GetDroidWeapon(m_Type), 0, TurretPos+normalize(m_Target*-1)*32.0f+vec2(m_Dir * 4, -8), m_Target*-1);
		
		m_AttackTick = Server()->Tick();
		
		if (m_Mode == CWalker::DRONE)
			m_Vel += normalize(m_Target)*4.0f;
	}
	
	if (m_FireCount > 4)
	{
		m_FireCount = 0;
		m_FireDelay = 20;
	}
}



bool CWalker::Target()
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
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			return false;
		
		if ((m_Dir < 0 && pCharacter->m_Pos.x > m_Pos.x) || 
			(m_Dir > 0 && pCharacter->m_Pos.x < m_Pos.x))
		{
			m_Dir *= -1;
			m_State = CWalker::IDLE;
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


bool CWalker::FindTarget()
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
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
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


void CWalker::TickPaused()
{
	//if(m_SpawnTick != -1)
	//	++m_SpawnTick;
}