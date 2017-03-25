#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "building.h"
#include "turret.h"
#include "projectile.h"
#include "laser.h"

CTurret::CTurret(CGameWorld *pGameWorld, vec2 Pos, int Team, int Weapon)
: CBuilding(pGameWorld, Pos, BUILDING_TURRET, Team)
{
	m_ProximityRadius = TurretPhysSize;
	m_Life = 80;
	m_MaxLife = 80;
	m_Angle = 0;
	m_TargetTimer = 0;
	m_TargetIndex = -1;
	m_ReloadTimer = 0;
	m_AttackTick = 0;
	m_Weapon = Weapon;
	m_OriginalDirection = vec2(0, 10);
	
	m_Flamethrower = 0;
	
	m_OwnerPlayer = -1;
	m_Chainsaw = 0;
	
	m_Ammo = 0;
	
	// sanity check
	if (m_Weapon < 0 || m_Weapon >= NUM_CUSTOMWEAPONS)
		m_Weapon = 0;
	
	m_Center = vec2(0, -40);
}



void CTurret::SetAngle(vec2 Direction)
{
	m_Angle = GetAngle(Direction) * (180/pi) + 90;
	
	m_OriginalDirection = Direction;
	m_Target = Direction;
}



void CTurret::Tick()
{
	if (m_Life < 40)
		m_aStatus[BSTATUS_REPAIR] = 1;
	else
		m_aStatus[BSTATUS_REPAIR] = 0;
	
	if (m_Ammo <= 0 && aCustomWeapon[m_Weapon].m_MaxAmmo)
		m_aStatus[BSTATUS_NOPE] = 1;
	else
		m_aStatus[BSTATUS_NOPE] = 0;
	
	if (m_DeathTimer > 0)
	{
		m_DeathTimer--;
		if (m_DeathTimer%10 == 1)
		{
			vec2 ep = m_Pos + vec2((frandom()-frandom())*32.0f, -48+(frandom()-frandom())*32.0f);
			GameServer()->CreateExplosion(ep, m_DamageOwner, WEAPON_HAMMER, false, false);
			GameServer()->CreateSound(ep, SOUND_GRENADE_EXPLODE);
		}
			
		if (m_Life <= 0 && m_DeathTimer <= 0)
		{
			GameServer()->CreateExplosion(m_Pos + vec2(0, -48), m_DamageOwner, WEAPON_HAMMER, false, false);
			//GameServer()->CreateExplosion(m_Pos, m_DamageOwner, WEAPON_HAMMER, false, false);
			GameServer()->CreateSound(m_Pos + vec2(0, -48), SOUND_GRENADE_EXPLODE);
			//GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
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
				//m_Target = vec2((frandom()-frandom())*150, frandom()*50-frandom()*100);
				m_Target = m_OriginalDirection;
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
		Fire();
	
	
	if (m_Chainsaw >= Server()->Tick())
	{
		int Owner = NEUTRAL_BASE;
		if (m_Team == TEAM_RED)
			Owner = RED_BASE;
		else if (m_Team == TEAM_BLUE)
			Owner = BLUE_BASE;
		
		float Angle = (m_Angle + 90) / (180/pi);
		vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		vec2 TurretPos = m_Pos+vec2(0, -54);
		GameServer()->CreateChainsawHit(m_OwnerPlayer, m_Weapon, TurretPos, TurretPos+Dir*40, NULL, this);
	}
	
	Flamethrower();
	
	UpdateStatus();
}


void CTurret::Flamethrower()
{
	if (m_Weapon == WEAPON_FLAMER && m_Flamethrower >= Server()->Tick())
	{
		float Angle = (m_Angle + 90) / (180/pi);
		vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		vec2 OffsetY = vec2(0, -48);
		
		vec2 StartPos = m_Pos+Dir*28*3.0f + OffsetY;
		
		for (int i = 0; i < 4; i++)
		{
			vec2 To = StartPos+Dir*28*i*2.1f;
			
			GameServer()->Collision()->IntersectLine(StartPos, To, 0x0, &To);
			GameServer()->CreateFlamethrowerHit(m_OwnerPlayer, m_Weapon, To, NULL, this);
			
			// to visualize hit points
			//GameServer()->CreateFlameHit(To);
		}
	}
	else
		m_Flamethrower = 0;	
}

void CTurret::Fire()
{
	if (m_ReloadTimer-- < 0 && (m_Ammo > 0 || !aCustomWeapon[m_Weapon].m_MaxAmmo))
	{
		m_ReloadTimer = aCustomWeapon[m_Weapon].m_BulletReloadTime * Server()->TickSpeed() / 1000;
		m_Ammo--;
		
		vec2 TurretPos = m_Pos+vec2(0, -52.5f);
		float Angle = (m_Angle + 90) / (180/pi);
		
		int Owner = NEUTRAL_BASE;
		if (m_Team == TEAM_RED)
			Owner = RED_BASE;
		else if (m_Team == TEAM_BLUE)
			Owner = BLUE_BASE;
		
		vec2 Dir = vec2(cosf(Angle), sinf(Angle));
		
		GameServer()->CreateSound(m_Pos, aCustomWeapon[m_Weapon].m_Sound);
		
		if (m_Weapon == WEAPON_LASER)
		{
			CLaser *pLaser = new CLaser(GameWorld(), TurretPos+Dir*40, Dir, GameServer()->Tuning()->m_LaserReach, m_OwnerPlayer, aCustomWeapon[m_Weapon].m_Damage, this);
		}
		else if (m_Weapon == WEAPON_CHAINSAW)
			m_Chainsaw = Server()->Tick() + 500 * Server()->TickSpeed()/1000;
		else if (m_Weapon == WEAPON_FLAMER)
			m_Flamethrower = Server()->Tick() + 400 * Server()->TickSpeed()/1000;
		else
			GameServer()->CreateProjectile(m_OwnerPlayer, m_Weapon, TurretPos+Dir*40, Dir, this);
		
		/*
		CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_RIFLE,
			Owner, // player index
			TurretPos,
			vec2(cosf(Angle), sinf(Angle)),
			(int)(Server()->TickSpeed()*400),
			12, 0, 14, -1);

		// pack the Projectile and send it to the client Directly
		CNetObj_Projectile p;
		pProj->FillInfo(&p);

		CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
		Msg.AddInt(1);
		for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
			Msg.AddInt(((int *)&p)[i]);

		Server()->SendMsg(&Msg, 0, -1);
		*/
		
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

		int iw = clamp(m_Weapon, 0, NUM_WEAPONS-1);
		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < aCustomWeapon[iw].m_AiAttackRange && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
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
		
		if ((!pCharacter->IsAlive() || pCharacter->GetPlayer()->GetCID() == m_OwnerPlayer) && !GameServer()->m_pController->IsTeamplay())
			continue;
		
		if (GameServer()->m_pController->IsCoop() && !pCharacter->m_IsBot)
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
	
	if (!GameServer()->m_pController->IsTeamplay() && SnappingClient == m_OwnerPlayer)
		pP->m_Team = TEAM_BLUE;
	else
		pP->m_Team = m_Team;
	
	pP->m_Status = m_Status;
	pP->m_Weapon = m_Weapon;
	pP->m_AttackTick = m_AttackTick;
}
