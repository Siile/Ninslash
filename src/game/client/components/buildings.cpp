#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/demo.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <engine/shared/config.h>

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


void CBuildings::RenderDoor1(const struct CNetObj_Building *pCurrent)
{
	float Time = 0.0f;
	
	int Anim = pCurrent->m_Status & (1<<BSTATUS_ON) ? ANIM_OPENED : ANIM_CLOSED;
	
	if (Anim == ANIM_CLOSED && pCurrent->m_Status & (1<<BSTATUS_EVENT))
	{
		Anim = ANIM_OPEN;
		if (CustomStuff()->m_DoorTimer == 0.0f)
			CustomStuff()->m_DoorTimer = 0.01f;
		
		Time = CustomStuff()->m_DoorTimer * 0.5f;
	}
	
	RenderTools()->RenderSkeleton(vec2(pCurrent->m_X, pCurrent->m_Y+16), ATLAS_DOOR1, aAnimList[Anim], Time, vec2(1.0f, 1.0f)*0.3f, 1, 0);
}


void CBuildings::RenderScreen(const struct CNetObj_Building *pCurrent)
{
	float Time = CustomStuff()->m_SawbladeAngle*0.1f;
	
	int Anim = ANIM_IDLE1+(pCurrent->m_X/32)%3;
	
	RenderTools()->RenderSkeleton(vec2(pCurrent->m_X, pCurrent->m_Y+18), ATLAS_SCREEN, aAnimList[Anim], Time, vec2(1.0f, 1.0f)*0.7f, 0, 0);
}


void CBuildings::RenderSpeaker(const struct CNetObj_Building *pCurrent)
{
	if (!g_Config.m_SndEnvironmental)
		return;
	
	bool Flip = false;
	vec2 p = vec2(pCurrent->m_X, pCurrent->m_Y);
	
	if (Collision()->CheckPoint(pCurrent->m_X - 48, pCurrent->m_Y))
		Flip = true;
	
	// sound
	int64 currentTime = time_get();
	
	int i = (pCurrent->m_X/64 + pCurrent->m_Y/17)%MAX_BG_SOUNDS;
	
	if (currentTime > CustomStuff()->m_aBGSound[i])
	{
		CustomStuff()->m_aBGSound[i] = currentTime+int(time_freq()*3.7f);
		m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_BG1+i%13, 1.0f, p);
	}
	
	if (currentTime > CustomStuff()->m_aBGEffect[i])
	{
		vec2 o = vec2(-28, 28);
		if (Flip)
			o.x *= -1;
		
		if (frandom() < 0.3f)
			CustomStuff()->m_aBGEffect[i] = currentTime+(time_freq()*3.7f)/16.0f;
		else
			CustomStuff()->m_aBGEffect[i] = currentTime+(time_freq()*3.7f)/8.0f;
		
		m_pClient->m_pEffects->Electrospark(p+o, 48, o*2.0f+vec2(frandom()-frandom(), frandom()-frandom()) * 20.0f);
	}
	
	// render
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_SPEAKER, Flip ? SPRITE_FLAG_FLIP_X : 0);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(p.x, p.y, 128);
	Graphics()->QuadsEnd();
}


void CBuildings::RenderSwitch(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_SWITCH);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-14, 96);
	Graphics()->QuadsEnd();
	
	Graphics()->ShaderBegin(SHADER_ELECTRIC, frandom());
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(pCurrent->m_Status & (1<<BSTATUS_ON) ? SPRITE_SWITCH_ON : SPRITE_SWITCH_OFF);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-14, 96);
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
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

void CBuildings::RenderPowerBarrel(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_POWERBARREL);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-16, 96+12);
	
	Graphics()->QuadsEnd();
}




void CBuildings::RenderFlametrap(const struct CNetObj_Building *pCurrent)
{
	// render the actual building in buildings2
	
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
	/*
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
	*/
}


