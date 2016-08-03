#include <engine/graphics.h>
#include <engine/demo.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/client/customstuff.h>

#include <game/client/components/flow.h>
#include <game/client/components/effects.h>
#include <game/client/skelebank.h>

#include "buildings.h"

void CBuildings::OnReset()
{

}



void CBuildings::RenderPowerupper(const struct CNetObj_Powerupper *pCurrent)
{
	// draw base
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_POWERUPPER);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-12, 96);
	Graphics()->QuadsEnd();
	

	if (pCurrent->m_Item >= 0)
	{
		// draw background effect
		{
			Graphics()->ShaderBegin(SHADER_SPAWN, 0.5f);
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
			Graphics()->QuadsBegin();
			
			RenderTools()->SelectSprite(SPRITE_ITEM1+pCurrent->m_Item);
			
			Graphics()->SetColor(1, 1, 1, 1);
			Graphics()->QuadsSetRotation(0);
				
			RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-48, 96);
			Graphics()->QuadsEnd();
			Graphics()->ShaderEnd();
		}
		
		// draw pickable item / buff
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
			Graphics()->QuadsBegin();
			
			RenderTools()->SelectSprite(SPRITE_ITEM1+pCurrent->m_Item);
			
			Graphics()->SetColor(1, 1, 1, 1);
			Graphics()->QuadsSetRotation(0);
				
			RenderTools()->DrawSprite(pCurrent->m_X+sin(CustomStuff()->m_SawbladeAngle/8)*2.0f, pCurrent->m_Y+cos(CustomStuff()->m_SawbladeAngle/4)*4.0f-48, 64);
			Graphics()->QuadsEnd();
		}
	}
}

void CBuildings::RenderSawblade(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_SAWBLADE);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(CustomStuff()->m_SawbladeAngle);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y, 128+32);
	
	Graphics()->QuadsEnd();
}

void CBuildings::RenderMine(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_MINE2);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y, 48);
	
	Graphics()->QuadsEnd();
}

void CBuildings::RenderElectromine(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_MINE1);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y, 48);
	
	Graphics()->QuadsEnd();
}

void CBuildings::RenderBarrel(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_BARREL8);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-16, 96+12);
	
	Graphics()->QuadsEnd();
}
	
void CBuildings::RenderLazer(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_LAZER);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y+15, 64);
	
	Graphics()->QuadsEnd();
}
	
void CBuildings::RenderTurret(const struct CNetObj_Turret *pCurrent)
{
	vec2 Pos = vec2(pCurrent->m_X, pCurrent->m_Y);

	m_pClient->m_pEffects->Light(Pos, 256);
	
	RenderTools()->RenderBuilding(Pos+vec2(0, 16), RenderTools()->Skelebank()->m_lSkeletons[SKELETON_TURRET], RenderTools()->Skelebank()->m_lAtlases[ATLAS_TURRET], pCurrent->m_Team, pCurrent->m_Angle);

	float Angle = (pCurrent->m_Angle+90) / (180/pi);
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(Angle);
	
	int iw = WEAPON_RIFLE;
	float IntraTick = Client()->IntraGameTick();
	
	// check if we're firing stuff
	if(g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)//prev.attackticks)
	{
		vec2 p = Pos + vec2(cosf(Angle)*90, sinf(Angle)*90-71);
		vec2 Dir = GetDirection((int)(Angle*256));
		
		float Alpha = 0.0f;
		int Phase1Tick = (Client()->GameTick() - pCurrent->m_AttackTick);
		if (Phase1Tick < (g_pData->m_Weapons.m_aId[iw].m_Muzzleduration + 3))
		{
			float t = ((((float)Phase1Tick) + IntraTick)/(float)g_pData->m_Weapons.m_aId[iw].m_Muzzleduration);
			Alpha = mix(2.0f, 0.0f, min(1.0f,max(0.0f,t)));
		}

		int IteX = rand() % g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles;
		static int s_LastIteX = IteX;
		if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		{
			const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
			if(pInfo->m_Paused)
				IteX = s_LastIteX;
			else
				s_LastIteX = IteX;
		}
		else
		{
			if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
				IteX = s_LastIteX;
			else
				s_LastIteX = IteX;
		}
		if (Alpha > 0.0f && g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX])
		{
			float OffsetY = -g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsety;
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX], Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
			if(Dir.x < 0)
				OffsetY = -OffsetY;

			vec2 DirY(-Dir.y,Dir.x);
			vec2 MuzzlePos = p + DirY * OffsetY;

			RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		}
	}
	
	Graphics()->QuadsEnd();
}



void CBuildings::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_TURRET)
			RenderTurret((const CNetObj_Turret *)pData);
		else if(Item.m_Type == NETOBJTYPE_POWERUPPER)
			RenderPowerupper((const CNetObj_Powerupper *)pData);
		else if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const struct CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
			
			switch (pBuilding->m_Type)
			{
			case BUILDING_SAWBLADE:
				RenderSawblade(pBuilding);
				break;
				
			case BUILDING_MINE1:
				RenderMine(pBuilding);
				break;
				
			case BUILDING_MINE2:
				RenderElectromine(pBuilding);
				break;
				
			case BUILDING_BARREL:
				RenderBarrel(pBuilding);
				break;
				
			case BUILDING_LAZER:
				RenderLazer(pBuilding);
				break;
				
			default:;
			};
		}
	}
}





