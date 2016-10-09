

#include <engine/demo.h>
#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/animdata.h>


#include <game/client/customstuff.h>
#include <game/client/customstuff/playerinfo.h>

#include <game/client/components/flow.h>
#include <game/client/components/skins.h>
#include <game/client/components/effects.h>
#include <game/client/components/sounds.h>
#include <game/client/components/controls.h>

#include "players.h"

void CPlayers::RenderHand(CTeeRenderInfo *pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset, vec2 PostRotOffset)
{
	// for drawing hand
	//const skin *s = skin_get(skin_id);

	float BaseSize = 10.0f;
	//dir = normalize(hook_pos-pos);

	vec2 HandPos = CenterPos + Dir;
	float Angle = GetAngle(Dir);
	if (Dir.x < 0)
		Angle -= AngleOffset;
	else
		Angle += AngleOffset;

	vec2 DirX = Dir;
	vec2 DirY(-Dir.y,Dir.x);

	if (Dir.x < 0)
		DirY = -DirY;

	HandPos += DirX * PostRotOffset.x;
	HandPos += DirY * PostRotOffset.y;

	//Graphics()->TextureSet(data->m_aImages[IMAGE_CHAR_DEFAULT].id);
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HAND].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

	// two passes
	//for (int i = 0; i < 2; i++)
	{
		//bool OutLine = i == 0;

		//RenderTools()->SelectSprite(OutLine?SPRITE_TEE_HAND_OUTLINE:SPRITE_TEE_HAND, 0, 0, 0);
		RenderTools()->SelectSprite(SPRITE_HAND);
		Graphics()->QuadsSetRotation(Angle);
		IGraphics::CQuadItem QuadItem(HandPos.x, HandPos.y, 2*BaseSize, 2*BaseSize);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}

	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsEnd();
}

inline float NormalizeAngular(float f)
{
	return fmod(f+pi*2, pi*2);
}

inline float AngularMixDirection (float Src, float Dst) { return sinf(Dst-Src) >0?1:-1; }
inline float AngularDistance(float Src, float Dst) { return asinf(sinf(Dst-Src)); }

inline float AngularApproach(float Src, float Dst, float Amount)
{
	float d = AngularMixDirection (Src, Dst);
	float n = Src + Amount*d;
	if(AngularMixDirection (n, Dst) != d)
		return Dst;
	return n;
}


