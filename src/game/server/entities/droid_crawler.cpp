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
	m_Health = 500;
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
	m_JumpTick = 0;
	m_JumpForce = 0.0f;
	m_AttackCount = 0;
}



void CCrawler::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Weapon)
{
	if (!Dmg)
		return;
	
	if (g_Config.m_SvOneHitKill)
		Dmg = 1000;

	vec2 DmgPos = m_Pos + m_Center;
	
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
	
	if (length(m_Vel) > 20.0f)
		m_Vel = normalize(m_Vel)*20.0f;
	
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
	}

	m_DamageTakenTick = Server()->Tick();
}





void CCrawler::MoveDead()
{
	
	
}


void CCrawler::Move()
{
	
}

void CCrawler::Tick()
{
	//m_Target = (m_NewTarget+m_Target)/3;
	
	//Move();
	
	m_Vel.y += 0.8f;
	m_Vel *= 0.99f;
	
	GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(60.0f, 60.0f), 0, false);
	
	const int OffY = m_JumpTick ? 50 : 80;
	
	vec2 To = m_Pos + vec2(0, OffY);
	
	if (m_Health <= 0)
	{
		float OldVelY = m_Vel.y;
		
		if (Server()->Tick() > m_DamageTakenTick+30 || OldVelY > 12.0f)
		{
			if (Server()->Tick() > m_DamageTakenTick+90 || abs(m_Vel.y) < 0.2f)
			{
				GameServer()->CreateExplosion(m_Pos+m_Center, TEAM_NEUTRAL, GetDroidWeapon(m_Type, true));
				m_DeathTick = Server()->Tick();
				
				for (int i = 0; i < 3; i++)
				{
					if (frandom() < 0.3f)
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_AMMO, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
					else if (frandom() < 0.3f)
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_ARMOR, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
					else
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_KIT, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
				}
				
				if (frandom() < 0.25f)
					GameServer()->m_pController->DropWeapon(m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_UPGRADE)));
				else if (frandom() < 0.15f)
					GameServer()->m_pController->DropWeapon(m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_RESPAWNER)));
				
				GameServer()->m_World.DestroyEntity(this);
				return;
			}
		}
		
		m_Status = DROIDSTATUS_TERMINATED;
		return;
	}
	
	if (GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To))
	{
		if (abs(m_Vel.x) < 1.0f && frandom() < 0.05f)
			m_Move = (frandom() < 0.5f) ? -1 : 1;
	
		float VelY = m_Pos.y-(To.y-OffY)*0.0002f;
		
		if (VelY > 0.0f && !m_JumpTick)
		{
			m_Vel.y -= min(1.4f, VelY);
			m_Vel.y *= 0.99f;
		}
		
		m_Vel.x *= 0.8f;
		
		if (m_Anim == DROIDANIM_ATTACK)
		{
			if (abs(m_Vel.x) < 15.0f)
				m_Vel.x += m_Move * 1.8f;
		}
		else if (abs(m_Vel.x) < 8.0f)
			m_Vel.x += m_Move * 0.9f;
		
		if (!m_JumpTick && (frandom() < 0.01f || (abs(m_Vel.x) < 0.15f && frandom() < 0.4f) || (abs(m_Target.x) > 300 && frandom() < 0.05f)))
		{
			m_JumpTick = Server()->Tick() + Server()->TickSpeed()*0.25f;
		}
		
		if (m_JumpTick && m_JumpTick < Server()->Tick())
		//if (m_JumpTick && abs(m_Pos.y - (To.y-5)) < 10.0f)
		{
			//m_Vel.y = -16;
			if (abs(m_Target.x) > 300)
				m_JumpForce = -5.0f;
			else
				m_JumpForce = -7.0f-frandom()*3.0f;
			
			//if (abs(m_Vel.x) < 10.0f)
			//	m_Vel.x *= 1.25f;
			//m_JumpTick = 0;
		}
		
		m_Vel.y += m_JumpForce;
		m_Vel.x -= m_JumpForce*m_Move*0.25f;
		
		if (m_JumpForce < -0.1f)
			m_Anim = DROIDANIM_JUMPATTACK;
		else if (abs(m_Target.x) > 20 && abs(m_Target.x) < 400)
			m_Anim = DROIDANIM_ATTACK;
		else
			m_Anim = DROIDANIM_IDLE;
	}
	else if (m_JumpTick && m_JumpTick < Server()->Tick())
		m_JumpTick = 0;
	else if (frandom() < 0.02f)
		m_Vel.x += (frandom()-frandom()) * 2.0f;
	
	m_Vel.x -= m_JumpForce*m_Move*0.1f;
	m_JumpForce *= 0.9f;
	

	
	m_Dir = m_Move;
	
	if (Target())
	{
		
	}
	else
	{
		FindTarget();
		m_Anim = DROIDANIM_IDLE;
	}
	
	// attack
	if (m_Anim == DROIDANIM_JUMPATTACK || m_Anim == DROIDANIM_ATTACK)
	{
		if (m_AttackCount++ > 3)
		{
			m_AttackCount = 0;
			vec2 ProjPos = To+vec2(m_Move*54.0f, -20.0f);
			GameServer()->CreateProjectile(NEUTRAL_BASE, GetDroidWeapon(m_Type), 0, ProjPos, normalize(m_Pos - ProjPos), m_Pos);
		}
	}
	
	if(Server()->Tick() > m_DamageTakenTick+15)
		m_Status = DROIDSTATUS_IDLE;
	
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
		
		//vec2 TurretPos = m_Pos+vec2(m_Dir * 16, m_Center.y-20);
		//float Angle = (m_Angle + 90) / (180/pi);

		//vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		
		m_Vel += normalize(m_Target)*7.0f;
		
		GameServer()->CreateSound(m_Pos, SOUND_STAR_FIRE);
		
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
	vec2 TurretPos = m_Pos;
	
	if (m_TargetIndex >= 0 && m_TargetIndex < MAX_CLIENTS)
	{
		CCharacter *pCharacter = GameServer()->GetPlayerChar(m_TargetIndex);
		if (!pCharacter)
			return false;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			return false;
		
		if (m_Move == -1 && pCharacter->m_Pos.x > m_Pos.x && frandom() < 0.15f)
		{
			m_Move = 1;
		}
		if (m_Move == 1 && pCharacter->m_Pos.x < m_Pos.x && frandom() < 0.15f)
		{
			m_Move = -1;
		}

		int Distance = distance(pCharacter->m_Pos, m_Pos);
		if (Distance < 700 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos+vec2(0, -24), m_Pos))
		{
			m_Target = pCharacter->m_Pos - m_Pos;
			return true;
		}
		else
		{
			m_Target = vec2(0, 0);
			return false;
		}
	}
	
	return false;
}


bool CCrawler::FindTarget()
{
	m_TargetIndex = -1;
	CCharacter *pClosestCharacter = NULL;
	int ClosestDistance = 0;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pCharacter = GameServer()->GetPlayerChar(i);
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive() || pCharacter->Invisible())
			continue;
		
		if (GameServer()->m_pController->IsCoop() && pCharacter->m_IsBot)
			continue;
			
		if (abs(m_Pos.x - pCharacter->m_Pos.x) < 600 && abs(m_Pos.y - pCharacter->m_Pos.y) < 220)
		{
			if (!GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos+vec2(0, -24), m_Pos))
			{
				int Distance = distance(pCharacter->m_Pos, m_Pos);
				if (!pClosestCharacter || Distance < ClosestDistance)
				{
					pClosestCharacter = pCharacter;
					ClosestDistance = Distance;
					m_TargetIndex = i;
				}
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