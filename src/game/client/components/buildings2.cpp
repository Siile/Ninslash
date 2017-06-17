#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/demo.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <engine/shared/config.h>

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



void CBuildings2::RenderLightningWall(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_LIGHTNINGWALL);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-4, 64);
	
	Graphics()->QuadsEnd();
	
	
	// lightning effect
	
	vec2 Out, Border;
	
	vec2 From = vec2(pCurrent->m_X, pCurrent->m_Y-5);
	vec2 Pos = From + vec2(0, -600);
	
	Collision()->IntersectLine(From, Pos, 0x0, &Pos);
	
	Pos.y += 20;
	
	vec2 Dir = normalize(Pos-From);
	
	Graphics()->ShaderBegin(SHADER_ELECTRIC, 1.0f);
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	
	Graphics()->SetColor(0.5f, 0.5f, 1, 1.0f);
	//Graphics()->SetColor(0.3f + frandom()*0.4f, 0.3f + frandom()*0.4f, 1, 1.0f);
	int Steps = 2 + length(Pos - From) / 60;
	vec2 Step = (Pos - From) / Steps;
	Out = vec2(Dir.y, -Dir.x) * 1.0f;
		
	vec2 p1 = From;
	vec2 s1 = vec2(0, 0);
		
	vec2 o1 = vec2(0, 0);
			
	for (int i = 0; i < Steps; i++)
	{
		vec2 p2 = p1 + Step;
		vec2 o2 = vec2(0, 0);
			
		if (i < Steps-1)
			o2 = vec2(frandom()-frandom(), (frandom()-frandom())/2) * 15.0f;
			
		vec2 s2 = Out * frandom()*8.0f;
			
		if (i == Steps -1)
			s2 = vec2(0, 0);
			
		IGraphics::CFreeformItem FreeFormItem(
			p1.x-s1.x+o1.x, p1.y-s1.y+o1.y,
			p1.x+s1.x+o1.x, p1.y+s1.y+o1.y,
			p2.x-s2.x+o2.x, p2.y-s2.y+o2.y,
			p2.x+s2.x+o2.x, p2.y+s2.y+o2.y);
								
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		
		//m_pClient->m_pEffects->BulletTrail(p1+o1, p2+o2, vec4(0.5f, 0.5f, 1.0f, 0.2f));
		
		s1 = s2;
		p1 = p2;
		o1 = o2;
	}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
}


void CBuildings2::RenderLightningWallTop(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_LIGHTNINGWALL, SPRITE_FLAG_FLIP_Y);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y+4, 64);
	
	Graphics()->QuadsEnd();
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


