#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "droid.h"


CDroid::CDroid(CGameWorld *pGameWorld, vec2 Pos, int Type)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DROID)
{
	m_ProximityRadius = DroidPhysSize;

	m_StartPos = Pos;
	m_Type = Type;
	
	Reset();
	//GameWorld()->InsertEntity(this);

}

void CDroid::Reset()
{
	m_Center = vec2(0, -50);
	m_Health = 100;
	m_Pos = m_StartPos;
	m_Status = 0;
	m_Dir = -1;
	m_DeathTick = 0;
	SetState(0);
	m_TargetIndex = -1;
	m_ReloadTimer = 0;
	m_AttackTick = 0;
	m_TargetTimer = 0;
	m_Target = vec2(0, 0);
	m_NewTarget = vec2(0, 0);
	m_Vel = vec2(0, 0);
	m_FlyTargetTick = 0;
	m_Mode = 0;
	m_ProximityRadius = DroidPhysSize;
	m_FireDelay = 0;
	m_FireCount = 0;
	m_AttackTimer = 0;
}



void CDroid::TakeDamage(vec2 Force, int Dmg, int From, vec2 Pos, int Type)
{
	// skip everything while spawning
	//if (m_aStatus[STATUS_SPAWNING] > 0.0f)
	//	return false;
	
	if (g_Config.m_SvOneHitKill)
		Dmg = 1000;

	vec2 DmgPos = m_Pos + m_Center;
	
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



void CDroid::Fire()
{
	
}

void CDroid::Tick()
{

}




bool CDroid::Target()
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
			m_State = CDroid::IDLE;
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


bool CDroid::FindTarget()
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


void CDroid::SetState(int State)
{
	m_State = State;
	m_NextState = State;
	m_StateChangeTick = Server()->Tick();
	
}

void CDroid::TickPaused()
{
	
}

void CDroid::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Droid *pP = static_cast<CNetObj_Droid *>(Server()->SnapNewItem(NETOBJTYPE_DROID, m_ID, sizeof(CNetObj_Droid)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
	pP->m_Status = m_Status;
	pP->m_AttackTick = m_AttackTick;
	pP->m_Anim = m_Anim;
	pP->m_Dir = m_Dir;
	pP->m_Angle = GetAngle(vec2(abs(m_Target.x), m_Target.y*-1)) * (180/pi);
}