void CPlayers::RenderPlayer(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPrevInfo,
	const CNetObj_PlayerInfo *pPlayerInfo
	)
{
	CNetObj_Character Prev;
	CNetObj_Character Player;
	Prev = *pPrevChar;
	Player = *pPlayerChar;

	vec2 WeaponOffset = vec2(0, -11);
	
	CNetObj_PlayerInfo pInfo = *pPlayerInfo;
	CTeeRenderInfo RenderInfo = m_aRenderInfo[pInfo.m_ClientID];

	CPlayerInfo *pCustomPlayerInfo = &CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID];
	
	bool NewTick = m_pClient->m_NewTick;

	
	// set size
	RenderInfo.m_Size = 64.0f;

	float IntraTick = Client()->IntraGameTick();

	float Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, IntraTick)/256.0f;

	
	//float angle = 0;

	if(pInfo.m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		// just use the direct input if it's local player we are rendering
		Angle = GetAngle(m_pClient->m_pControls->m_MousePos);
	}
	else
	{
		// If player move his weapon through top then change the end angle on 2*Pi.
		// So mix function will calculate offset angle by a short path, and not by long one.
		if (Player.m_Angle > (256.0f * pi) && Prev.m_Angle < 0)
		{
			Player.m_Angle -= 256.0f * 2 * pi;
			Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, IntraTick) / 256.0f;
		}
		else if (Player.m_Angle < 0 && Prev.m_Angle > (256.0f * pi))
		{
			Player.m_Angle += 256.0f * 2 * pi;
			Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, IntraTick) / 256.0f;
		}
	}

	// use preditect players if needed
	if(pInfo.m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(!m_pClient->m_Snap.m_pLocalCharacter || (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
		{
		}
		else
		{
			// apply predicted results
			m_pClient->m_PredictedChar.Write(&Player);
			m_pClient->m_PredictedPrevChar.Write(&Prev);
			IntraTick = Client()->PredIntraGameTick();
			NewTick = m_pClient->m_NewPredictedTick;
		}
	}

	vec2 Direction = GetDirection((int)(Angle*256.0f));
	vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), IntraTick);
	vec2 Vel = mix(vec2(Prev.m_VelX/256.0f, Prev.m_VelY/256.0f), vec2(Player.m_VelX/256.0f, Player.m_VelY/256.0f), IntraTick);

	m_pClient->m_pFlow->Add(Position, Vel*100.0f, 10.0f);

	RenderInfo.m_GotAirJump = Player.m_Jumped&2?0:1;

	// detect events
	if(NewTick)
	{
		// detect air jump
		if(!RenderInfo.m_GotAirJump && !(Prev.m_Jumped&2))
			m_pClient->m_pEffects->AirJump(Position);
	}

	// Player.IsOnForceTile()
	
	int ForceState = Collision()->IsForceTile(Player.m_X-8, Player.m_Y+18);
	if (ForceState == 0)
		ForceState = Collision()->IsForceTile(Player.m_X+8, Player.m_Y+18);
	
	ForceState *= 1024.0f;
	
	/*
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "Vel.x = %d", int(Player.m_VelX));
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", aBuf);
	*/
	
	bool Stationary = Player.m_VelX <= 1 + ForceState && Player.m_VelX >= -1 + ForceState;
	bool InAir = !Collision()->CheckPoint(Player.m_X, Player.m_Y+16);
	bool WantOtherDir = (Player.m_Direction == -1 && Vel.x > 0) || (Player.m_Direction == 1 && Vel.x < 0);

	
	// flip feet animation when needed
	if (!InAir || ((Direction.x < 0 && Player.m_VelX < 0) || (Direction.x > 0 && Player.m_VelX > 0)))
	{
		if (Direction.x > 0.0f)
		{
			CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_FlipFeet = true;
		}
		else
		{
			CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_FlipFeet = false;
		}
	}
	
	
	m_pClient->AddFluidForce(Position+vec2(0, -12), Vel*2);
	
	
	float AnimSpeed = abs(Vel.x) / 300.0f;
	
	float FlipRunAnim = false;
	
	if ((Direction.x < 0 && Player.m_VelX > 0) || (Direction.x > 0 && Player.m_VelX < 0))
		FlipRunAnim = true;
	
	if (Direction.x > 0)
	{
		pCustomPlayerInfo->Animation()->m_Flip = false;
		pCustomPlayerInfo->Animation()->m_HeadTilt = GetAngle(Direction) / 3.0f;
	}
	
	if (Direction.x < 0)
	{
		pCustomPlayerInfo->Animation()->m_Flip = true;
		pCustomPlayerInfo->Animation()->m_HeadTilt = GetAngle(vec2(-Direction.x, Direction.y)) / 3.0f;
	}
	
	int WantedAnimation = -1;
	float AnimationSpeed = 0.0f;
	

	if(InAir)
	{
		if (Vel.y > 0 && pCustomPlayerInfo->Animation()->GetAnimation() == PANIM_AIRJUMP)
			pCustomPlayerInfo->m_AirJumpAnimLoaded = false;
		
		if (!RenderInfo.m_GotAirJump && Vel.y < 0 && pCustomPlayerInfo->m_AirJumpAnimLoaded)
		{
			//pCustomPlayerInfo->Animation()->SetAnimation(PANIM_AIRJUMP, 0.01f);
			WantedAnimation = PANIM_AIRJUMP;
			AnimationSpeed = 0.01f;
		}
		else
		{
			//pCustomPlayerInfo->Animation()->SetAnimation(PANIM_JUMP, 0.01f);
			WantedAnimation = PANIM_JUMP;
			AnimationSpeed = 0.01f;
		}
	}
	else
	{
		pCustomPlayerInfo->m_AirJumpAnimLoaded = true;
		if(Stationary)
		{
			//pCustomPlayerInfo->Animation()->SetAnimation(PANIM_IDLE, 0.01f);
			WantedAnimation = PANIM_IDLE;
			AnimationSpeed = 0.01f;
		}
		else
		{
			if (AnimSpeed > 0)
			{
				if (FlipRunAnim)
				{
					//pCustomPlayerInfo->Animation()->SetAnimation(PANIM_RUNBACK);
					//pCustomPlayerInfo->Animation()->SetSpeed(AnimSpeed);
					WantedAnimation = PANIM_RUNBACK;
					AnimationSpeed = AnimSpeed;
				}
				else
				{
					//pCustomPlayerInfo->Animation()->SetAnimation(PANIM_RUN);
					//pCustomPlayerInfo->Animation()->SetSpeed(AnimSpeed);
					WantedAnimation = PANIM_RUN;
					AnimationSpeed = AnimSpeed;
				}
			}
		}
	}
		
	bool Paused = true;
		
	static float s_LastGameTickTime = Client()->GameTickTime();
	if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
	{
		s_LastGameTickTime = Client()->GameTickTime();
		Paused = false;
	}
		
	if (Player.m_Weapon == WEAPON_HAMMER)
	{
		// melee attack effect
		if (pCustomPlayerInfo->m_MeleeTick < Player.m_AttackTick && !Paused)
		{
			pCustomPlayerInfo->m_MeleeTick = Player.m_AttackTick;
			
			if (pCustomPlayerInfo->m_MeleeState == MELEE_UP)
			{
				pCustomPlayerInfo->m_MeleeAnimState = 1.0f;
				pCustomPlayerInfo->m_MeleeState = MELEE_DOWN;
				m_pClient->m_pEffects->SwordHit(Position+vec2(0, -24)+Direction*60, GetAngle(Direction), false);
				pCustomPlayerInfo->m_Weapon2Recoil += Direction * 15;
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*80, vec2(frandom()-frandom(), frandom()-frandom())*30);
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*95, vec2(frandom()-frandom(), frandom()-frandom())*30);
			}
			else
			{
				pCustomPlayerInfo->m_MeleeAnimState = 1.0f;
				pCustomPlayerInfo->m_MeleeState = MELEE_UP;
				m_pClient->m_pEffects->SwordHit(Position+vec2(0, -24)+Direction*60, GetAngle(Direction), true);
				pCustomPlayerInfo->m_Weapon2Recoil += Direction * 15;
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*80, vec2(frandom()-frandom(), frandom()-frandom())*30);
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*95, vec2(frandom()-frandom(), frandom()-frandom())*30);
			}
		}
	}
	else if (Player.m_Weapon == WEAPON_TOOL)
	{
		// melee attack effect
		if (pCustomPlayerInfo->m_MeleeTick < Player.m_AttackTick && !Paused)
		{
			pCustomPlayerInfo->m_MeleeTick = Player.m_AttackTick;
			pCustomPlayerInfo->m_ToolAngleOffset = 45.0f;
			pCustomPlayerInfo->m_Weapon2Recoil += Direction * 15;
		}
	}
	else
	{
		pCustomPlayerInfo->m_MeleeTick = Player.m_AttackTick;
	}


	// do skidding
	if(!InAir && WantOtherDir && length(Vel*50) > 350.0f) // from 500 to 350
	{
		static int64 SkidSoundTime = 0;
		if(time_get()-SkidSoundTime > time_freq()/10)
		{
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_SKID, 0.25f, Position);
			SkidSoundTime = time_get();
		}

		m_pClient->m_pEffects->SkidTrail(
			Position+vec2(-Player.m_Direction*6,12),
			vec2(-Player.m_Direction*100*length(Vel),-50)
		);
	}
	
	// store some data to customstuff for easy access
	if (pInfo.m_Local)
	{
		CustomStuff()->m_LocalColor = RenderInfo.m_ColorBody;
		CustomStuff()->m_LocalWeapon = Player.m_Weapon;
		CustomStuff()->m_LocalPos = Position;
		CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].SetLocal();
		
		int Group = m_pClient->m_pControls->m_InputData.m_WantedWeapon;
		
		if (Group > 0 && Group < 4)
			CustomStuff()->m_WantedWeapon = m_pClient->m_pControls->m_InputData.m_WantedWeapon;
		
		CustomStuff()->m_SelectedWeapon = Player.m_Weapon;
	}
	

	// draw aim line 
	if (pPlayerInfo->m_Local && g_Config.m_GoreAimLine && pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING] < 0.9f)
	{
		//vec2 Pos = Position + Direction * 400; //Position + Direction * 500.0f;
		vec2 Pos = Position + m_pClient->m_pControls->m_MousePos; //Position + Direction * 500.0f;
		vec2 From = Position + WeaponOffset;
		vec2 Dir = Direction;

		vec2 Out, Border;

		Graphics()->BlendNormal();
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();

		float a = 1.0f - pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY];
		
		// do outline
		if (Player.m_Weapon == WEAPON_SHOTGUN)
			Graphics()->SetColor(1.0f, 1.0f, 0.2f, 0.3f*a);
		else if (Player.m_Weapon == WEAPON_RIFLE)
			Graphics()->SetColor(0.2f, 1.0f, 0.2f, 0.3f*a);
		else if (Player.m_Weapon == WEAPON_GRENADE || Player.m_Weapon == WEAPON_FLAMER)
			Graphics()->SetColor(1.0f, 0.2f, 0.2f, 0.3f*a);
		else if (Player.m_Weapon == WEAPON_LASER || Player.m_Weapon == WEAPON_ELECTRIC)
			Graphics()->SetColor(0.2f, 0.2f, 1.0f, 0.3f*a);
		else if (Player.m_Weapon == WEAPON_CHAINSAW || Player.m_Weapon == WEAPON_HAMMER)
			Graphics()->SetColor(1.0f, 0.2f, 0.2f, 0.0f);
		else
			Graphics()->SetColor(0.2f, 1.0f, 0.2f, 0.3f*a);

		Out = vec2(Dir.y, -Dir.x) * 1.0f;

		IGraphics::CFreeformItem Freeform2(
				From.x-Out.x, From.y-Out.y,
				From.x+Out.x, From.y+Out.y,
				Pos.x-Out.x, Pos.y-Out.y,
				Pos.x+Out.x, Pos.y+Out.y);
		Graphics()->QuadsDrawFreeform(&Freeform2, 1);
		
		Graphics()->QuadsEnd();
	}

	
	CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_Weapon = Player.m_Weapon;
	
	if (Player.m_DamageTick > pCustomPlayerInfo->m_DamageTick)
	{
		pCustomPlayerInfo->m_DamageTick = Player.m_DamageTick;
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE] = 1.0f;
	}
	
	
	// render weapon
	if (Player.m_Weapon != WEAPON_TOOL && Player.m_Weapon != WEAPON_HAMMER)
	{
		if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY] > 0.0f)
			Graphics()->ShaderBegin(SHADER_DEATHRAY);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE] > 0.0f)
			Graphics()->ShaderBegin(SHADER_ELECTRIC, pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING] > 0.0f)
			Graphics()->ShaderBegin(SHADER_SPAWN, pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE] > 0.0f)
			Graphics()->ShaderBegin(SHADER_DAMAGE, pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY] > 0.0f)
			Graphics()->ShaderBegin(SHADER_INVISIBILITY, pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] > 0.0f)
			Graphics()->ShaderBegin(SHADER_RAGE, pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL] > 0.0f)
			Graphics()->ShaderBegin(SHADER_FUEL, pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL]);
		
		//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		//Graphics()->QuadsSetRotation(State.GetAttach()->m_Angle*pi*2+Angle);
		Graphics()->QuadsSetRotation(Angle);

		// normal weapons
		int iw = clamp(Player.m_Weapon, 0, NUM_WEAPONS-1);
		//RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);

		vec2 Dir = Direction;
		float Recoil = 0.0f;
		vec2 p;

		{
			Recoil = 0;
			static float s_LastIntraTick = IntraTick;
			if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
				s_LastIntraTick = IntraTick;

			float a = (Client()->GameTick()-Player.m_AttackTick+s_LastIntraTick)/5.0f;
			
			if(a < 1)
			{
				Recoil = sinf(a*pi);

				if (CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilLoaded)
				{
					CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilLoaded = false;
					if (Player.m_Weapon == WEAPON_GUN)
						CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilVel -= Direction * 10.0f;
					else if (Player.m_Weapon == WEAPON_CHAINSAW)
					{
						CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilVel -= Direction * 1.0f;
						pCustomPlayerInfo->m_LastChainsawSoundTick = Client()->GameTick() + 500 * Client()->GameTickSpeed()/1000;
						
						
						m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*80, vec2(frandom()-frandom(), frandom()-frandom())*30);
					}
					else
						CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilVel -= Direction * 14.0f;
						
					/*
					if (Player.m_Weapon == WEAPON_GUN || Player.m_Weapon == WEAPON_SHOTGUN)
					{
						CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponFlashAlpha = 1.0f;
						CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponFlashNum = rand() % g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles;
					}
					*/
				}
			}
			else
			{
				CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilLoaded = true;
			}
			
			p = Position + Dir * g_pData->m_Weapons.m_aId[iw].m_Offsetx + CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoil + CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_Weapon2Recoil + WeaponOffset;
			
			// chainsaw shaking
			if (!Paused && Player.m_Weapon == WEAPON_CHAINSAW && Player.m_AttackTick > Client()->GameTick() - 500 * Client()->GameTickSpeed()/1000)
			{
				p = p + vec2(frandom()-frandom(), frandom()-frandom()) * 4.0f;
			}
			
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		}
		
		Graphics()->QuadsEnd();
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		//Graphics()->QuadsSetRotation(State.GetAttach()->m_Angle*pi*2+Angle);
		Graphics()->QuadsSetRotation(Angle);
		
		Graphics()->ShaderEnd();
		
		// muzzle
		if (Player.m_Weapon == WEAPON_GUN || Player.m_Weapon == WEAPON_RIFLE || Player.m_Weapon == WEAPON_SHOTGUN)
		{
			// check if we're firing stuff
			if(g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)//prev.attackticks)
			{
				float Alpha = 0.0f;
				int Phase1Tick = (Client()->GameTick() - Player.m_AttackTick);
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
					RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX], Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
					if(Direction.x < 0)
						OffsetY = -OffsetY;

					vec2 DirY(-Dir.y,Dir.x);
					vec2 MuzzlePos = p + Dir * g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsetx + DirY * OffsetY;

					RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
				}
			}
		}
		Graphics()->QuadsEnd();
		
		if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY] > 0.0f)
			Graphics()->ShaderBegin(SHADER_DEATHRAY);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE] > 0.0f)
			Graphics()->ShaderBegin(SHADER_ELECTRIC, pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING] > 0.0f)
			Graphics()->ShaderBegin(SHADER_SPAWN, pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE] > 0.0f)
			Graphics()->ShaderBegin(SHADER_DAMAGE, pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY] > 0.0f)
			Graphics()->ShaderBegin(SHADER_INVISIBILITY, pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] > 0.0f)
			Graphics()->ShaderBegin(SHADER_RAGE, pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE]);
		else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL] > 0.0f)
			Graphics()->ShaderBegin(SHADER_FUEL, pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL]);
		
		switch (Player.m_Weapon)
		{
			case WEAPON_GUN: RenderHand(&RenderInfo, p, Direction, -3*pi/4, vec2(-15, 4)); break;
			case WEAPON_SHOTGUN: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 4)); break;
			case WEAPON_GRENADE: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_RIFLE: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_ELECTRIC: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_LASER: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_FLAMER: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-10, 7)); break;
			case WEAPON_CHAINSAW:
				RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, -4));
				//RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-42, 4));
				break;
		}

		Graphics()->ShaderEnd();
	}

	RenderInfo.m_Size = 64.0f; // force some settings
	RenderInfo.m_ColorBody.a = 1.0f;
	RenderInfo.m_ColorFeet.a = 1.0f;
	RenderInfo.m_ColorTopper.a = 1.0f;
	RenderInfo.m_ColorSkin.a = 1.0f;
	
	// jetpack effects
	if (Player.m_Jetpack == 1 && Player.m_JetpackPower > 0 && Player.m_Wallrun == 0)
	{
		m_pClient->m_pEffects->Triangle(Position + vec2(4, 8), vec2(Vel.x*-20, 600));
		m_pClient->m_pEffects->Triangle(Position + vec2(-4, 8), vec2(Vel.x*-20, 600));
		
		if (pCustomPlayerInfo->m_LastJetpackSoundTick <= Client()->GameTick())
		{
			pCustomPlayerInfo->m_LastJetpackSoundTick = Client()->GameTick() + 190 * Client()->GameTickSpeed()/1000;
			if (pCustomPlayerInfo->m_LastJetpackSound == 0)
			{
				pCustomPlayerInfo->m_LastJetpackSound = 1;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK1, 1.0f, Position);
			}
			else
			{
				pCustomPlayerInfo->m_LastJetpackSound = 0;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK2, 1.0f, Position);
			}
		}
	}
	
	
	// chainsaw effects
	if (Player.m_Weapon == WEAPON_CHAINSAW && !Paused)
	{
		// chainsaw sound
		if (pCustomPlayerInfo->m_LastChainsawSoundTick < Client()->GameTick())
		{
			//m_pClient->m_pEffects->SmokeTrail(Position, vec2(0, 0));
			pCustomPlayerInfo->m_LastChainsawSoundTick = Client()->GameTick() + 500 * Client()->GameTickSpeed()/1000;
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_CHAINSAW_IDLE, 1.0f, Position);
		}
	}
	
	// body on flame
	int s = Player.m_Status;
	if (s & (1<<STATUS_AFLAME))
		m_pClient->m_pEffects->Flame(Position + vec2((frandom()-frandom())*8.0f, (frandom()-frandom())*14.0f - 4.0f), vec2(Vel.x*50, Vel.y*50 - 100), 0.7f);
	
	
	pCustomPlayerInfo->Animation()->m_FlipBody = false;
	
	// set animation
	if (Player.m_Anim == 1)
	{
		WantedAnimation = PANIM_WALL;
		pCustomPlayerInfo->Animation()->m_Flip = false;
	}
	
	if (Player.m_Anim == -1)
	{
		WantedAnimation = PANIM_WALL;
		pCustomPlayerInfo->Animation()->m_Flip = true;
	}
	
	
	// sliding animation
	if (Player.m_Slope != 0)
	{
		if ((Direction.x < 0.0f && Player.m_Slope < 0) || (Direction.x > 0.0f && Player.m_Slope > 0))
		{
			if (Player.m_Direction != Player.m_Slope && Player.m_Direction != 0)
			{
				WantedAnimation = PANIM_SLIDERUN_BACK;
				AnimationSpeed = 0.0275f;
			}
			else
			WantedAnimation = PANIM_SLIDE_FRONT;
		}
		else
		{
			if (Player.m_Direction != Player.m_Slope && Player.m_Direction != 0)
			{
				WantedAnimation = PANIM_SLIDERUN;
				AnimationSpeed = 0.0275f;
			}
			else
				WantedAnimation = PANIM_SLIDE_BACK;
		}
	}
	
	
	// roll animation
	if (Player.m_Anim == 2 && Player.m_Roll > 0)
	{
		WantedAnimation = PANIM_ROLL;
		pCustomPlayerInfo->Animation()->m_Flip = false;
		AnimationSpeed = 0.022f;
		
		//if (Direction.x < 0)
		//	pCustomPlayerInfo->Animation()->m_FlipBody = true;
	}
	
	if (Player.m_Anim == -2 && Player.m_Roll > 0)
	{
		WantedAnimation = PANIM_ROLL;
		pCustomPlayerInfo->Animation()->m_Flip = true;
		AnimationSpeed = 0.022f;
		
		//if (Direction.x > 0)
		//	pCustomPlayerInfo->Animation()->m_FlipBody = true;
	}
	
	
	// sliding
	if (Player.m_Anim == 3 && Player.m_Slide > 0)
	{
		if (Player.m_Slide > 4)
		{
			WantedAnimation = PANIM_SLIDE;
			//AnimationSpeed = 0.022f;
			AnimationSpeed = AnimSpeed;
		}
		else
		{
			WantedAnimation = PANIM_SLIDEDOWN;
			AnimationSpeed = 0.022f;
		}
		
		pCustomPlayerInfo->Animation()->m_Flip = false;
	}
	
	if (Player.m_Anim == -3 && Player.m_Slide > 0)
	{
		if (Player.m_Slide > 4)
		{
			WantedAnimation = PANIM_SLIDE;
			//AnimationSpeed = 0.022f;
			AnimationSpeed = AnimSpeed;
		}
		else
		{
			WantedAnimation = PANIM_SLIDEDOWN;
			AnimationSpeed = 0.022f;
		}
		
		pCustomPlayerInfo->Animation()->m_Flip = true;
	}
	
	
	
	if (Player.m_Anim == 4 && Player.m_Slide < 0)
	{
		WantedAnimation = PANIM_SLIDEUP;
		AnimationSpeed = 0.017f;
		//AnimationSpeed = AnimSpeed*0.3f;
		
		pCustomPlayerInfo->Animation()->m_Flip = false;
	}
	
	if (Player.m_Anim == -4 && Player.m_Slide < 0)
	{
		WantedAnimation = PANIM_SLIDEUP;
		AnimationSpeed = 0.017f;
		//AnimationSpeed = AnimSpeed*0.3f;
		
		pCustomPlayerInfo->Animation()->m_Flip = true;
	}
	
	/* ugly
	if (Player.m_Anim == 5)
	{
		WantedAnimation = PANIM_JUMPGROUND;
		AnimationSpeed = 0.025f;
	}
	*/
	
	if (Player.m_Anim == -5)
	{
		WantedAnimation = PANIM_JUMPSLIDE;
		AnimationSpeed = 0.025f;
	}
	

	
	
	
	
	// wallrun animations
	if (Player.m_Wallrun > 0)
	{
		WantedAnimation = PANIM_WALLRUN;
		AnimationSpeed = 0.02f;
		
		pCustomPlayerInfo->Animation()->m_Flip = true;
		
		if (Player.m_Wallrun > 10)
		{
			WantedAnimation = PANIM_WALLJUMP;
		}
	}
	else if (Player.m_Wallrun < 0)
	{
		WantedAnimation = PANIM_WALLRUN;
		AnimationSpeed = 0.02f;
		
		pCustomPlayerInfo->Animation()->m_Flip = false;
		
		if (Player.m_Wallrun < -10)
		{
			WantedAnimation = PANIM_WALLJUMP;
		}
	}
	
	// set the animation
	if (WantedAnimation >= 0)
		pCustomPlayerInfo->Animation()->SetAnimation(WantedAnimation, AnimationSpeed);
	
	
	s = Player.m_Status;
	if (s & (1<<STATUS_DEATHRAY))
	{
		if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY] <= 0.0f)
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ELECTRODEATH, 1.0f, Position);
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY] = 1.0f;
	}	
	
	s = Player.m_Status;
	if (s & (1<<STATUS_RAGE))
	{
		if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] <= 0.0f)
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ITEM_RAGE, 1.0f, Position);
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] = 1.0f;
	}	
	
	s = Player.m_Status;
	if (s & (1<<STATUS_FUEL))
	{
		if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL] <= 0.0f)
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ITEM_FUEL, 1.0f, Position);
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL] = 1.0f;
	}	
	
	s = Player.m_Status;
	if (s & (1<<STATUS_SPAWNING))
	{
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING] = 1.0f;
	}
	
	s = Player.m_Status;
	if (s & (1<<STATUS_ELECTRIC))
	{
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE] = 1.0f;
	}

	s = Player.m_Status;
	if (s & (1<<STATUS_INVISIBILITY))
	{
		if (!pCustomPlayerInfo->m_LoadInvisibility && pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY] <= 0.0f)
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ITEM_INVISIBILITY, 1.0f, Position);
		pCustomPlayerInfo->m_LoadInvisibility = true;
	}
	else
		pCustomPlayerInfo->m_LoadInvisibility = false;
	
	// custom stuff
	pCustomPlayerInfo->Update(Position);
	pCustomPlayerInfo->UpdatePhysics(vec2(Player.m_VelX, Player.m_VelY), vec2(Prev.m_VelX, Prev.m_VelY));
	
	// set correct shader
	if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY] > 0.0f)
		Graphics()->ShaderBegin(SHADER_DEATHRAY);
	else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE] > 0.0f)
		Graphics()->ShaderBegin(SHADER_ELECTRIC, pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE]);
	else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING] > 0.0f)
		Graphics()->ShaderBegin(SHADER_SPAWN, pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING]);
	else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE] > 0.0f)
		Graphics()->ShaderBegin(SHADER_DAMAGE, pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE]);
	else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY] > 0.0f)
		Graphics()->ShaderBegin(SHADER_INVISIBILITY, pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY]);
	else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] > 0.0f)
		Graphics()->ShaderBegin(SHADER_RAGE, pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE]);
	else if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL] > 0.0f)
		Graphics()->ShaderBegin(SHADER_FUEL, pCustomPlayerInfo->m_EffectIntensity[EFFECT_FUEL]);
	
	RenderTools()->RenderPlayer(&CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID], &RenderInfo, Player.m_Weapon, Player.m_Emote, Direction, Position);
	
	

	// iron man jetpack
	if (Player.m_HandJetpack)
	{
		vec2 HandPos = Position - Direction*10 + vec2(0, -10);
		
		RenderHand(&RenderInfo, HandPos, Direction, -pi/2, vec2(-8, 7));
		
		m_pClient->m_pEffects->Triangle(HandPos+vec2(0, +6), -Direction*600);
		m_pClient->m_pEffects->Triangle(HandPos+vec2(0, +6), -Direction*620);
		
		//m_pClient->m_pEffects->Triangle(Position + vec2(-4, 8), vec2(Vel.x*-20, 600));
		
		if (pCustomPlayerInfo->m_LastJetpackSoundTick <= Client()->GameTick())
		{
			pCustomPlayerInfo->m_LastJetpackSoundTick = Client()->GameTick() + 190 * Client()->GameTickSpeed()/1000;
			if (pCustomPlayerInfo->m_LastJetpackSound == 0)
			{
				pCustomPlayerInfo->m_LastJetpackSound = 1;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK1, 1.0f, Position);
			}
			else
			{
				pCustomPlayerInfo->m_LastJetpackSound = 0;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK2, 1.0f, Position);
			}
		}
	}
	
	Graphics()->ShaderEnd();
	
	
	
	
	s = Player.m_Status;
	if (s & (1<<STATUS_SHIELD))
	{
		RenderTools()->RenderShield(Position+vec2(0, -30), vec2(128, 128), pCustomPlayerInfo->m_EffectState);

		if (!pCustomPlayerInfo->m_Shield)
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ITEM_SHIELD, 1.0f, Position);
		
		pCustomPlayerInfo->m_Shield = true;
	}
	else
		pCustomPlayerInfo->m_Shield = false;

	
	
	s = Player.m_Status;
	if (s & (1<<STATUS_HEAL) && pCustomPlayerInfo->m_Heal == 0.0f)
	{
		m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_ITEM_HEAL, 1.0f, Position);
		pCustomPlayerInfo->m_Heal = 0.01f;
	}
	
	if (pCustomPlayerInfo->m_Heal > 0.0f && pCustomPlayerInfo->m_Heal < 1.0f)
		RenderTools()->RenderHeal(Position+vec2(0, -30), vec2(64, 128), pCustomPlayerInfo->m_Heal);
		
	

	
	
	m_pClient->m_pEffects->Light(Position+vec2(0, -32), 512+128);
	
	// electric sparks
	if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE] > 0.5f && Client()->GameTick()%5 == 1)
	{
		m_pClient->m_pEffects->Electrospark(Position+vec2((frandom()-frandom())*16.0f, -frandom()*48.0f), 30.0f);
	}
	
	if(Player.m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_DOTDOT);
		IGraphics::CQuadItem QuadItem(Position.x + 24, Position.y - 84, 64,64);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	if (m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart != -1 && m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart + 2 * Client()->GameTickSpeed() > Client()->GameTick())
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_Id);
		Graphics()->QuadsBegin();

		int SinceStart = Client()->GameTick() - m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart;
		int FromEnd = m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart + 2 * Client()->GameTickSpeed() - Client()->GameTick();

		float a = 1;

		if (FromEnd < Client()->GameTickSpeed() / 5)
			a = FromEnd / (Client()->GameTickSpeed() / 5.0);

		float h = 1;
		if (SinceStart < Client()->GameTickSpeed() / 10)
			h = SinceStart / (Client()->GameTickSpeed() / 10.0);

		float Wiggle = 0;
		if (SinceStart < Client()->GameTickSpeed() / 5)
			Wiggle = SinceStart / (Client()->GameTickSpeed() / 5.0);

		float WiggleAngle = sinf(5*Wiggle);

		Graphics()->QuadsSetRotation(pi/6*WiggleAngle);

		Graphics()->SetColor(1.0f,1.0f,1.0f,a);
		// client_datas::emoticon is an offset from the first emoticon
		RenderTools()->SelectSprite(SPRITE_OOP + m_pClient->m_aClients[pInfo.m_ClientID].m_Emoticon);
		IGraphics::CQuadItem QuadItem(Position.x, Position.y - 64 - 32*h, 64, 64*h);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
}