void CBuildings::RenderReactor(const struct CNetObj_Building *pCurrent)
{
	int Anim = ANIM_IDLE;
	int s = pCurrent->m_Status;
	bool Repair = s & (1<<BSTATUS_REPAIR);
	
	float Time = pCurrent->m_X * 0.432f + pCurrent->m_Y * 0.2354f + CustomStuff()->m_SawbladeAngle * 0.1f;
	
	if (Repair)
		Time += CustomStuff()->m_SawbladeAngle * 0.15f;
	
	m_pClient->m_pEffects->Light(vec2(pCurrent->m_X, pCurrent->m_Y-30), 320);
	m_pClient->m_pEffects->Light(vec2(pCurrent->m_X, pCurrent->m_Y-0), 320);
	
	RenderTools()->RenderSkeleton(vec2(pCurrent->m_X, pCurrent->m_Y+16+50), ATLAS_REACTOR, aAnimList[Anim], Time, vec2(1.0f, 1.0f)*0.8f, 1, 0);
	
	// damage mask
	if (Repair)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_REACTOR_DAMAGE].m_Id);
		Graphics()->QuadsBegin();
		
		RenderTools()->SelectSprite(SPRITE_REACTOR_DAMAGE);
		
		Graphics()->SetColor(1, 1, 1, 1);
		Graphics()->QuadsSetRotation(0);
			
		RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y, 70);
		
		Graphics()->QuadsEnd();
	}
	
	// repair sprite
	if (Repair && (CustomStuff()->LocalTick()/12+(pCurrent->m_X/8 + pCurrent->m_Y/32))%8 < 4)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_STATUS_REPAIR);
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(0);
		
		RenderTools()->DrawSprite(pCurrent->m_X-20, pCurrent->m_Y-102, 52);
		Graphics()->QuadsEnd();
	}
	
	//int f = CustomStuff()->m_SawbladeAngle * 100;
	
	
	if (Repair && frandom() < 0.15f)
		m_pClient->m_pEffects->Electrospark(vec2(pCurrent->m_X, pCurrent->m_Y-10+frandom()*30-frandom()*30)+vec2(frandom()-frandom(), frandom()-frandom()) * 20.0f, 20+frandom()*20, vec2(0, 0));
	
	/*
	if (Repair && CustomStuff()->LocalTick()%30 == 1)
		m_pClient->m_pEffects->Electrospark(vec2(pCurrent->m_X, pCurrent->m_Y-30-frandom()*10)+vec2(frandom()-frandom(), frandom()-frandom()) * 20.0f, 20+frandom()*20, vec2(0, 0));
	if (Repair && CustomStuff()->LocalTick()%30 == 11)
		m_pClient->m_pEffects->Electrospark(vec2(pCurrent->m_X, pCurrent->m_Y+20-frandom()*10)+vec2(frandom()-frandom(), frandom()-frandom()) * 20.0f, 20+frandom()*20, vec2(0, 0));
	*/
}


void CBuildings::RenderTeslacoil(const struct CNetObj_Building *pCurrent)
{
	int Anim = ANIM_IDLE;
	int s = pCurrent->m_Status;
	bool Repair = s & (1<<BSTATUS_REPAIR);
	
	s = pCurrent->m_Status;
	int FlipY = (s & (1<<BSTATUS_MIRROR)) ? -1 : 1;
	
	float Time = pCurrent->m_X * 0.432f + pCurrent->m_Y * 0.2354f + CustomStuff()->m_SawbladeAngle * 0.06f;
	
	if (Repair)
		Time += CustomStuff()->m_SawbladeAngle * 0.1f;
	
	//m_pClient->m_pEffects->Light(vec2(pCurrent->m_X, pCurrent->m_Y-30), 320);
	//m_pClient->m_pEffects->Light(vec2(pCurrent->m_X, pCurrent->m_Y-0), 320);
	
	RenderTools()->RenderSkeleton(vec2(pCurrent->m_X, pCurrent->m_Y+20*FlipY), ATLAS_TESLACOIL, aAnimList[Anim], Time, vec2(1.0f, 1.0f*FlipY)*0.55f, 1, 0, pCurrent->m_Team);

	
	// repair sprite
	if (Repair && (CustomStuff()->LocalTick()/12+(pCurrent->m_X/8 + pCurrent->m_Y/32))%8 < 4)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_STATUS_REPAIR);
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(0);
		
		RenderTools()->DrawSprite(pCurrent->m_X-20, pCurrent->m_Y-30-70*FlipY, 52);
		Graphics()->QuadsEnd();
	}
	
	//int f = CustomStuff()->m_SawbladeAngle * 100;
	
	
	if (Repair && frandom() < 0.15f)
		m_pClient->m_pEffects->Electrospark(vec2(pCurrent->m_X, pCurrent->m_Y-(10+frandom()*30)*FlipY)+vec2(frandom()-frandom(), frandom()-frandom()) * 20.0f, 20+frandom()*20, vec2(0, 0));
}


