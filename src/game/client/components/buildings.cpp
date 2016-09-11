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

#include "buildings.h"
#include "binds.h"

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
	/*
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_SAWBLADE);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(CustomStuff()->m_SawbladeAngle);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y, 128+32);
	
	Graphics()->QuadsEnd();
	*/
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
	
	RenderTools()->SelectSprite(SPRITE_BARREL1+(pCurrent->m_X/32+pCurrent->m_Y)%8);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-16, 96+12);
	
	Graphics()->QuadsEnd();
}


void CBuildings::RenderFlametrap(const struct CNetObj_Building *pCurrent)
{
	// render flame effect
	/*
	int i = pCurrent->m_X/4 + pCurrent->m_Y/16;
	i = i%64;
	
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
			
		IGraphics::CQuadItem QuadItem2(pCurrent->m_X + (Flip ? -13-64 : 13+64), pCurrent->m_Y - 17, 128, 64);
		Graphics()->QuadsDraw(&QuadItem2, 1);
		
		Graphics()->QuadsEnd();
	}
	*/
	
	// render frame
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
		
	int s = pCurrent->m_Status;
	bool Flip = s & (1<<BSTATUS_MIRROR);
	
	RenderTools()->SelectSprite(SPRITE_FLAMETRAP, Flip ? SPRITE_FLAG_FLIP_X : 0);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	IGraphics::CQuadItem QuadItem(pCurrent->m_X + (Flip ? -13 : 13), pCurrent->m_Y, 64, 64);
	Graphics()->QuadsDraw(&QuadItem, 1);
	
	Graphics()->QuadsEnd();
}


void CBuildings::RenderBase(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_KIT_BASE);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	//RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y+8, 64*1.3f);
	
	float Scale = 0.8f;
	IGraphics::CQuadItem QuadItem(pCurrent->m_X, pCurrent->m_Y+7.5f, 96*Scale, 32*Scale); // -5
	Graphics()->QuadsDraw(&QuadItem, 1);
	
	Graphics()->QuadsEnd();
}

void CBuildings::RenderStand(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_STAND);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	//RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-24, 96*1.3f);
	
	float Scale = 0.8f;
	IGraphics::CQuadItem QuadItem(pCurrent->m_X, pCurrent->m_Y-25, 96*Scale, 128*Scale); // -37
	Graphics()->QuadsDraw(&QuadItem, 1);
	
	Graphics()->QuadsEnd();
	
	if (distance(CustomStuff()->m_LocalPos, vec2(pCurrent->m_X, pCurrent->m_Y)) < 40 && 
		CustomStuff()->m_LocalWeapon != WEAPON_TOOL && CustomStuff()->m_LocalWeapon != WEAPON_HAMMER && CustomStuff()->m_LocalWeapon != WEAPON_GUN)
	{
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, pCurrent->m_X + 22, pCurrent->m_Y - 90, 32, m_pClient->m_pBinds->GetKey("+dropweapon"), -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}
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
	float Scale = 0.8f;
	
	// stand
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_STAND);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	//RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-24-9, 96*1.3f);
	
	IGraphics::CQuadItem Stand(pCurrent->m_X, pCurrent->m_Y-25, 96*Scale, 128*Scale); // -37
	Graphics()->QuadsDraw(&Stand, 1);
	Graphics()->QuadsEnd();
	
	
	// weapon
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();

	int Weapon = pCurrent->m_Weapon;
	float Angle = (pCurrent->m_Angle+90) / (180/pi);
	vec2 p = Pos + vec2(cosf(Angle)*12, sinf(Angle)*12-40-9); //+ vec2(cosf(Angle)*90, sinf(Angle)*90-71);
	vec2 Dir = GetDirection((int)(Angle*256));
	
	Graphics()->QuadsSetRotation(Angle);
	
	int iw = clamp(Weapon, 0, NUM_WEAPONS-1);
	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
	
	/*
	p = Dir * g_pData->m_Weapons.m_aId[iw].m_Offsetx;
	p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
	*/

	RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
	Graphics()->QuadsEnd();
	
	// fastener
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_TURRET_FASTENER, Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
	Graphics()->SetColor(1, 1, 1, 1);
	
	if (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_INFECTION))
	{
		if (pCurrent->m_Team == TEAM_RED)
			Graphics()->SetColor(1, 0.8f, 0.0f, 1);
		if (pCurrent->m_Team == TEAM_BLUE)
			Graphics()->SetColor(0.3f, 0.5f, 1, 1);
	}
	else if (!(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS))
	{
		vec4 c = CustomStuff()->m_LocalColor;
		if (pCurrent->m_Team == TEAM_BLUE)
			Graphics()->SetColor(c.r, c.g, c.g, 1);
	}

	
	Graphics()->QuadsSetRotation(Angle);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-40-9, 64*1.3f);
	Graphics()->QuadsEnd();
	
	
	if (iw == WEAPON_RIFLE || iw == WEAPON_SHOTGUN)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(Angle);
		Graphics()->SetColor(1, 1, 1, 1);
	
		// muzzle
		float IntraTick = Client()->IntraGameTick();
		
		// check if we're firing stuff
		if(g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)//prev.attackticks)
		{
			vec2 p = Pos + vec2(cosf(Angle)*80, sinf(Angle)*80-49);
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
	
	// no ammo & low health status
	int s = pCurrent->m_Status;
	bool Repair = s & (1<<BSTATUS_REPAIR);
	
	s = pCurrent->m_Status;
	bool NoAmmo = s & (1<<BSTATUS_NOPE);
	
	if (Repair && (CustomStuff()->LocalTick()/12+(pCurrent->m_X/8 + pCurrent->m_Y/32))%8 < 4)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_STATUS_REPAIR);
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(0);
		
		RenderTools()->DrawSprite(pCurrent->m_X - (NoAmmo ? 24 : 0), pCurrent->m_Y-102, 52);
		Graphics()->QuadsEnd();
	}
	
	s = pCurrent->m_Status;
	if (NoAmmo && (CustomStuff()->LocalTick()/12+(pCurrent->m_X/8 + pCurrent->m_Y/32))%8 >= 4)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_STATUS_NOPE);
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(0);
			
		RenderTools()->DrawSprite(pCurrent->m_X + (Repair ? 24 : 0), pCurrent->m_Y-102, 52);
		Graphics()->QuadsEnd();
		
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PICKUPS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpritePickup);
		
		Graphics()->QuadsSetRotation(0);
			
		//RenderTools()->DrawSprite(pCurrent->m_X + (Repair ? 24 : 0), pCurrent->m_Y-102, 52);
		IGraphics::CQuadItem Ammo(pCurrent->m_X + (Repair ? 24 : 0), pCurrent->m_Y-102, 18, 36); // -37
		Graphics()->QuadsDraw(&Ammo, 1);
		Graphics()->QuadsEnd();
	}
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
				
			case BUILDING_FLAMETRAP:
				RenderFlametrap(pBuilding);
				break;
				
			case BUILDING_BASE:
				RenderBase(pBuilding);
				break;
				
			case BUILDING_STAND:
				RenderStand(pBuilding);
				break;
				
			default:;
			};
		}
	}
}





