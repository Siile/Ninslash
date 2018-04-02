#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "staticlaser.h"
#include "droid_crawler.h"


CCrawler::CCrawler(CGameWorld *pGameWorld, vec2 Pos)
: CDroid(pGameWorld, Pos, DROIDTYPE_CRAWLER)
{
	m_ProximityRadius = DroidPhysSize;

	m_StartPos = Pos;
	
	Reset();
	GameWorld()->InsertEntity(this);

}

void CCrawler::Reset()
{
	m_Center = vec2(0, 0);
	m_Health = 140;
	m_Pos = m_StartPos;
	m_Status = DROIDSTATUS_IDLE;
	m_Dir = -1;
	m_DeathTick = 0;
	SetState(0);
	m_TargetIndex = -1;
	m_ReloadTimer = 0;
	m_AttackTick = 0;
	m_TargetTimer = 0;
	m_Target = vec2(0, 0);
	m_NewTarget = vec2(0, 0);
	m_MoveTarget = m_Pos;
	m_Vel = vec2(0, 0);
	m_FlyTargetTick = 0;
	m_Mode = 0;
	m_ProximityRadius = CrawlerPhysSize;
	m_FireDelay = 0;
	m_FireCount = 0;
	m_AttackTimer = 0;
	m_AngleTimer = frandom()*pi;
	m_DamageTakenTick = 0;
	m_Move = 0;
}



void CCrawler::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Type)
{
	// skip everything while spawning
	//if (m_aStatus[STATUS_SPAWNING] > 0.0f)
	//	return false;
	
	if (g_Config.m_SvOneHitKill)
		Dmg = 1000;

	vec2 DmgPos = m_Pos + m_Center;
	
	if (m_TargetIndex < 0 && frandom() < 0.3f)
	{
		SetState(CCrawler::TURN);
		
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

		m_Status = DROIDSTATUS_HURT;
	}
	else
	if (Type == DAMAGETYPE_ELECTRIC)
	{
		//GameServer()->SendEffect(m_pPlayer->GetCID(), EFFECT_ELECTRODAMAGE);
		m_Status = DROIDSTATUS_ELECTRIC;
	}
	else if (Type == DAMAGETYPE_FLAME)
		m_Status = DROIDSTATUS_HURT;
	
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

		m_DamageTakenTick = Server()->Tick();
		
		/*
		GameServer()->CreateExplosion(m_Pos+m_Center, NEUTRAL_BASE, WEAPON_HAMMER, 0, false, false);
		GameServer()->CreateSound(m_Pos+m_Center, SOUND_GRENADE_EXPLODE);
		m_DeathTick = Server()->Tick();
		
		// random pickup drop
		if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_AMMO, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		else if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_HEALTH, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);
		else if (frandom()*10 < 4)
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_ARMOR, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);			
		else
			GameServer()->m_pController->DropPickup(m_Pos + vec2(0, -42), POWERUP_KIT, Force+vec2(frandom()*6.0-frandom()*6.0, frandom()*6.0-frandom()*6.0), 0);			
		*/
		return;
	}

	m_DamageTakenTick = Server()->Tick();
}





void CCrawler::MoveDead()
{
	
	
}


void CCrawler::Move()
{
	m_Vel.y += 0.8f;
	m_Vel *= 0.97f;
	
	GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(80.0f, 80.0f), 0, false);
	
	if (!m_Move)
		m_Move = frandom() < 0.5f ? -1 : 1;
	
	m_Vel.x += m_Move * 1.0f;
}