void CBuildings::RenderDestroyedReactor(const struct CNetObj_Building *pCurrent)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	
	RenderTools()->SelectSprite(SPRITE_REACTOR_DESTROYED);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
	
	RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-12, 183);
	
	Graphics()->QuadsEnd();
}

	
void CBuildings::RenderStand(const struct CNetObj_Building *pCurrent)
{
	int s = pCurrent->m_Status;
	bool Flip = s & (1<<BSTATUS_MIRROR);
	int FlipY = Flip ? -1 : 1;
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_STAND, Flip ? SPRITE_FLAG_FLIP_Y : 0);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	//RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-24, 96*1.3f);
	
	float Scale = 0.8f;
	IGraphics::CQuadItem QuadItem(pCurrent->m_X, pCurrent->m_Y-25*FlipY, 96*Scale, 128*Scale); // -37
	Graphics()->QuadsDraw(&QuadItem, 1);
	
	Graphics()->QuadsEnd();
	
	// skip tip if local player is undead
	if (m_pClient->m_Snap.m_pGameInfoObj)
	{
		int Flags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_INFECTION && CustomStuff()->m_LocalTeam == TEAM_BLUE)
			return;
	}
	
	// render drop weapon tip for local player
	if (distance(CustomStuff()->m_LocalPos, vec2(pCurrent->m_X, pCurrent->m_Y+15)) < 45 && 
		CustomStuff()->m_LocalWeapon != WEAPON_TOOL && CustomStuff()->m_LocalWeapon != WEAPON_SCYTHE && CustomStuff()->m_LocalWeapon != WEAPON_HAMMER)
	{
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, pCurrent->m_X + 22, pCurrent->m_Y - 30 - 60*FlipY, 32, m_pClient->m_pBinds->GetKey("+dropweapon"), -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}
}
	