vec3 CPlayers::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}

void CPlayers::OnRender()
{
	const int aTeamColors[2] = {2555648, 8912640};
	const int aTeamFeetColors[2] = {65280, 10354432};
	
	// update RenderInfo for ninja
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		m_aRenderInfo[i] = m_pClient->m_aClients[i].m_RenderInfo;
		
		vec3 TeamColor = GetColorV3(aTeamColors[TEAM_RED]);
		
		// change to custom team colors
		/*
		if (g_Config.m_GoreCustomTeams)
		{
			if (m_aRenderInfo[i].m_ColorBody.r == TeamColor.r && m_aRenderInfo[i].m_ColorBody.g == TeamColor.g && m_aRenderInfo[i].m_ColorBody.b == TeamColor.b)
			{
				m_aRenderInfo[i].m_ColorBody.r = 217/255.0f;
				m_aRenderInfo[i].m_ColorBody.g = 140/255.0f;
				m_aRenderInfo[i].m_ColorBody.b = 65/255.0f;
				
				m_aRenderInfo[i].m_ColorFeet.r = 255/255.0f;
				m_aRenderInfo[i].m_ColorFeet.g = 140/255.0f;
				m_aRenderInfo[i].m_ColorFeet.b = 10/255.0f;
			}
			else
			{
				TeamColor = GetColorV3(aTeamColors[TEAM_BLUE]);
					
				if (m_aRenderInfo[i].m_ColorBody.r == TeamColor.r && m_aRenderInfo[i].m_ColorBody.g == TeamColor.g && m_aRenderInfo[i].m_ColorBody.b == TeamColor.b)
				{
					m_aRenderInfo[i].m_ColorBody.r = 100/255.0f;
					m_aRenderInfo[i].m_ColorBody.g = 140/255.0f;
					m_aRenderInfo[i].m_ColorBody.b = 100/255.0f;
					
					m_aRenderInfo[i].m_ColorFeet.r = 55/255.0f;
					m_aRenderInfo[i].m_ColorFeet.g = 155/255.0f;
					m_aRenderInfo[i].m_ColorFeet.b = 105/255.0f;
				}
			}
		}
		*/
	}

	// render other players in two passes, first pass we render the other, second pass we render our self
	for(int p = 0; p < 4; p++)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			// only render active characters
			if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
				continue;

			const void *pPrevInfo = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_PLAYERINFO, i);
			const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, i);

			if(pPrevInfo && pInfo)
			{
				//
				bool Local = ((const CNetObj_PlayerInfo *)pInfo)->m_Local !=0;
				if((p % 2) == 0 && Local) continue;
				if((p % 2) == 1 && !Local) continue;
				
				// send weapon info to custom stuff
				if (Local)
				{
					CustomStuff()->m_LocalWeapons = ((const CNetObj_PlayerInfo *)pInfo)->m_Weapons;
					CustomStuff()->m_LocalKits = ((const CNetObj_PlayerInfo *)pInfo)->m_Kits;
					CustomStuff()->m_aLocalItems[0] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item1;
					CustomStuff()->m_aLocalItems[1] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item2;
					CustomStuff()->m_aLocalItems[2] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item3;
					CustomStuff()->m_aLocalItems[3] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item4;
					CustomStuff()->m_aLocalItems[4] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item5;
					CustomStuff()->m_aLocalItems[5] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item6;
				}

				CNetObj_Character PrevChar = m_pClient->m_Snap.m_aCharacters[i].m_Prev;
				CNetObj_Character CurChar = m_pClient->m_Snap.m_aCharacters[i].m_Cur;

				if(p<2)
					;
				else
					RenderPlayer(
							&PrevChar,
							&CurChar,
							(const CNetObj_PlayerInfo *)pPrevInfo,
							(const CNetObj_PlayerInfo *)pInfo
						);
			}
		}
	}
}
