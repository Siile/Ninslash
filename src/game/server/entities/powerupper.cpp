#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/weapons.h>
#include "character.h"
#include "building.h"
#include "powerupper.h"

CPowerupper::CPowerupper(CGameWorld *pGameWorld, vec2 Pos)
: CBuilding(pGameWorld, Pos, BUILDING_LAZER, TEAM_NEUTRAL)
{
	m_ProximityRadius = PowerupperPhysSize;
	m_Life = 100;

	m_Item = -1;
	m_ItemTakenTick = 0;
}

void CPowerupper::Reset()
{
	m_Life = 100;

	m_Item = -1;
	m_ItemTakenTick = 0;
}

void CPowerupper::Tick()
{
	if (m_Item < 0 && ((!GameServer()->m_pController->IsCoop() && m_ItemTakenTick + Server()->TickSpeed()*30.0f < GameServer()->Server()->Tick()) || m_ItemTakenTick == 0))
	{
		while (m_Item < 0 || m_Item == PLAYERITEM_FILL || m_Item == PLAYERITEM_LANDMINE || m_Item == PLAYERITEM_ELECTROMINE || (m_Item == PLAYERITEM_FUEL && g_Config.m_SvUnlimitedTurbo))
		{
			m_Item = rand()%NUM_PLAYERITEMS;
			if (frandom() < 0.4f)
				m_Item = PLAYERITEM_UPGRADE;
		}
	}
	
	// give buff to player
	if (m_Item >= 0)
	{
		CCharacter *apEnts[MAX_CLIENTS];
		int Num = GameServer()->m_World.FindEntities(m_Pos+vec2(0, -24), 16.0f, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		
		int Bots = 0;
		
		for(int i = 0; i < Num; i++)
		{
			if (apEnts[i]->m_IsBot)
				Bots++;
			else
				apEnts[i]->GiveBuff(m_Item);
		}
		
		if (Num - Bots > 0)
		{
			m_Item = -1;
			m_ItemTakenTick = GameServer()->Server()->Tick();
		}
	}
}



void CPowerupper::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Powerupper *pP = static_cast<CNetObj_Powerupper *>(Server()->SnapNewItem(NETOBJTYPE_POWERUPPER, m_ID, sizeof(CNetObj_Powerupper)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Item = m_Item;
	pP->m_Team = m_Team;
}
