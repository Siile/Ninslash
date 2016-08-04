#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "building.h"
#include "turret.h"
#include "projectile.h"

CTurret::CTurret(CGameWorld *pGameWorld, vec2 Pos, int Team)
: CBuilding(pGameWorld, Pos, BUILDING_TURRET, Team)
{
	m_ProximityRadius = TurretPhysSize;
	m_Life = 100;
	m_Angle = 0;
	m_TargetTimer = 0;
	m_TargetIndex = -1;
	m_ShootTimer = 0;
	m_AttackTick = 0;
	
	m_Center = vec2(0, -62);
}



void CTurret::Tick()
{
	if (m_DeathTimer > 0)
	{
		m_DeathTimer--;
		if (m_DeathTimer%10 == 1)
		{
			vec2 ep = m_Pos + vec2((frandom()-frandom())*32.0f, -70+(frandom()-frandom())*32.0f);
			GameServer()->CreateExplosion(ep, m_DamageOwner, WEAPON_HAMMER, false, false);
			GameServer()->CreateSound(ep, SOUND_GRENADE_EXPLODE);
		}
			
		if (m_Life <= 0 && m_DeathTimer <= 0)
		{
			GameServer()->CreateExplosion(m_Pos + vec2(0, -70), m_DamageOwner, WEAPON_HAMMER, false, false);
			GameServer()->CreateExplosion(m_Pos, m_DamageOwner, WEAPON_HAMMER, false, false);
			GameServer()->CreateSound(m_Pos + vec2(0, -70), SOUND_GRENADE_EXPLODE);
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
			GameServer()->m_World.DestroyEntity(this);
		}
		return;
	}

	bool WillFire = false;
	
	if (Target())
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
				m_Target = vec2((frandom()-frandom())*150, frandom()*50-frandom()*100);
			}
		}
	}
	
	int TargetAngle = int(atan2(m_Target.y, m_Target.x) * 180/pi + 90);
	if (TargetAngle < 0)
		TargetAngle += 360;
	
	if (m_Angle < TargetAngle+1)
		m_Angle += 2;
	else if (m_Angle > TargetAngle-1)
		m_Angle -= 2;
	
	if (m_Angle < 0)
		m_Angle += 360;
	
	if (WillFire && abs(m_Angle - TargetAngle) < 30)
		Shoot();
}



void CTurret::Shoot()
{
	if (m_ShootTimer-- < 0)
	{
		m_ShootTimer = 7;
		
		GameServer()->CreateSound(m_Pos, aCustomWeapon[WEAPON_RIFLE].m_Sound);
		
		vec2 TurretPos = m_Pos+vec2(0, -67);
		float Angle = (m_Angle + 90) / (180/pi);
		
		int Owner = NEUTRAL_BASE;
		if (m_Team == TEAM_RED)
			Owner = RED_BASE;
		else if (m_Team == TEAM_BLUE)
			Owner = BLUE_BASE;
		
		CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_GUN,
			Owner, // player index
			TurretPos,
			vec2(cosf(Angle), sinf(Angle)),
			(int)(Server()->TickSpeed()*400),
			12, 0, 14, -1, WEAPON_RIFLE, 0);

		// pack the Projectile and send it to the client Directly
		CNetObj_Projectile p;
		pProj->FillInfo(&p);

		CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
		Msg.AddInt(1);
		for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
			Msg.AddInt(((int *)&p)[i]);

		Server()->SendMsg(&Msg, 0, -1);
		
		m_AttackTick = Server()->Tick();
	}
}


bool CTurret::Target()
{
	vec2 TurretPos = m_Pos+vec2(0, -67);
	
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
		
		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 900 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
		{
			m_Target = TurretPos - ((pCharacter->m_Pos+vec2(0, -64)) + pCharacter->GetCore().m_Vel * 2.0f);
			return true;
		}
		else
			return false;
	}
	
	return false;
}


bool CTurret::FindTarget()
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

		if (pPlayer->GetTeam() == m_Team && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if (!pCharacter->IsAlive())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 900 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
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



void CTurret::TickPaused()
{
}

void CTurret::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Turret *pP = static_cast<CNetObj_Turret *>(Server()->SnapNewItem(NETOBJTYPE_TURRET, m_ID, sizeof(CNetObj_Turret)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Angle = m_Angle;
	pP->m_Team = m_Team;
	pP->m_AttackTick = m_AttackTick;
}
