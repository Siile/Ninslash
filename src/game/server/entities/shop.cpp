#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/weapons.h>
#include "character.h"
#include "building.h"
#include "shop.h"

CShop::CShop(CGameWorld *pGameWorld, vec2 Pos)
: CBuilding(pGameWorld, Pos, BUILDING_SHOP, TEAM_NEUTRAL)
{
	m_ProximityRadius = 64;
	m_Life = 9000;

	m_Collision = false;
	m_Autofill = true;
	
	for (int i = 0; i < 4; i++)
		m_aItem[i] = 0;
	
	FillSlots();
}

void CShop::Reset()
{
	m_Life = 9000;

}

void CShop::FillSlots()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_aItem[i])
			continue;
		
		int t = GetRandomWeaponType(false);
		
		if (frandom() < 0.1f)
			t = GetStaticWeapon(SW_UPGRADE);
		
		if (WeaponMaxLevel(t) > 0 && GetStaticType(t) != SW_UPGRADE)
		{
			if (frandom() < 0.1f && WeaponMaxLevel(t) >= 4)
				m_aItem[i] = GetChargedWeapon(t, WeaponMaxLevel(t)+4);
			else if (frandom() < 0.1f && WeaponMaxLevel(t) >= 4)
				m_aItem[i] = GetChargedWeapon(t, WeaponMaxLevel(t)+2);
			else if (frandom() < 0.1f && WeaponMaxLevel(t) < 4)
				m_aItem[i] = GetChargedWeapon(t, WeaponMaxLevel(t)+2);
			else if (frandom() < 0.1f && WeaponMaxLevel(t) < 4)
				m_aItem[i] = GetChargedWeapon(t, WeaponMaxLevel(t)+1);
			else if (frandom() < 0.5f)
				m_aItem[i] = GetChargedWeapon(t, frandom()*(WeaponMaxLevel(t)+1));
			else
				m_aItem[i] = t;
		}
		else
			m_aItem[i] = t;
	}
}


int CShop::GetItem(int Slot)
{
	if (Slot < 0 || Slot >= 4)
		return 0;
	
	return m_aItem[Slot];
}

void CShop::ClearItem(int Slot)
{
	if (Slot < 0 || Slot >= 4)
		return;
	
	m_aItem[Slot] = 0;
	
	if (m_Autofill)
		FillSlots();
}


void CShop::SurvivalReset()
{
	FillSlots();
}

void CShop::Tick()
{
	/*
	if (m_Item >= 0)
	{
		CCharacter *apEnts[MAX_CLIENTS];
		int Num = GameServer()->m_World.FindEntities(m_Pos+vec2(0, -24), 16.0f, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		
		int Bots = 0;
		bool Taken = false;
		
		for(int i = 0; i < Num; i++)
		{
			if (apEnts[i]->m_IsBot)
				Bots++;
			else
				Taken = apEnts[i]->GiveBuff(m_Item);
		}
		
		//if (Num - Bots > 0)
		if (Taken)
		{
			m_Item = -1;
			m_ItemTakenTick = GameServer()->Server()->Tick();
		}
	}
	*/
}


void CShop::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Shop *pP = static_cast<CNetObj_Shop *>(Server()->SnapNewItem(NETOBJTYPE_SHOP, m_ID, sizeof(CNetObj_Shop)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Team = m_Team;
	pP->m_Item1 = m_aItem[0];
	pP->m_Item2 = m_aItem[1];
	pP->m_Item3 = m_aItem[2];
	pP->m_Item4 = m_aItem[3];
}
