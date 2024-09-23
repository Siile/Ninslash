#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <game/server/gamecontext.h>

#include "block.h"

CBlock::CBlock(CGameWorld *pGameWorld, int Type, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_BLOCK)
{
	m_Pos = vec2(int(Pos.x / 32)*32, int(Pos.y / 32)*32);
	m_Type = Type;
	m_OriginalType = Type;
	
	if (Type == 1)
		m_Life = 100;
	else
		m_Life = 250;
	
	m_MaxLife = m_Life;
	
	m_DestroyTick = 0;
	Sync();
	GameServer()->Collision()->SetBlock(ivec2(m_Pos.x, m_Pos.y), true);
	GameServer()->OnBlockChange(m_Pos);
	GameWorld()->InsertEntity(this);
}

void CBlock::Reset()
{
	Sync();
}

void CBlock::SurvivalReset()
{
	Sync();
}

void CBlock::Destroy()
{
	//GameServer()->Collision()->SetBlock(ivec2(m_Pos.x, m_Pos.y), false);
	GameServer()->m_World.DestroyEntity(this);
	m_Life = 0;
}


void CBlock::Sync()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aSnapped[i] = false;
		m_aSnapTimer[i] = 0;
	}
}


void CBlock::Tick()
{
	if (m_DestroyTick && m_DestroyTick < Server()->Tick())
	{
		m_DestroyTick = 0;
		Destroy();
		return;
	}
	

	for (int i = 0; i < MAX_CLIENTS; i++)
		if (m_aSnapped[i])
		{
			if (m_aSnapTimer[i]++ > 20)
			{
				m_aSnapped[i] = false;
				m_aSnapTimer[i] = 0;
			}
		}
}


void CBlock::TakeDamage(int Damage)
{
	if (m_Life <= 0 || m_Type == 0)
		return;
	
	if (Damage < 0 && m_Life >= m_MaxLife)
		return;
	
	m_Life -= Damage;
	//GameServer()->CreateDamageInd(m_Pos, 0, -Damage, -1);
	
	int NewType = mix(m_OriginalType+3, m_OriginalType, float(m_Life)/float(m_MaxLife));
	if (NewType != m_Type)
	{
		m_Type = NewType;
		Sync();
	}
	
	// snap to clients when dead
	if (m_Life <= 0)
	{
		m_Type = 0;
		m_DestroyTick = Server()->Tick() + Server()->TickSpeed()*3.0f;
		GameServer()->Collision()->SetBlock(ivec2(m_Pos.x, m_Pos.y), false);
		GameServer()->OnBlockChange(m_Pos);
		
		for (int i = 0; i < MAX_CLIENTS; i++)
			m_aSnapped[i] = false;
	}
}


void CBlock::TickPaused()
{
}

void CBlock::Snap(int SnappingClient)
{
	// snap only so often to reduce bandwidth usage
	if (m_aSnapped[SnappingClient])
		return;
	
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Block *pP = static_cast<CNetObj_Block *>(Server()->SnapNewItem(NETOBJTYPE_BLOCK, m_ID, sizeof(CNetObj_Block)));
	if(!pP)
		return;

	m_aSnapped[SnappingClient] = true;
	
	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
}