void CCrawler::Tick()
{

	if (!Target())
		FindTarget();
		
	m_Target = (m_NewTarget+m_Target)/3;
	
	Move();
	
	/*
	vec2 To = m_Pos+vec2(frandom()-frandom(), frandom()-frandom())*500;
		
	if (m_Health <= 0)
	{
		float OldVelY = m_Vel.y;
		m_Vel.y += 0.8f;
		//m_Vel *= 0.97f;
		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(80.0f, 100.0f), 0, false);
		//return;
		
		if (Server()->Tick() > m_DamageTakenTick+30 || OldVelY > 12.0f)
		{
			if (abs(m_Vel.y) < 0.2f)
			{
				GameServer()->CreateExplosion(m_Pos+m_Center, NEUTRAL_BASE, DEATHTYPE_DROID_WALKER, 0, false, false);
				GameServer()->CreateSound(m_Pos+m_Center, SOUND_GRENADE_EXPLODE);
				m_DeathTick = Server()->Tick();
				
				for (int i = 0; i < 3; i++)
				{
					if (frandom() < 0.4f)
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_AMMO, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
					else
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_HEALTH, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
				}
				
				GameServer()->m_World.DestroyEntity(this);
				return;
			}
		}
		
		m_Status = DROIDSTATUS_TERMINATED;
	}
	else
	{
		m_AngleTimer += 0.025f;

		//To += vec2(sin(m_AngleTimer), cos(m_AngleTimer))*20.0f;
		
		if (GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To))
		{
			To = m_Pos+vec2(frandom()-frandom(), frandom()-frandom())*30;
			To.x += vec2(sin(m_AngleTimer), cos(m_AngleTimer)).y*40.0f;
			To.y += vec2(sin(m_AngleTimer), cos(m_AngleTimer)).y*20.0f;
			GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To);
		}
		
		m_MoveTarget += (To-m_MoveTarget) / 15.0f;
		
		if (abs(length(m_MoveTarget - m_Pos)) > 8.0f)
			m_Vel += normalize(m_MoveTarget - m_Pos) * 0.5f;
		
		m_Vel.y += 0.25f;
		
		m_Vel *= 0.97f;
		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(96.0f, 128.0f), 0, false);
	
		//bool WillFire = false;
		
		if (!Target())
			FindTarget();
		else
		{
			// move towards target
			To = (m_Pos-m_NewTarget);
			if (length(m_NewTarget) > 500.0f)
			{
				m_MoveTarget += (vec2(m_Pos.x, To.y)-m_MoveTarget) / 10.0f;
				m_MoveTarget += (vec2(To.x, m_Pos.y)-m_MoveTarget) / 10.0f;
			}
			
			//WillFire = true;
		}
		
		m_Target = (m_NewTarget+m_Target)/3;
		

		// body angle
		m_Target.x = vec2(sin(m_AngleTimer), cos(m_AngleTimer)).x;
		m_Target.y = -1;
		*/
		/*
		if (WillFire)
		{
			if (--m_FireDelay < 0)
			{
				m_FireDelay = 0;
				Fire();
			}
		}
		*/

		/*
		if(Server()->Tick() > m_DamageTakenTick+15)
			m_Status = DROIDSTATUS_IDLE;
	}
	*/
}



void CCrawler::Fire()
{
	if (m_ReloadTimer-- < 0)
	{
		m_FireCount++;
		m_ReloadTimer = 700 * Server()->TickSpeed() / 1000;
		
		vec2 TurretPos = m_Pos+vec2(m_Dir * 16, m_Center.y-20);
		//float Angle = (m_Angle + 90) / (180/pi);

		//vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		
		m_Vel += normalize(m_Target)*7.0f;
		
		GameServer()->CreateSound(m_Pos, aCustomWeapon[W_DROID_STAR].m_Sound);
		
		GameServer()->CreateProjectile(NEUTRAL_BASE, W_DROID_STAR, 0, TurretPos+normalize(m_Target*-1)*30.0f, m_Target*-1);
		GameServer()->CreateProjectile(NEUTRAL_BASE, W_DROID_STAR, 0, TurretPos+normalize(m_Target*-1)*30.0f+vec2(-m_Dir * 64, 0), m_Target*-1);
		
		//GameServer()->CreateProjectile(NEUTRAL_BASE, W_DROID_WALKER, 0, TurretPos+normalize(m_Target*-1)*32.0f, m_Target*-1);
		//GameServer()->CreateProjectile(NEUTRAL_BASE, W_DROID_WALKER, 0, TurretPos+normalize(m_Target*-1)*32.0f+vec2(m_Dir * 44, -28), m_Target*-1);
		
		m_AttackTick = Server()->Tick();
	}
	
	if (m_FireCount > 4)
	{
		m_FireCount = 0;
		m_FireDelay = 20;
	}
}



bool CCrawler::Target()
{
	vec2 TurretPos = m_Pos+m_Center;
	
	if (m_TargetIndex >= 0 && m_TargetIndex < MAX_CLIENTS)
	{
		CCharacter *pCharacter = GameServer()->GetPlayerChar(m_TargetIndex);
		if (!pCharacter)
			return false;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			return false;
		
		if ((m_Dir < 0 && pCharacter->m_Pos.x > m_Pos.x) || 
			(m_Dir > 0 && pCharacter->m_Pos.x < m_Pos.x))
		{
			m_Dir *= -1;
			m_State = CCrawler::IDLE;
			m_StateChangeTick = Server()->Tick() + Server()->TickSpeed() * (1 + frandom());
		}

		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 700 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos+vec2(0, -24), TurretPos))
		{
			vec2 r = vec2(sin(Server()->Tick()*0.075f), cos(Server()->Tick()*0.075f))*Distance*0.1f;
			m_NewTarget = r + TurretPos - ((pCharacter->m_Pos+vec2(0, -24)) + pCharacter->GetCore().m_Vel * 2.0f);
			return true;
		}
		else
			return false;
	}
	
	return false;
}


bool CCrawler::FindTarget()
{
	m_TargetIndex = -1;
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	vec2 TurretPos = m_Pos+vec2(0, -67);
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pCharacter = GameServer()->GetPlayerChar(i);
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			continue;
		
		if (GameServer()->m_pController->IsCoop() && pCharacter->m_IsBot)
			continue;
			
		//if ((m_Dir < 0 && pCharacter->m_Pos.x > m_Pos.x) || 
		//	(m_Dir > 0 && pCharacter->m_Pos.x < m_Pos.x))
		//	continue;
			
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


void CCrawler::TickPaused()
{
	//if(m_SpawnTick != -1)
	//	++m_SpawnTick;
}