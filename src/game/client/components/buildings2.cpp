#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/demo.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/buildables.h>
#include <game/gamecore.h> // get_angle
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>

#include <game/client/customstuff.h>

#include <game/client/components/controls.h>
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
			
			// new sound
			//CustomStuff()->m_FlametrapSoundTick[i] = Client()->GameTick() + 2000 * Client()->GameTickSpeed()/1000;
			//m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_FIRETRAP, 1.0f, vec2(pCurrent->m_X, pCurrent->m_Y));
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


// todo: separate from buildings2.cpp
void CBuildings2::RenderBuildMode()
{
	if (m_pClient->m_Snap.m_pGameDataObj)
	{
		int Flags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_INFECTION && CustomStuff()->m_LocalTeam == TEAM_BLUE)
			m_pClient->m_pControls->m_BuildMode = false;
	}

	if (!m_pClient->m_pControls->m_BuildMode || !CustomStuff()->m_LocalAlive)
		return;
	
	int Selected = m_pClient->m_pControls->m_SelectedBuilding-1;

	int SnapRange = 128;
	
	if (Selected >= 0 && Selected < 3)
	{
		int Cost = BuildableCost[Selected];

		//if (Cost <= CustomStuff()->m_LocalKits)
		{
			// render selected building
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
			
			Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
			Graphics()->QuadsBegin();

			bool Valid = false;
			
			// snap pos
			vec2 Pos = m_pClient->m_pControls->m_TargetPos;
			
			if (!Collision()->IsTileSolid(Pos.x, Pos.y))
				Valid = true;
			
			// snap y down
			if (Valid && Selected != 2)
			{
				vec2 To = Pos+vec2(0, SnapRange);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Pos = To;
					Pos.y += BuildableOffset[Selected];
				}
				else
					Valid = false;
				
				if (!Collision()->IsTileSolid(To.x - 22, To.y+2) || !Collision()->IsTileSolid(To.x + 22, To.y+2))
					Valid = false;

				if (Collision()->IsTileSolid(To.x , To.y-64))
					Valid = false;
				
				if (Collision()->IsForceTile(To.x, To.y+16))
					Valid = false;
			}
			
			// snap x
			if (Valid && Selected == 2)
			{
				vec2 To = Pos+vec2(SnapRange, 0);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Pos = To;
					CustomStuff()->m_FlipBuilding = true;
					Pos.x -= BuildableOffset[Selected];
				}
				else
					Valid = false;
				
				if (!Valid)
				{
					To = Pos+vec2(-SnapRange, 0);
					if (Collision()->IntersectLine(Pos, To, 0x0, &To))
					{
						Pos = To;
						Pos.x += BuildableOffset[Selected];
						Valid = true;
						CustomStuff()->m_FlipBuilding = false;
					}
				}
				
				int cx = CustomStuff()->m_FlipBuilding ? 16 : -16;
				
				if (!Collision()->IsTileSolid(To.x+cx, To.y-26) || !Collision()->IsTileSolid(To.x+cx, To.y+26))
					Valid = false;
				
				if (Collision()->IsTileSolid(To.x-cx, To.y-26) || Collision()->IsTileSolid(To.x-cx, To.y+26))
					Valid = false;
			}
			
			
			// final sanity checks
			if (Selected != 2 && Valid)
			{
				// ground on both sides
				if (Collision()->IsTileSolid(Pos.x - 12, Pos.y) || Collision()->IsTileSolid(Pos.x + 12, Pos.y))
					Valid = false;
			}
			
			// not too close to other buildings
			if (m_pClient->BuildingNear(Pos, 48))
				Valid = false;
		
			// check for kits
			if (Cost > CustomStuff()->m_LocalKits)
				Valid = false;
		
			// color
			if (Valid)
			{
				CustomStuff()->m_BuildPos = Pos;
				CustomStuff()->m_BuildPosValid = true;
				Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.75f);
			}
			else
			{
				CustomStuff()->m_BuildPosValid = false;
				Graphics()->SetColor(1.0f, 0.0f, 0.0f, 0.75f);
			}
		
			RenderTools()->SelectSprite(SPRITE_KIT_BARREL+Selected,
										(Selected == 2 && CustomStuff()->m_FlipBuilding) ? SPRITE_FLAG_FLIP_X : 0);
										
			RenderTools()->DrawSprite(Pos.x, Pos.y, BuildableSize[Selected]);

			Graphics()->QuadsEnd();
			Graphics()->ShaderEnd();
		}
	}
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
	
	RenderBuildMode();
}





