

#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "lightning.h"

CLightning::CLightning(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, float StepEnergy, int Owner, int Damage, int MaxDesc, int Num)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Damage = Damage;
	m_Pos = Pos;
	m_Owner = Owner;
	m_Energy = StartEnergy;
	m_StartEnergy = StartEnergy;
	m_StepEnergy = StepEnergy;
	m_Num = Num;
	m_Dir = Direction;
	m_Bounces = 0;
	m_EvalTick = 0;
	m_MaxDesc = MaxDesc;
	GameWorld()->InsertEntity(this);
	DoBounce();
}


bool CLightning::HitCharacter(vec2 From, vec2 To)
{
	vec2 At;
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pHit = GameServer()->m_World.IntersectCharacter(m_Pos, To, 0.f, At, pOwnerChar);
	if(!pHit)
		return false;

	m_From = From;
	m_Pos = At;
	m_Energy = -1;
	
	//vec2 d = normalize(m_Pos-From) * 0.1f;

	//pHit->TakeDamage(d, m_Damage, m_Owner, WEAPON_RIFLE);
	pHit->TakeDamage(vec2(0.f, 0.f), m_Damage, m_Owner, WEAPON_RIFLE, At);
		
	return true;
}



void CLightning::DoBounce()
{
	m_EvalTick = Server()->Tick();

	if(m_Energy < 0)
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}

	vec2 To = m_Pos + m_Dir * min(int(m_Energy), int(m_StepEnergy)) + vec2(frandom()-frandom(), frandom()-frandom())*40.0f;

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
			
			m_Energy -= m_StepEnergy;
		}
	}
	else
	{
		if(!HitCharacter(m_Pos, To))
		{
			m_From = m_Pos;
			m_Pos = To;
			m_Energy -= m_StepEnergy;
			
			if (m_Num < m_MaxDesc)
				new CLightning(GameWorld(), m_Pos, m_Dir, m_StartEnergy, m_StepEnergy, m_Owner, m_Damage, m_MaxDesc, m_Num+1);
		}
	}
}

void CLightning::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CLightning::Tick()
{
	if(Server()->Tick() > m_EvalTick+(Server()->TickSpeed()*GameServer()->Tuning()->m_LaserBounceDelay)/2000.0f)
		DoBounce();
}

void CLightning::TickPaused()
{
	++m_EvalTick;
}

void CLightning::Snap(int SnappingClient)
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
