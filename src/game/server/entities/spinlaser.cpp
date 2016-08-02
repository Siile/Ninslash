

#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "spinlaser.h"

CSpinlaser::CSpinlaser(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner1, int Owner2, int Damage)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Damage = Damage;
	m_Pos = Pos;
	m_Owner1 = Owner1;
	m_Owner2 = Owner2;
	m_Energy = StartEnergy;
	m_Dir = Direction;

	
	m_EvalTick = Server()->Tick();
	GameWorld()->InsertEntity(this);
	
	m_DamageTimer = 0;
}


bool CSpinlaser::HitCharacter(vec2 From, vec2 To)
{
	vec2 At;
	CCharacter *pOwner1Char = GameServer()->GetPlayerChar(m_Owner1);
	CCharacter *pOwner2Char = GameServer()->GetPlayerChar(m_Owner2);
	CCharacter *pHit = GameServer()->m_World.IntersectCharacter(m_Pos, To, 0.f, At, pOwner1Char);
	if(!pHit)
		return false;

	m_From = From;
	m_Pos = At;
	m_Energy = -1;
	if (m_DamageTimer <= 0)
	{
		if (pOwner1Char)
		{
			if (pOwner1Char != pHit && pOwner2Char != pHit)
				pHit->TakeDamage(vec2(0.f, 0.f), m_Damage, m_Owner1, WEAPON_RIFLE, vec2(0, 0));
		}
		else
			pHit->TakeDamage(vec2(0.f, 0.f), m_Damage, m_Owner1, WEAPON_RIFLE, At);
		m_DamageTimer = 5;
	}
	return true;
}


void CSpinlaser::Spin()
{
	m_DamageTimer--;
	vec2 To = m_Pos + m_Dir * m_Energy;

	if(GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To))
	{
		if(!HitCharacter(m_Pos, To))
		{
			// intersected
			m_From = m_Pos;
			m_Pos = To;

			vec2 TempPos = m_Pos;
			vec2 TempDir = m_Dir * 4.0f;

			GameServer()->Collision()->MovePoint(&TempPos, &TempDir, 1.0f, 0);
			m_Pos = TempPos;
			m_Dir = normalize(TempDir);
		}
	}
}

void CSpinlaser::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}


void CSpinlaser::Update(vec2 Pos, vec2 Dir)
{
	m_EvalTick = Server()->Tick();
	m_Dir = Dir;
	m_Pos = Pos;
}


void CSpinlaser::Tick()
{
	Spin();
	
	if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/2000.0f)
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}
}

void CSpinlaser::TickPaused()
{
	++m_EvalTick;
}

void CSpinlaser::Snap(int SnappingClient)
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