void CBuildings2::RenderJumppad(const struct CNetObj_Building *pCurrent)
{
	float Time = 0.0f;
	
	int Anim = pCurrent->m_Status & (1<<BSTATUS_ON) ? ANIM_TRIGGER : ANIM_IDLE;
	
	
	// todo: get a truly unique index
	int i = (pCurrent->m_X/7 + pCurrent->m_Y/15)%512;
	
	float OffsetY = 0.0f;
	
	if (Anim == ANIM_TRIGGER && (CustomStuff()->m_aJumppad[i] < 0.01f || CustomStuff()->m_aJumppad[i] > 1.0f))
	{
		
		//if (CustomStuff()->m_aImpactTick[i] < Client()->GameTick() + 20 * Client()->GameTickSpeed()/1000)
		//	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JUMPPAD, 1.0f, vec2(pCurrent->m_X, pCurrent->m_Y));
			
		CustomStuff()->m_aImpactTick[i] = Client()->GameTick() + 1000 + Client()->GameTickSpeed();
		
		//if (CustomStuff()->m_aJumppad[i] < 0.01f)
		{
			CustomStuff()->m_aJumppad[i] = 0.01f;
			CustomStuff()->AddImpact(vec4(pCurrent->m_X-64, pCurrent->m_Y+OffsetY-16, pCurrent->m_X+64, pCurrent->m_Y+OffsetY+16), CCustomStuff::IMPACT_HIT);
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JUMPPAD, 1.0f, vec2(pCurrent->m_X, pCurrent->m_Y));
		}
		//else
		//	CustomStuff()->AddImpact(vec4(pCurrent->m_X-64, pCurrent->m_Y+OffsetY-16, pCurrent->m_X+64, pCurrent->m_Y+OffsetY+16), CCustomStuff::IMPACT_READY);
	}
	else
	{
	//	CustomStuff()->m_aJumppad[i] = 0.0f;
		CustomStuff()->AddImpact(vec4(pCurrent->m_X-64, pCurrent->m_Y+OffsetY-16, pCurrent->m_X+64, pCurrent->m_Y+OffsetY+16), CCustomStuff::IMPACT_READY);
	}
	
	if (CustomStuff()->m_aJumppad[i] < 3.7f)
		Time = CustomStuff()->m_aJumppad[i] * 0.11f;
	else
		CustomStuff()->m_aJumppad[i] = 0.0f;
		
	RenderTools()->RenderSkeleton(vec2(pCurrent->m_X, pCurrent->m_Y), ATLAS_JUMPPAD, aAnimList[ANIM_TRIGGER], Time, vec2(1.0f, 1.0f)*0.54f, 1, 0);

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
	if (m_pClient->m_Snap.m_pGameInfoObj)
	{
		int Flags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_INFECTION && CustomStuff()->m_LocalTeam == TEAM_BLUE)
			m_pClient->m_pControls->m_BuildMode = false;
	}

	if (!m_pClient->m_pControls->m_BuildMode || !CustomStuff()->m_LocalAlive)
		return;
	
	int Selected = m_pClient->m_pControls->m_SelectedBuilding-1;

	int SnapRange = 128;
	
	if (Selected >= 0 && Selected < NUM_BUILDABLES)
	{
		int Cost = BuildableCost[Selected];

		//if (Cost <= CustomStuff()->m_LocalKits)
		{
			bool Valid = false;
			bool FlipY = false;
			
			// snap pos
			vec2 Pos = m_pClient->m_pControls->m_TargetPos;
			
			if (!Collision()->IsTileSolid(Pos.x, Pos.y))
				Valid = true;
			
			// snap y down
			if (Valid && Selected != BUILDABLE_FLAMETRAP)
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
			
			// snap y up / turret stand
			if (!Valid && (Selected == BUILDABLE_TURRET || Selected == BUILDABLE_TESLACOIL))
			{
				vec2 To = Pos+vec2(0, -SnapRange);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Pos = To;
					Pos.y -= BuildableOffset[Selected];
					Valid = true;
				}
				else
					Valid = false;
				
				if (Valid)
					FlipY = true;
				
				if (!Collision()->IsTileSolid(To.x - 22, To.y-12) || !Collision()->IsTileSolid(To.x + 22, To.y-12))
					Valid = false;

				if (Collision()->IsTileSolid(To.x , To.y+64))
					Valid = false;
				
				if (Collision()->IsTileSolid(To.x , To.y+6))
					Valid = false;
			}
			
			// lightning wall line
			if (Valid && Selected == BUILDABLE_LIGHTNINGWALL)
			{
				vec2 To = Pos+vec2(0, -550);
				if (Collision()->IntersectLine(Pos, To, 0x0, &To))
				{
					Graphics()->TextureSet(-1);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.3f);
				
					IGraphics::CFreeformItem FreeFormItem(
						Pos.x-4, Pos.y,
						Pos.x+4, Pos.y,
						To.x-4, To.y,
						To.x+4, To.y);
										
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				
					Graphics()->QuadsEnd();
				}
				else
					Valid = false;
			}
			
			// snap x
			if (Valid && Selected == BUILDABLE_FLAMETRAP)
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
			float Range = 48.0f;
			
			if (Selected == BUILDABLE_TESLACOIL)
				Range = 74.0f;
			
			if (m_pClient->BuildingNear(Pos, Range))
				Valid = false;
		
			// check for kits
			if (Cost > CustomStuff()->m_LocalKits)
				Valid = false;
		
		
			// render selected building
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
			
			Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
			Graphics()->QuadsBegin();
		
			// color
			if (Valid)
			{
				CustomStuff()->m_BuildPos = Pos;
				
				if (FlipY)
				{
					if (Selected == BUILDABLE_TURRET)
						CustomStuff()->m_BuildPos.y += BuildableOffset[Selected]-18;
					else if (Selected == BUILDABLE_TESLACOIL)
						CustomStuff()->m_BuildPos.y += BuildableOffset[Selected]-38;
				}
				
				CustomStuff()->m_BuildPosValid = true;
				Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.75f);
			}
			else
			{
				CustomStuff()->m_BuildPosValid = false;
				Graphics()->SetColor(1.0f, 0.0f, 0.0f, 0.75f);
			}
		
			RenderTools()->SelectSprite(SPRITE_KIT_BARREL+Selected,
										(Selected == 2 && CustomStuff()->m_FlipBuilding) ? SPRITE_FLAG_FLIP_X : 0 + (FlipY ? SPRITE_FLAG_FLIP_Y : 0));
										
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
			case BUILDING_LIGHTNINGWALL:
				RenderLightningWall(pBuilding);
				break;
				
			case BUILDING_LIGHTNINGWALL2:
				RenderLightningWallTop(pBuilding);
				break;
				
				
			case BUILDING_SAWBLADE:
				RenderSawblade(pBuilding);
				break;
				
			case BUILDING_FLAMETRAP:
				RenderFlametrap(pBuilding);
				break;
				
			case BUILDING_JUMPPAD:
				RenderJumppad(pBuilding);
				break;
				
			default:;
			};
		}
	}
	
	RenderBuildMode();
}





