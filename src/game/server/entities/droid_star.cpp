#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "staticlaser.h"
#include "droid_star.h"


CStar::CStar(CGameWorld *pGameWorld, vec2 Pos)
: CDroid(pGameWorld, Pos, DROIDTYPE_STAR)
{
	m_ProximityRadius = DroidPhysSize;

	m_StartPos = Pos;
	
	Reset();
	GameWorld()->InsertEntity(this);

}

void CStar::Reset()
{
	m_Center = vec2(0, 0);
	m_Health = 240;
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
	m_ProximityRadius = DroidPhysSize;
	m_FireDelay = 0;
	m_FireCount = 0;
	m_AttackTimer = 0;
	m_AngleTimer = frandom()*pi;
	m_DamageTakenTick = 0;
}



void CStar::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Weapon)
{
	// skip everything while spawning
	//if (m_aStatus[STATUS_SPAWNING] > 0.0f)
	//	return false;
	
	if (!Dmg)
		return;
	
	if (g_Config.m_SvOneHitKill)
		Dmg = 1000;

	vec2 DmgPos = m_Pos + m_Center;
	
	if (m_TargetIndex < 0 && frandom() < 0.3f)
	{
		SetState(CStar::TURN);
		
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



void CStar::Tick()
{
	vec2 To = m_Pos+vec2(frandom()-frandom(), frandom()-frandom())*500;
		
	if (m_SnapTick && m_SnapTick < Server()->Tick()-Server()->TickSpeed()*5.0f)
	{
		if (GameServer()->StoreEntity(m_ObjType, m_Type, 0, m_Pos.x, m_Pos.y))
		{
			GameServer()->m_World.DestroyEntity(this);
			return;
		}
	}
		
	if (m_Health <= 0)
	{
		float OldVelY = m_Vel.y;
		m_Vel.y += 0.8f;
		//m_Vel *= 0.97f;
		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(80.0f, 100.0f), 0, false);
		//return;
		
		if (Server()->Tick() > m_DamageTakenTick+30 || OldVelY > 12.0f)
		{
			if (Server()->Tick() > m_DamageTakenTick+200 || abs(m_Vel.y) < 0.2f)
			{
				GameServer()->CreateExplosion(m_Pos+m_Center, TEAM_NEUTRAL, GetDroidWeapon(m_Type, true));
				m_DeathTick = Server()->Tick();
				
				for (int i = 0; i < 3; i++)
				{
					if (frandom() < 0.4f)
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_AMMO, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
					else if (frandom() < 0.4f)
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_ARMOR, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
					else
						GameServer()->m_pController->DropPickup(m_Pos + vec2(0, 0), POWERUP_KIT, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), 0);
				}
				
				if (frandom() < 0.15f)
					GameServer()->m_pController->DropWeapon(m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_UPGRADE)));
				else if (frandom() < 0.1f)
					GameServer()->m_pController->DropWeapon(m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_RESPAWNER)));
				
				GameServer()->m_World.DestroyEntity(this);
				return;
			}
		}
		
		m_Status = DROIDSTATUS_TERMINATED;
	}
	else
	{
		m_AngleTimer += 0.025f;

		if (GameServer()->Collision()->IsInFluid(m_Pos.x, m_Pos.y))
			TakeDamage(vec2(0, -0.5f), 2, -1, vec2(0, 0), DAMAGETYPE_FLUID);
	
		To += vec2(sin(m_AngleTimer), cos(m_AngleTimer))*100.0f;
		
		if (GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To))
		{
			To = m_Pos+vec2(frandom()-frandom(), frandom()-frandom())*300;
			To += vec2(sin(m_AngleTimer), cos(m_AngleTimer))*100.0f;
			GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To);
		}
		
		m_MoveTarget += (To-m_MoveTarget) / 20.0f;
		
		if (abs(length(m_MoveTarget - m_Pos)) > 8.0f)
			m_Vel += normalize(m_MoveTarget - m_Pos) * 0.40f * ((m_Status == DROIDSTATUS_ELECTRIC)?0.5f:1.0f);
		
		m_Vel *= 0.97f;
		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(96.0f, 128.0f), 0, false);
	
		bool WillFire = false;
		
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
			
			WillFire = true;
		}
		
		m_Target = (m_NewTarget+m_Target)/3;
		
		
		if (WillFire)
		{
			if (--m_FireDelay < 0)
			{
				m_FireDelay = 0;
				Fire();
			}
		}

		
		if(Server()->Tick() > m_DamageTakenTick+15)
			m_Status = DROIDSTATUS_IDLE;
	}
}



void CStar::Fire()
{
	if (m_ReloadTimer-- < 0)
	{
		m_FireCount++;
		m_ReloadTimer = 700 * Server()->TickSpeed() / 1000;
		
		vec2 TurretPos = m_Pos+vec2(m_Dir * 16, m_Center.y-20);
		//float Angle = (m_Angle + 90) / (180/pi);

		//vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		
		m_Vel += normalize(m_Target)*6.0f;
		
		GameServer()->CreateSound(m_Pos, SOUND_STAR_FIRE);
		
		GameServer()->CreateProjectile(NEUTRAL_BASE, GetDroidWeapon(m_Type), 0, TurretPos+normalize(m_Target*-1)*30.0f, m_Target*-1, TurretPos);
		GameServer()->CreateProjectile(NEUTRAL_BASE, GetDroidWeapon(m_Type), 0, TurretPos+normalize(m_Target*-1)*30.0f+vec2(-m_Dir * 64, 0), m_Target*-1, TurretPos);
		
		m_AttackTick = Server()->Tick();
	}
	
	if (m_FireCount > 4)
	{
		m_FireCount = 0;
		m_FireDelay = 20;
	}
}



bool CStar::Target()
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
			m_State = CStar::IDLE;
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


bool CStar::FindTarget()
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


void CStar::TickPaused()
{
	//if(m_SpawnTick != -1)
	//	++m_SpawnTick;
}