void CBuildings::RenderLazer(const struct CNetObj_Building *pCurrent)
{
	// danger line
	Graphics()->ShaderBegin(SHADER_DEATHRAY, 0.3f);
	Graphics()->TextureSet(-1);
	RenderTools()->SelectSprite(-1);
	Graphics()->QuadsBegin();
	
	Graphics()->SetColor(0, 0, 0, 0.15f);
	
	vec2 p1 = vec2(pCurrent->m_X, pCurrent->m_Y+8);
	//vec2 p2 = vec2(pCurrent->m_X, pCurrent->m_Y+1200);
	
	float y = 64.0f;
	bool Break = false;
	
	float s1 = 2.0f;
	
	while (y < 1200.0f && !Break)
	{
		vec2 p2 = vec2(pCurrent->m_X, pCurrent->m_Y+y);
		if (Collision()->IntersectLine(p1, p2, 0x0, &p2))
			Break = true;
		
		float s2 = frandom()*6.0f;
		
		IGraphics::CFreeformItem FreeFormItem(
			p1.x-s1, p1.y,
			p1.x+s1, p1.y,
			p1.x-s2, p2.y,
			p1.x+s2, p2.y);
							
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	
		s1 = s2;
		p1 = p2;
		y += 64.0f;
	}
	
	/*
	Collision()->IntersectLine(p1, p2, 0x0, &p2);
	
	IGraphics::CFreeformItem FreeFormItem(
		p1.x-2, p1.y,
		p1.x+2, p1.y,
		p1.x-2, p2.y,
		p1.x+2, p2.y);
						
	Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	*/
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	
	// deathray
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
	
	int s = pCurrent->m_Status;
	bool Flip = s & (1<<BSTATUS_MIRROR);
	int FlipY = Flip ? -1 : 1;
	
	// stand
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_STAND, Flip ? SPRITE_FLAG_FLIP_Y : 0);
	
	Graphics()->SetColor(1, 1, 1, 1);
	Graphics()->QuadsSetRotation(0);
		
	//RenderTools()->DrawSprite(pCurrent->m_X, pCurrent->m_Y-24-9, 96*1.3f);
	
	IGraphics::CQuadItem Stand(pCurrent->m_X, pCurrent->m_Y-25*FlipY, 96*Scale, 128*Scale); // -37
	Graphics()->QuadsDraw(&Stand, 1);
	Graphics()->QuadsEnd();
	
	
	int Weapon = pCurrent->m_Weapon;
	float Angle = (pCurrent->m_Angle+90) / (180/pi);
	vec2 p = Pos + vec2(cosf(Angle)*12, sinf(Angle)*12+(-40-9)*FlipY); //+ vec2(cosf(Angle)*90, sinf(Angle)*90-71);
	vec2 Dir = GetDirection((int)(Angle*256));
	
	
	// render weapon
	RenderTools()->SetShadersForWeapon(Weapon);
		
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(Angle);
	
	RenderTools()->RenderWeapon(Weapon, p, Dir, WEAPON_GAME_SIZE);
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	
	
	// render muzzle
	if (GetWeaponFiringType(Weapon) != WFT_HOLD)
	{
		//float Alpha = 0.0f;
		//int Phase1Tick = (Client()->GameTick() - Player.m_AttackTick);
					
		//if (Phase1Tick < 10)
		{
			/*
			if (Player.m_AttackTick && Player.m_AttackTick != pCustomPlayerInfo->m_MuzzleTick)
						{
							vec2 Moff = GetMuzzleRenderOffset(Player.m_Weapon);
							vec2 DirY(-Dir.y,Dir.x);
							vec2 MuzzlePos = p + Dir * Moff.x + DirY * Moff.y;
							//vec2 MuzzlePos = p + Dir * 20;
							m_pClient->m_pEffects->Muzzle(MuzzlePos, Dir, Player.m_Weapon);
						}
						
						pCustomPlayerInfo->AddMuzzle(Player.m_AttackTick, Player.m_Weapon);
					}
						
					
					// render muzzles
					for (int i = 0; i < 4; i++)
					{
						if (pCustomPlayerInfo->m_aMuzzleWeapon[i])
						{
							Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MUZZLE].m_Id);
							Graphics()->QuadsBegin();
							Graphics()->QuadsSetRotation(Angle);
							
							vec2 Moff = GetMuzzleRenderOffset(Player.m_Weapon);
							RenderTools()->SelectSprite(SPRITE_MUZZLE1_1 + pCustomPlayerInfo->m_aMuzzleType[i]*4 + pCustomPlayerInfo->m_aMuzzleTime[i]*4, SPRITE_FLAG_FLIP_X);

							vec2 DirY(-Dir.y,Dir.x);
							vec2 MuzzlePos = p + Dir * Moff.x + DirY * Moff.y;

							RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, 60);
							
							Graphics()->QuadsEnd();	
						}
			*/
		}
	}
	
	

	vec2 Offset = vec2(0, 0);
	
	// fastener
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
	Graphics()->QuadsBegin();
	RenderTools()->SelectSprite(SPRITE_TURRET_FASTENER, Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
	Graphics()->SetColor(1, 1, 1, 1);
	
	
	if (m_pClient->m_Snap.m_pGameInfoObj)
	{
		int Flags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
		int Team = pCurrent->m_Team;
	
		if ((Flags & GAMEFLAG_TEAMS) && !(Flags & GAMEFLAG_INFECTION))
		{
			if (Team == TEAM_RED)
				Graphics()->SetColor(1.0f, 0.8f, 0.0f, 1.0f);
			else if (Team == TEAM_BLUE)
				Graphics()->SetColor(0.3f, 0.5f, 1.0f, 1.0f);
		}
	}
	// local player's turret
	else if (pCurrent->m_Team == TEAM_BLUE)
	{
		vec4 c = CustomStuff()->m_LocalColor;
		Graphics()->SetColor(c.r, c.g, c.b, 1);
	}
	
	Graphics()->QuadsSetRotation(Angle);
		
	RenderTools()->DrawSprite(pCurrent->m_X+Offset.x, pCurrent->m_Y+(-40-9)*FlipY+Offset.y, 64*1.3f);
	Graphics()->QuadsEnd();

	
	/*
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
			vec2 p = Pos + vec2(cosf(Angle)*80, sinf(Angle)*80-49*FlipY);
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
	
	*/
	// no ammo & low health status
	s = pCurrent->m_Status;
	bool Repair = s & (1<<BSTATUS_REPAIR);
	
	//s = pCurrent->m_Status;
	//bool NoAmmo = s & (1<<BSTATUS_NOPE);
	bool NoAmmo = false;
	
	if (Repair && (CustomStuff()->LocalTick()/12+(pCurrent->m_X/8 + pCurrent->m_Y/32))%8 < 4)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_STATUS_REPAIR);
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(0);
		
		RenderTools()->DrawSprite(pCurrent->m_X - (NoAmmo ? 24 : 0), pCurrent->m_Y-50-52*FlipY, 52);
		Graphics()->QuadsEnd();
	}
	
	/*
	s = pCurrent->m_Status;
	if (NoAmmo && (CustomStuff()->LocalTick()/12+(pCurrent->m_X/8 + pCurrent->m_Y/32))%8 >= 4)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_STATUS_NOPE);
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(0);
			
		RenderTools()->DrawSprite(pCurrent->m_X + (Repair ? 24 : 0), pCurrent->m_Y-50-52*FlipY, 52);
		Graphics()->QuadsEnd();
		
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PICKUPS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpritePickup);
		
		Graphics()->QuadsSetRotation(0);
			
		//RenderTools()->DrawSprite(pCurrent->m_X + (Repair ? 24 : 0), pCurrent->m_Y-102, 52);
		IGraphics::CQuadItem Ammo(pCurrent->m_X + (Repair ? 24 : 0), pCurrent->m_Y-50-52*FlipY, 18, 36); // -37
		Graphics()->QuadsDraw(&Ammo, 1);
		Graphics()->QuadsEnd();
	}
	*/
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
				
			case BUILDING_POWERBARREL:
				RenderPowerBarrel(pBuilding);
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
				
			case BUILDING_REACTOR:
				RenderReactor(pBuilding);
				break;
				
			case BUILDING_TESLACOIL:
				RenderTeslacoil(pBuilding);
				break;
				
			case BUILDING_REACTOR_DESTROYED:
				RenderDestroyedReactor(pBuilding);
				break;
				
			case BUILDING_SWITCH:
				RenderSwitch(pBuilding);
				break;
				
			case BUILDING_DOOR1:
				RenderDoor1(pBuilding);
				break;
				
			case BUILDING_SPEAKER:
				RenderSpeaker(pBuilding);
				break;
				
			case BUILDING_SCREEN:
				RenderScreen(pBuilding);
				break;
				
			default:;
			};
			
			//m_pClient->m_pEffects->Light(vec2(pBuilding->m_X, pBuilding->m_Y), 512);
		}
	}
}





