#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "ball.h"

//MACRO_ALLOC_POOL_ID_IMPL(CBall, 1)

CBall::CBall(CGameWorld *pWorld)
: CEntity(pWorld, CGameWorld::ENTTYPE_BALL)
{
	m_ProximityRadius = ms_PhysSize;
	m_ForceCoreSend = false;
	m_OriginalPos = vec2(0, 0);
}


void CBall::Reset()
{
	Destroy();
}

bool CBall::Spawn(vec2 Pos)
{
	m_Pos = Pos;
	GameServer()->m_pController->m_LastBallToucher = -1;

	if (m_OriginalPos.x == 0)
		m_OriginalPos = m_Pos;
	
	m_Core.Reset();
	m_Core.Init(&GameServer()->m_World.m_Core, GameServer()->Collision());
	m_Core.m_Pos = m_Pos;
	m_ProximityRadius = m_Core.BallSize();

	GameServer()->m_World.m_Core.m_pBall = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));

	GameServer()->m_World.InsertEntity(this);
	
	return true;
}

void CBall::RoundReset()
{
	GameServer()->m_pController->m_LastBallToucher = -1;
	if (m_OriginalPos.x != 0)
		m_Pos = m_OriginalPos;
	
	m_Core.Reset();
	m_Core.m_Pos = m_Pos;
}

void CBall::Destroy()
{
	//GameServer()->m_World.m_Core.m_pBall = 0;
}


void CBall::AddForce(vec2 Force)
{
	m_Core.m_Vel += Force;
	m_Core.PlayerHit();
}

void CBall::Tick()
{
	m_Core.Tick();
	
	if (m_Core.m_ForceCoreSend)
	{
		m_Core.m_ForceCoreSend = false;
		m_ForceCoreSend = true;
	}
	
	if (m_Core.m_Pos.x < -1000 || m_Core.m_Pos.x > GameServer()->Collision()->GetWidth()*32+1000 || m_Core.m_Pos.y < -1000 || m_Core.m_Pos.y > GameServer()->Collision()->GetWidth()*32+1000)
		RoundReset();
}

void CBall::TickDefered()
{
	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GameServer()->Collision());
		m_ReckoningCore.Tick();
		m_ReckoningCore.Move();
		m_ReckoningCore.Quantize();
	}

	m_Core.Move();
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;

	// update the m_SendCore if needed
	{
		CNetObj_Ball Predicted;
		CNetObj_Ball Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		if(m_ForceCoreSend || m_ReckoningTick+Server()->TickSpeed()*0.5f < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Ball)) != 0)
		{
			m_ForceCoreSend = false;
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
}

void CBall::TickPaused()
{
	++m_ReckoningTick;
}


void CBall::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "Ball", "Snap");
	
	CNetObj_Ball *pBall = static_cast<CNetObj_Ball *>(Server()->SnapNewItem(NETOBJTYPE_BALL, 0, sizeof(CNetObj_Ball)));
	if(!pBall)
		return;

	//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "Ball", "Snapped");
	
	// write down the m_Core
	if(!m_ReckoningTick || GameServer()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pBall->m_Tick = 0;
		m_Core.Write(pBall);
	}
	else
	{
		pBall->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pBall);
	}
}
