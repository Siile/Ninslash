#include <engine/graphics.h>
#include <engine/demo.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/client/customstuff.h>

#include <game/client/components/effects.h>

#include "monsters.h"

void CMonsters::OnReset()
{

}


void CMonsters::RenderMonster(const CNetObj_Monster *pPrev, const CNetObj_Monster *pCurrent, int ItemID)
{
	vec2 Pos = mix(vec2(pPrev->m_X, pPrev->m_Y), vec2(pCurrent->m_X, pCurrent->m_Y), Client()->IntraGameTick());

	if (pCurrent->m_Status != MONSTERSTATUS_IDLE)
	{
		CustomStuff()->m_MonsterDamageIntensity[ItemID%MAX_MONSTERS] = 1.0f;
		CustomStuff()->m_MonsterDamageType[ItemID%MAX_MONSTERS] = pCurrent->m_Status;
	}
	
	
	if (CustomStuff()->m_MonsterDamageIntensity[ItemID%MAX_MONSTERS] > 0.0f)
	{
		if (CustomStuff()->m_MonsterDamageType[ItemID%MAX_MONSTERS] == MONSTERSTATUS_ELECTRIC)
			RenderTools()->Graphics()->ShaderBegin(SHADER_ELECTRIC, CustomStuff()->m_MonsterDamageIntensity[ItemID%MAX_MONSTERS]);
		else
			RenderTools()->Graphics()->ShaderBegin(SHADER_DAMAGE, CustomStuff()->m_MonsterDamageIntensity[ItemID%MAX_MONSTERS]);
	}
	RenderTools()->RenderMonster(Pos, CustomStuff()->m_MonsterAnim+ItemID*0.3f, pCurrent->m_Dir, pCurrent->m_Status);
	RenderTools()->Graphics()->ShaderEnd();
	
	//dbg_msg("monster", "%u", ItemID);
}



void CMonsters::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_MONSTER)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if(pPrev)
				RenderMonster((const CNetObj_Monster *)pPrev, (const CNetObj_Monster *)pData, Item.m_ID);
		}
	}
}
