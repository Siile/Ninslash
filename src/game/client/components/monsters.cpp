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


void CMonsters::RenderWalker(const CNetObj_Monster *pPrev, const CNetObj_Monster *pCurrent, int ItemID)
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
	
	RenderTools()->RenderWalker(Pos, pCurrent->m_Anim, CustomStuff()->m_MonsterAnim+ItemID*0.3f, pCurrent->m_Dir*-1, pCurrent->m_Angle/2, pCurrent->m_Status);
	RenderTools()->Graphics()->ShaderEnd();
	
	int Dir = pCurrent->m_Dir;
	
	// muzzle
	//if (Client()->GameTick() > pCurrent->m_AttackTick + 100)
	{
		float Angle = 0.0f;
		
		if (Dir > 0)
			Angle = pCurrent->m_Angle / (180/pi);
		else
			Angle = (180-pCurrent->m_Angle) / (180/pi);
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MUZZLE].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(Angle);
		Graphics()->SetColor(1, 1, 1, 1);
	
		// muzzle
		float IntraTick = Client()->IntraGameTick();
		
		int MuzzleDuration = 10;
		
		// check if we're firing stuff
		{
			//vec2 Dir = GetDirection((int)(Angle*256));
			
			int Sprite = SPRITE_MUZZLE1;
			
			float Alpha = 0.0f;
			int Phase1Tick = (Client()->GameTick() - pCurrent->m_AttackTick);
			if (Phase1Tick < MuzzleDuration) // duration
			{
				float t = ((((float)Phase1Tick) + IntraTick)/(float)MuzzleDuration);
				Alpha = mix(2.0f, 0.0f, min(1.0f,max(0.0f,t)));
			}
			
			Sprite += Phase1Tick/2;
			if (Sprite > SPRITE_MUZZLE1+2)
				Sprite = SPRITE_MUZZLE1+2;
			
			if (Alpha > 0.0f)
			{
				RenderTools()->SelectSprite(Sprite, Dir < 0 ? SPRITE_FLAG_FLIP_Y : 0);
				
				float OffsetY = pCurrent->m_Anim < 3 ? -57 : 7;
				
				vec2 p = Pos + vec2(Dir*14, OffsetY) + vec2(cosf(Angle), sinf(Angle))*55;
				RenderTools()->DrawSprite(p.x, p.y, 96);
				
				p = Pos + vec2(Dir*28, OffsetY-7) + vec2(cosf(Angle), sinf(Angle))*64;
				p += vec2(sinf(Angle), 0)*6*Dir;
				RenderTools()->DrawSprite(p.x, p.y, 96);
			}
		}
		
		Graphics()->QuadsEnd();
	}
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
				RenderWalker((const CNetObj_Monster *)pPrev, (const CNetObj_Monster *)pData, Item.m_ID);
		}
	}
}
