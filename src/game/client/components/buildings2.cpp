#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
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
#include <game/client/components/sounds.h>
#include <game/client/skelebank.h>

#include "buildings2.h"
#include "binds.h"

void CBuildings2::OnReset()
{

}


void CBuildings2::RenderSawblade(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_SAWBLADE);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(CustomStuff()->m_SawbladeAngle);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y, (128+32));
	
	Graphics()->QuadsEnd();
}


void CBuildings2::RenderFlametrap(const struct CNetObj_Building *pCurrent)
{
	// render flame effect
	int i = pCurrent->m_X/4 + pCurrent->m_Y/16;
	i = i%64;
	
	i = clamp(i, 0, 63);
	
	int s = pCurrent->m_Status;
	if (s & (1<<BSTATUS_FIRE))
	{
		if (CustomStuff()->m_FlametrapState[i] == 0)
			CustomStuff()->m_FlametrapState[i]++;
		
		if (CustomStuff()->m_FlametrapState[i] > 9*6)
			CustomStuff()->m_FlametrapState[i] = 5*6;
		
		if (CustomStuff()->m_FlametrapSoundTick[i] <= Client()->GameTick())
		{
			CustomStuff()->m_FlametrapSoundTick[i] = Client()->GameTick() + 190 * Client()->GameTickSpeed()/1000;
			if (CustomStuff()->m_FlametrapLastSound[i] == 0)
			{
				CustomStuff()->m_FlametrapLastSound[i] = 1;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK1, 1.0f, vec2(pCurrent->m_X, pCurrent->m_Y));
			}
			else
			{
				CustomStuff()->m_FlametrapLastSound[i] = 0;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK2, 1.0f, vec2(pCurrent->m_X, pCurrent->m_Y));
			}
		}
	}
	
	if (CustomStuff()->m_FlametrapState[i] > 0)
	{
		int f = CustomStuff()->m_FlametrapState[i] / 6;
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FLAME].m_Id);
		Graphics()->QuadsBegin();
		
		bool Flip = s & (1<<BSTATUS_MIRROR);
		RenderTools()->SelectSprite(SPRITE_FLAME1+f, Flip ? SPRITE_FLAG_FLIP_X : 0);
		
		Graphics()->SetColor(1, 1, 1, 1);
		Graphics()->QuadsSetRotation(0);
			
		IGraphics::CQuadItem QuadItem2(pCurrent->m_X + (Flip ? -13-72 : 13+72), pCurrent->m_Y - 18, 128, 64);
		Graphics()->QuadsDraw(&QuadItem2, 1);
		
		Graphics()->QuadsEnd();
	}
	
	
	// render frame
	/*
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
		
	s = pCurrent->m_Status;
	bool Flip = s & (1<<BSTATUS_MIRROR);
	
	RenderTools()->SelectSprite(SPRITE_FLAMETRAP, Flip ? SPRITE_FLAG_FLIP_X : 0);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	IGraphics::CQuadItem QuadItem(pCurrent->m_X + (Flip ? -13 : 13), pCurrent->m_Y, 64, 64);
	Graphics()->QuadsDraw(&QuadItem, 1);
	
	Graphics()->QuadsEnd();
	*/
}


void CBuildings2::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const struct CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
			
			switch (pBuilding->m_Type)
			{
			case BUILDING_SAWBLADE:
				RenderSawblade(pBuilding);
				break;
				
			case BUILDING_FLAMETRAP:
				RenderFlametrap(pBuilding);
				break;
				
			default:;
			};
		}
	}
}





