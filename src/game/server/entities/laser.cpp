

#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "laser.h"
#include "building.h"
#include "monster.h"
#include "superexplosion.h"

CLaser::CLaser(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner, int Damage, CBuilding *OwnerBuilding)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Damage = Damage;
	m_Pos = Pos;
	m_Owner = Owner;
	m_Energy = StartEnergy;
	m_Dir = Direction;
	m_OwnerBuilding = OwnerBuilding;
	
	//if (StartEnergy > 200)
	//	m_Bounces = 2;
	//else
		m_Bounces = 0;
	m_EvalTick = 0;
	m_IgnoreScythe = -1;
	GameWorld()->InsertEntity(this);
	DoBounce();
}


bool CLaser::HitCharacter(vec2 From, vec2 To)
{
	vec2 At;
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pHit = GameServer()->m_World.IntersectCharacter(m_Pos, To, 0.f, At, pOwnerChar);
	if(!pHit)
		return false;

	m_From = From;
	m_Pos = At;
	m_Energy = -1;

	//vec2 Dir = normalize(m_Pos-From) * 0.1f;
	
	//pHit->TakeDamage(Dir, m_Damage, m_Owner, WEAPON_RIFLE);
	pHit->TakeDamage(normalize(To-From)*0.1f, m_Damage, m_Owner, WEAPON_LASER, At, DAMAGETYPE_NORMAL, m_OwnerBuilding ? true : false);

	
	return true;
}


bool CLaser::HitScythe(vec2 From, vec2 To)
{
	vec2 At;
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pHit = GameServer()->m_World.IntersectScythe(m_Pos, To, 50.0f, At, pOwnerChar);
	if(!pHit)
		return false;

	if (pHit->GetPlayer()->GetCID() == m_IgnoreScythe)
		return false;
	
	m_From = From;
	m_Pos = At;
	
	vec2 d = pHit->m_Pos-m_Pos;
	d += vec2(frandom()-frandom(), frandom()-frandom()) * length(d) * 0.4f;
	m_Dir = -normalize(d);
	
	GameServer()->CreateBuildingHit(m_Pos);
	m_IgnoreScythe = pHit->GetPlayer()->GetCID();
	
	return true;
}


bool CLaser::HitMonster(vec2 From, vec2 To)
{
	vec2 At;
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	if (!pOwnerChar)
		return false;
	
	CMonster *pHit = GameServer()->m_World.IntersectWalker(m_Pos, To, 8.0f, At);
	if(!pHit)
		return false;
	
	m_From = From;
	m_Pos = At;
	m_Energy = -1;

	pHit->TakeDamage(normalize(To-From)*0.1f, m_Damage, m_Owner, At);
	return true;
}

bool CLaser::HitBuilding(vec2 From, vec2 To)
{
	vec2 At;
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	if (!pOwnerChar)
		return false;
	
	CBuilding *pHit = GameServer()->m_World.IntersectBuilding(m_Pos, To, 8.0f, At, pOwnerChar->GetPlayer()->GetTeam(), m_OwnerBuilding);
	if(!pHit)
		return false;
	
	m_From = From;
	m_Pos = At;
	m_Energy = -1;

	pHit->TakeDamage(m_Damage, m_Owner, WEAPON_LASER);
	GameServer()->CreateBuildingHit(m_Pos);
	return true;
}

void CLaser::DoBounce()
{
	m_EvalTick = Server()->Tick();

	
	if (GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y) == CCollision::COLFLAG_DAMAGEFLUID)
		m_Energy = -1;
	
	if(m_Energy < 0)
	{
		/*
		if (m_ExtraInfo == DOOMROCKETS)
		{
			CSuperexplosion *S = new CSuperexplosion(&GameServer()->m_World, m_Pos, m_Owner, WEAPON_RIFLE, 2);
			GameServer()->m_World.InsertEntity(S);
		}
		*/
		
		GameServer()->m_World.DestroyEntity(this);
		return;
	}

	vec2 To = m_Pos + m_Dir * m_Energy;

	if(GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To))
	{
		if(!HitScythe(m_Pos, To))
		{
			if(!HitCharacter(m_Pos, To))
			{
				if(!HitBuilding(m_Pos, To))
				{
					if(!HitMonster(m_Pos, To))
					{
						// intersected
						m_From = m_Pos;
						m_Pos = To;

						vec2 TempPos = m_Pos;
						vec2 TempDir = m_Dir * 4.0f;

						GameServer()->Collision()->MovePoint(&TempPos, &TempDir, 1.0f, 0);
						m_Pos = TempPos;
						m_Dir = normalize(TempDir);

						m_Energy -= distance(m_From, m_Pos) + GameServer()->Tuning()->m_LaserBounceCost;
						m_Bounces++;

						if(m_Bounces > 4)
							m_Energy = -1;
						
						m_IgnoreScythe = -1;

						GameServer()->CreateSound(m_Pos, SOUND_LASER_BOUNCE);
					}
				}
			}
		}
	}
	else
	{
		if(!HitScythe(m_Pos, To))
		{
			if(!HitCharacter(m_Pos, To))
			{
				if(!HitBuilding(m_Pos, To))
				{
					if(!HitMonster(m_Pos, To))
					{
						m_From = m_Pos;
						m_Pos = To;
						m_Energy = -1;
					}
				}
			}
		}
	}
}

void CLaser::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CLaser::Tick()
{
	if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/1000.0f)
		DoBounce();
}

void CLaser::TickPaused()
{
	++m_EvalTick;
}

void CLaser::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_StartTick = m_EvalTick;
}
