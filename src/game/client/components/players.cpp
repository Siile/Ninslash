

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
#include <game/client/components/tracer.h>
#include <game/client/components/sounds.h>
#include <game/client/components/controls.h>

#include "players.h"

void CPlayers::RenderHand(CTeeRenderInfo *pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset, vec2 PostRotOffset)
{
	// for drawing hand
	//const skin *s = skin_get(skin_id);

	float BaseSize = 16.0f;
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
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

	// two passes
	//for (int i = 0; i < 2; i++)
	{
		//bool OutLine = i == 0;

		//RenderTools()->SelectSprite(OutLine?SPRITE_TEE_HAND_OUTLINE:SPRITE_TEE_HAND, 0, 0, 0);
		RenderTools()->SelectSprite(SPRITE_HAND1_4+pInfo->m_Body*4);
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

	pCustomPlayerInfo->m_RenderInfo = RenderInfo;
	pCustomPlayerInfo->m_Weapon = Player.m_Weapon;
	
	
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
	
	pCustomPlayerInfo->m_Angle = Angle;
	
	// use preditect players if needed
	if(pInfo.m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(!m_pClient->m_Snap.m_pLocalCharacter || (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
		{
		}
		else
		{
			// m_PredictedChar.Write causes crash on some conditions when joining the game! todo: fix somehow 
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

	//RenderInfo.m_GotAirJump = Player.m_Jumped&2?0:1;

	// detect events
	if(NewTick)
	{
		// detect air jump
		//if(!RenderInfo.m_GotAirJump && !(Prev.m_Jumped&2))
		//	m_pClient->m_pEffects->AirJump(Position);
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

	pCustomPlayerInfo->UpdatePhysics(vec2(Player.m_VelX, Player.m_VelY), vec2(Prev.m_VelX, Prev.m_VelY));
	
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
		
	//static float s_LastGameTickTime = Client()->GameTickTime();
	if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
	{
		//s_LastGameTickTime = Client()->GameTickTime();
		Paused = false;
	}

	pCustomPlayerInfo->m_WeaponCharge = 0;
	
	// recoil & muzzle to static weapons
	if (IsStaticWeapon(Player.m_Weapon) || !Player.m_Weapon)
	{
		int Phase1Tick = (Client()->GameTick() - Player.m_AttackTick);
	
		//if (Phase1Tick < 10 && pCustomPlayerInfo->m_RecoilTick < Player.m_AttackTick && !Paused)
		if (Player.m_AttackTick && pCustomPlayerInfo->m_RecoilTick < Player.m_AttackTick && !Paused)
		{
			pCustomPlayerInfo->m_RecoilTick = Player.m_AttackTick;
			pCustomPlayerInfo->m_WeaponRecoil -= Direction * GetWeaponRenderRecoil(Player.m_Weapon);
			
			if (GetStaticType(Player.m_Weapon) == SW_GUN1 || GetStaticType(Player.m_Weapon) == SW_GUN2)
			{
				vec2 Moff = GetMuzzleRenderOffset(Player.m_Weapon);
				vec2 DirY(-pCustomPlayerInfo->m_MuzzleDir.y, pCustomPlayerInfo->m_MuzzleDir.x);
					
				if (pCustomPlayerInfo->m_MuzzleDir.x < 0)
					Moff.y *= -1;
					
				vec2 MuzzlePos = pCustomPlayerInfo->m_MuzzlePos + pCustomPlayerInfo->m_MuzzleDir * Moff.x + DirY * Moff.y;
				m_pClient->m_pEffects->Muzzle(MuzzlePos, pCustomPlayerInfo->m_MuzzleDir, Player.m_Weapon);
			}
			
			if (GetWeaponFiringType(Player.m_Weapon) == WFT_THROW)
			{
				
				
			}
		}
		
		if (Player.m_ChargeLevel > 0 && GetWeaponFiringType(Player.m_Weapon) == WFT_THROW)
			pCustomPlayerInfo->m_WeaponCharge = Phase1Tick;
			
	
		/*
		if (Player.m_AttackTick != pCustomPlayerInfo->m_MuzzleTick)
		{
			vec2 MuzzlePos = p + Dir * 20;
			m_pClient->m_pEffects->Muzzle(MuzzlePos, Dir, Player.m_Weapon);
			pCustomPlayerInfo->m_MuzzleTick = Player.m_AttackTick
		}
		*/
	
	
		// throwing
		//if (GetWeaponFiringType(Player.m_Weapon) == WFT_THROW)
		//	pCustomPlayerInfo->m_WeaponRecoil -= Direction * Player.m_ChargeLevel * 0.12f;
		
		//if (Phase1Tick < 10)
		//	pCustomPlayerInfo->AddMuzzle(Player.m_AttackTick, Player.m_Weapon);
	}
	
	if (GetWeaponFiringType(Player.m_Weapon) == WFT_MELEE)
	{
		// melee attack effect
		if (pCustomPlayerInfo->m_MeleeTick < Player.m_AttackTick && !Paused)
		{
			pCustomPlayerInfo->m_MeleeTick = Player.m_AttackTick;
			
			bool Flip = false;
			
			if (pCustomPlayerInfo->m_MeleeState == MELEE_UP)
			{
				if (Direction.x < 0.0f)
					Flip = !Flip;
				
				pCustomPlayerInfo->m_MeleeAnimState = 1.0f;
				pCustomPlayerInfo->m_MeleeState = MELEE_DOWN;
				m_pClient->m_pEffects->SwordHit(Position+vec2(0, -24)+Direction*60, GetAngle(Direction), Flip, Player.m_WeaponPowerLevel);
				pCustomPlayerInfo->m_WeaponRecoil += Direction * 15;
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*80, vec2(frandom()-frandom(), frandom()-frandom())*30);
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*95, vec2(frandom()-frandom(), frandom()-frandom())*30);
			}
			else
			{
				if (Direction.x < 0.0f)
					Flip = !Flip;
				
				pCustomPlayerInfo->m_MeleeAnimState = 1.0f;
				pCustomPlayerInfo->m_MeleeState = MELEE_UP;
				m_pClient->m_pEffects->SwordHit(Position+vec2(0, -24)+Direction*60, GetAngle(Direction), !Flip, Player.m_WeaponPowerLevel);
				pCustomPlayerInfo->m_WeaponRecoil += Direction * 15;
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*80, vec2(frandom()-frandom(), frandom()-frandom())*30);
				m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*95, vec2(frandom()-frandom(), frandom()-frandom())*30);
			}
			
			// impact to particles
			vec2 p = Position+vec2(0, -24)+Direction*70;
			float r = 48;

			CustomStuff()->AddImpact(vec4(p.x-r, p.y-r, p.x+r, p.y+r), CCustomStuff::IMPACT_HIT, normalize(p-Position+vec2(0, -0.3f)));
		}
	}
	/*
	else if (Player.m_Weapon == WEAPON_TOOL)
	{
		// melee attack effect
		if (pCustomPlayerInfo->m_MeleeTick < Player.m_AttackTick && !Paused)
		{
			pCustomPlayerInfo->m_MeleeTick = Player.m_AttackTick;
			pCustomPlayerInfo->m_ToolAngleOffset = 45.0f;
			pCustomPlayerInfo->m_WeaponRecoil += Direction * 15;
		}
	}
	else if (Player.m_Weapon == WEAPON_SCYTHE)
	{
		// melee attack effect
		if (pCustomPlayerInfo->m_MeleeTick < Player.m_AttackTick && !Paused)
		{
			pCustomPlayerInfo->m_MeleeTick = Player.m_AttackTick;
			pCustomPlayerInfo->FireMelee();
		}
	}
	*/
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
	
	
	// dash effect
	if (Player.m_Movement)
	{
		m_pClient->m_pTracers->Add(-2, pInfo.m_ClientID, Position+vec2(0, -6), Position+vec2(0, -6), 0, 0);
			
		if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] < 0.5f)
			m_pClient->m_pEffects->DashEffect(Position+vec2(0, -6), Player.m_Movement1>>6);
			
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_RAGE] = 1.0f;
	}
	else
		m_pClient->m_pTracers->UpdatePos(pInfo.m_ClientID, Position+vec2(0, -6));
	
	// store some data to customstuff for easy access
	if (pInfo.m_Local)
	{
		//if (rand()%10 == 1)
		//	m_pClient->m_pEffects->Guts(Position);

		CustomStuff()->m_LocalAlive = true;
		CustomStuff()->m_LocalColor = RenderInfo.m_ColorBody;
		CustomStuff()->m_LocalWeapon = Player.m_Weapon;
		CustomStuff()->m_LocalPos = Position;
		CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].SetLocal();
		
		int Group = m_pClient->m_pControls->m_InputData.m_WantedWeapon;
		
		if (Group > 0 && Group < 4)
			CustomStuff()->m_WantedWeapon = m_pClient->m_pControls->m_InputData.m_WantedWeapon;
		
		CustomStuff()->m_SelectedWeapon = Player.m_Weapon;
	}
	
	CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_Color = RenderInfo.m_ColorBody;
		
	// draw aim line 
	if (!CustomStuff()->m_Inventory && pPlayerInfo->m_Local && WeaponAimline(Player.m_Weapon) && pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING] < 0.9f)
	{
		//vec2 Pos = Position + Direction * 400; //Position + Direction * 500.0f;
		//vec2 Pos = Position + m_pClient->m_pControls->m_MousePos; //Position + Direction * 500.0f;
		vec2 Pos = Position + Direction * 1000.0f;
		vec2 From = Position + WeaponOffset;
		vec2 Dir = Direction;

		Collision()->IntersectLine(From, Pos, 0x0, &Pos);
		vec2 Out, Border;

		Graphics()->BlendNormal();
		Graphics()->TextureSet(-1);
		Graphics()->QuadsBegin();

		float a = 1.0f - pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY];
		
		// do outline
		//Graphics()->SetColor(0.2f, 1.0f, 0.2f, 0.3f*a);
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
	CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponPowerLevel = Player.m_WeaponPowerLevel;
	
	if (Player.m_DamageTick > pCustomPlayerInfo->m_DamageTick)
	{
		pCustomPlayerInfo->m_DamageTick = Player.m_DamageTick;
		pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE] = 1.0f;
	}
	
	float WeaponScale = 1.0f;
	
	float ChargeLevel = pCustomPlayerInfo->ChargeIntensity(Player.m_ChargeLevel);
	
	if (Player.m_ChargeLevel == 100)
	{
		if (!pCustomPlayerInfo->m_Charged)
		{
			pCustomPlayerInfo->m_Charged = true;
			//m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_CHARGE_FULL, 1.0f, Position);
		}
	}
	else if (Player.m_ChargeLevel < 0)
	{
		if (!pCustomPlayerInfo->m_ChargeFailed)
		{
			pCustomPlayerInfo->m_ChargeFailed = true;
			//m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_CHARGE_DOWN, 1.0f, Position);
		}
	}
	else
	{
		pCustomPlayerInfo->m_ChargeFailed = false;
		pCustomPlayerInfo->m_Charged = false;
	}
	
	// render chainsaw effect
	if (!Paused && GetStaticType(Player.m_Weapon) == SW_CHAINSAW && Player.m_AttackTick > Client()->GameTick() - 500 * Client()->GameTickSpeed()/1000)
	{
		WeaponScale = 1.07f;
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FX_CHAINSAW].m_Id);
		Graphics()->QuadsBegin();
		
		Graphics()->QuadsSetRotation(Angle);
		
		RenderTools()->SelectSprite(SPRITE_FX_CHAINSAW1+rand()%3, frandom()*10 < 5.0f ? SPRITE_FLAG_FLIP_Y : 0);
		
		vec2 p = Position + Direction * g_pData->m_Weapons.m_aId[WEAPON_CHAINSAW].m_Offsetx + CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoil + CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_Weapon2Recoil + WeaponOffset;

		p += Direction*14.0f;
		
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		
		float Size = 132;
		
		if (Player.m_WeaponPowerLevel > 1)
			Size *= 1.15f;
		
		//p.y += g_pData->m_Weapons.m_aId[WEAPON_CHAINSAW].m_Offsety;
		RenderTools()->DrawSprite(p.x, p.y, Size);
		
		Graphics()->QuadsEnd();
	}
	
	
	pCustomPlayerInfo->m_WeaponColorSwap = 0.0f;
	
	// render weapon
	if (GetWeaponRenderType(Player.m_Weapon) == WRT_WEAPON1)
	{
		//pCustomPlayerInfo->m_WeaponColorSwap = GetWeaponColorswap(Player.m_Weapon);
		RenderTools()->SetShadersForWeapon(pCustomPlayerInfo);
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(Angle);

		// normal weapons
		//int iw = clamp(Player.m_Weapon, 0, NUM_WEAPONS-1);
		//RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);

		vec2 Dir = Direction;
		vec2 p;

		{
			static float s_LastIntraTick = IntraTick;
			if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
				s_LastIntraTick = IntraTick;

			float a = (Client()->GameTick()-Player.m_AttackTick+s_LastIntraTick)/5.0f;
			
			
			// recoil to modular
			if (Player.m_AttackTick && pCustomPlayerInfo->m_RecoilTick < Player.m_AttackTick && !Paused)
			{
				pCustomPlayerInfo->m_RecoilTick = Player.m_AttackTick;
				pCustomPlayerInfo->m_WeaponRecoil -= Direction * GetWeaponRenderRecoil(Player.m_Weapon);
			}
			
			
			
			if(a < 1)
			{

				if (CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilLoaded)
				{
					CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilLoaded = false;
					
					if (GetStaticType(Player.m_Weapon) == SW_CHAINSAW)
					{
						CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilVel -= Direction * 1.0f;
						pCustomPlayerInfo->m_LastChainsawSoundTick = Client()->GameTick() + 500 * Client()->GameTickSpeed()/1000;
						
						
						m_pClient->AddFluidForce(Position+vec2(0, -24)+Direction*80, vec2(frandom()-frandom(), frandom()-frandom())*30);
					}
					//else if (Player.m_Weapon != WEAPON_FLAMER)
					//	CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilVel -= Direction * 12.0f;
				}
			}
			else
			{
				CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilLoaded = true;
			}
			
			vec2 Offset = GetWeaponRenderOffset(Player.m_Weapon);
			
			p = Position + Dir * Offset.x + CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoil + CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_Weapon2Recoil + WeaponOffset;
			
			
			// chainsaw shaking
			if (!Paused && GetStaticType(Player.m_Weapon) == SW_CHAINSAW && Player.m_AttackTick > Client()->GameTick() - 500 * Client()->GameTickSpeed()/1000)
			{
				p = p + vec2(frandom()-frandom(), frandom()-frandom()) * 4.0f;
				
				p += Dir*6.0f;
				
				Graphics()->QuadsSetRotation(Angle+(frandom()-frandom())*0.1f);
				
				// smoke
				m_pClient->m_pEffects->ChainsawSmoke(p);
					
				// sparks to walls
				vec2 To = p + Dir * 32;
				
				if (Collision()->IntersectLine(p, To, 0x0, &To))
				{
					m_pClient->m_pEffects->Spark(To);
					m_pClient->m_pEffects->Spark(To);
				}
			}
			
			
			{
				p.y += Offset.y;
				RenderTools()->RenderWeapon(Player.m_Weapon, p, Dir, WEAPON_GAME_SIZE);
				
				//RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize*WeaponScale);
				
				Graphics()->QuadsEnd();
				RenderHand(&RenderInfo, p, Direction, -1.8f*pi/4, vec2(-26, 8));
				
				// muzzle
				
				if (Player.m_AttackTick && Player.m_AttackTick != pCustomPlayerInfo->m_MuzzleTick)
				{
					if (GetWeaponFiringType(Player.m_Weapon) != WFT_HOLD)
					{		
						vec2 Moff = GetMuzzleRenderOffset(Player.m_Weapon);
						vec2 DirY(-Dir.y,Dir.x);
						vec2 MuzzlePos = p + Dir * Moff.x + DirY * Moff.y;
						m_pClient->m_pEffects->Muzzle(MuzzlePos, Dir, Player.m_Weapon);
					}
				}
				
				
				if (GetWeaponFiringType(Player.m_Weapon) != WFT_HOLD)
				{
					pCustomPlayerInfo->AddMuzzle(Player.m_AttackTick, Player.m_Weapon);
				
					// render muzzles
					for (int i = 0; i < 4; i++)
					{
						if (pCustomPlayerInfo->m_aMuzzleWeapon[i])
						{
							Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MUZZLE].m_Id);
							Graphics()->QuadsBegin();
							Graphics()->QuadsSetRotation(Angle);
								
							vec2 Moff = GetMuzzleRenderOffset(Player.m_Weapon);
							
							if (Dir.x < 0)
								Moff.y *= -1;
					
							RenderTools()->SelectSprite(SPRITE_MUZZLE1_1 + pCustomPlayerInfo->m_aMuzzleType[i]*4 + pCustomPlayerInfo->m_aMuzzleTime[i]*4, SPRITE_FLAG_FLIP_X);

							vec2 DirY(-Dir.y,Dir.x);
							vec2 MuzzlePos = p + Dir * Moff.x + DirY * Moff.y;

							RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, 60);
							
							Graphics()->QuadsEnd();	
						}
					}
				}
				else
					pCustomPlayerInfo->m_MuzzleTick = Player.m_AttackTick;
				
					
				
				
				/*
				
				if (GetWeaponFiringType(Player.m_Weapon) != WFT_HOLD)
				{
					//float Alpha = 0.0f;
					//int Phase1Tick = (Client()->GameTick() - Player.m_AttackTick);
					
					//if (Phase1Tick < 10)
					{
						if (Player.m_AttackTick && Player.m_AttackTick != pCustomPlayerInfo->m_MuzzleTick)
						{
							vec2 Moff = GetMuzzleRenderOffset(Player.m_Weapon);
							vec2 DirY(-Dir.y,Dir.x);
							vec2 MuzzlePos = p + Dir * Moff.x + DirY * Moff.y;
							m_pClient->m_pEffects->Muzzle(MuzzlePos, Dir, Player.m_Weapon);
						}
						
						pCustomPlayerInfo->AddMuzzle(Player.m_AttackTick, Player.m_Weapon);

						pCustomPlayerInfo->m_MuzzleTick = Player.m_AttackTick;
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
							
							if (Dir.x < 0)
								Moff.y *= -1;
				
							RenderTools()->SelectSprite(SPRITE_MUZZLE1_1 + pCustomPlayerInfo->m_aMuzzleType[i]*4 + pCustomPlayerInfo->m_aMuzzleTime[i]*4, SPRITE_FLAG_FLIP_X);

							vec2 DirY(-Dir.y,Dir.x);
							vec2 MuzzlePos = p + Dir * Moff.x + DirY * Moff.y;

							RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, 60);
							
							Graphics()->QuadsEnd();	
						}
					}
				}
				*/
			}
		}
		
		// flamer
		if (GetStaticType(Player.m_Weapon) == SW_FLAMER)
		{
			// roll the animation
			if (Player.m_AttackTick > Client()->GameTick() - 220 * Client()->GameTickSpeed()/1000)
			{
				if (pCustomPlayerInfo->m_FlameState == 0)
					pCustomPlayerInfo->m_FlameState++;
				
				if (pCustomPlayerInfo->m_FlameState > 9*4)
					pCustomPlayerInfo->m_FlameState = 5*4;
				
				
				CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID].m_WeaponRecoilVel -= Direction * 0.8f;
			}
			else
			{
				if (pCustomPlayerInfo->m_FlameState > 0 && pCustomPlayerInfo->m_FlameState < 9*4)
					pCustomPlayerInfo->m_FlameState = 9*4;
			}
			
			// render the flame
			if (pCustomPlayerInfo->m_FlameState > 0)
			{
				int f = pCustomPlayerInfo->m_FlameState / 4;
				
				Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FLAME].m_Id);
				Graphics()->QuadsBegin();
				
				bool Flip = Dir.x < 0 ? true : false;
				
				Graphics()->SetColor(1, 1, 1, 1);
				Graphics()->QuadsSetRotation(0);
					
				vec2 DirY(-Dir.y,Dir.x);
				float OffsetY = -20 * (Dir.x < 0 ? -1 : 1);

				
				int Slices = 9;

				
				vec2 PrevPos = vec2(0, 0);
				float PrevA = 0.0f;
				
				vec2 FPos = p + DirY * OffsetY;
				FPos += vec2(cos(Angle), sin(Angle))*16.0f;
				
				vec2 StartPos = FPos;
				
				int Flames = 0;
				
				for (int i = 0; i <= Slices; i++)
				{
					float fa = pCustomPlayerInfo->m_aFlameAngle[i];
					
					float x1 = (i-1.0f) / float(Slices);
					float x2 = (i+0.0f) / float(Slices);
					if (i > 0)
						RenderTools()->SelectSprite(SPRITE_FLAME1+f,  + (Flip ? SPRITE_FLAG_FLIP_Y : 0), 0, 0, x1, x2);
					
					FPos += vec2(cos(fa), sin(fa)) * (240.0f / Slices);
					
					if (PrevPos.x == 0.0f)
					{
						PrevPos = FPos;
						PrevA = fa;
						continue;
					}
				
					float a1 = PrevA-pi/2.0f;
					float a2 = fa-pi/2.0f;
					float a3 = PrevA+pi/2.0f;
					float a4 = fa+pi/2.0f;
					
					float s1 = 32.0f;
					
					vec2 p1 = PrevPos+vec2(cos(a1), sin(a1))*s1;
					vec2 p2 = FPos+vec2(cos(a2), sin(a2))*s1;
					vec2 p3 = PrevPos+vec2(cos(a3), sin(a3))*s1;
					vec2 p4 = FPos+vec2(cos(a4), sin(a4))*s1;
					
					if (!Flames && Collision()->IntersectLine(StartPos, PrevPos, 0x0, 0x0))
					{
						Flames++;
						m_pClient->m_pEffects->Flame(PrevPos + vec2(frandom()-frandom(), frandom()-frandom()) * 12.0f, vec2(frandom()-frandom(), frandom()-frandom()) * 200.0f, 0.5f, true);
					}
					
					// prev
					Collision()->IntersectLine(StartPos, p1, 0x0, &p1);
					Collision()->IntersectLine(StartPos, p3, 0x0, &p3);
					Collision()->IntersectLine(StartPos, p2, 0x0, &p2);
					Collision()->IntersectLine(StartPos, p4, 0x0, &p4);
					
					IGraphics::CFreeformItem FreeFormItem(
						p1.x, p1.y,
						p2.x, p2.y,
						p3.x, p3.y,
						p4.x, p4.y);
						
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					
					PrevPos = FPos;
					PrevA = fa;
				}
				
				
				Graphics()->QuadsEnd();
				
				OffsetY = -0 * (Dir.x < 0 ? -1 : 1);
				//FPos = p + Dir * 52 + DirY * OffsetY;
				
				if (Player.m_AttackTick > Client()->GameTick() - 240 * Client()->GameTickSpeed()/1000)
				{
					//m_pClient->m_pEffects->Flame(FPos, Dir * (900.0f+frandom()*500.0f), 0.5f);
					
					FPos = p + Dir * 40 + DirY * OffsetY;
						
					if (frandom()*10 < 4 && !Collision()->CheckPoint(FPos.x, FPos.y))
					{
						m_pClient->m_pEffects->SmokeTrail(FPos, Dir * (600.0f+frandom()*500.0f));
					}
				}
			}
		}
		
		
		// render hand
		
		/*
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
		Graphics()->QuadsBegin();
		//Graphics()->QuadsSetRotation(State.GetAttach()->m_Angle*pi*2+Angle);
		Graphics()->QuadsSetRotation(Angle);
		
		Graphics()->ShaderEnd();
		
		// muzzle
		if (Player.m_Weapon == WEAPON_RIFLE || Player.m_Weapon == WEAPON_SHOTGUN)
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
		
		
		// flamethrower
		
					
		RenderTools()->SetShadersForWeapon(pCustomPlayerInfo);
			
		switch (Player.m_Weapon)
		{
			//case WEAPON_GUN: RenderHand(&RenderInfo, p, Direction, -3*pi/4, vec2(-15, 4)); break;
			case WEAPON_SHOTGUN: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 4)); break;
			case WEAPON_GRENADE: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_RIFLE: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_ELECTRIC: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 10)); break;
			case WEAPON_LASER: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 7)); break;
			case WEAPON_FLAMER: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-10, 7)); break;
			case WEAPON_CHAINSAW:
				RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-8, 10));
				//RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-42, 4));
				break;
		}
		*/

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
		/*
		m_pClient->m_pEffects->Triangle(Position + vec2(4, 8), vec2(Vel.x*-20, 600));
		m_pClient->m_pEffects->Triangle(Position + vec2(-4, 8), vec2(Vel.x*-20, 600));
		*/
		
		pCustomPlayerInfo->m_Jetpack = 1;
		
		if (!Paused && pCustomPlayerInfo->m_LastJetpackSoundTick <= Client()->GameTick())
		{
			pCustomPlayerInfo->m_LastJetpackSoundTick = Client()->GameTick() + 190 * Client()->GameTickSpeed()/1000;
			
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK1+pCustomPlayerInfo->m_LastJetpackSound, 1.0f, Position);
				
			if (++pCustomPlayerInfo->m_LastJetpackSound > 3)
				pCustomPlayerInfo->m_LastJetpackSound = 0;
				
			/*
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
			*/
		}
	}
	else
		pCustomPlayerInfo->m_Jetpack = 0;
	
	
	// chainsaw sound
	if (GetStaticType(Player.m_Weapon) == SW_CHAINSAW && !Paused)
	{
		if (pCustomPlayerInfo->m_LastChainsawSoundTick < Client()->GameTick())
		{
			pCustomPlayerInfo->m_LastChainsawSoundTick = Client()->GameTick() + 500 * Client()->GameTickSpeed()/1000;
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_CHAINSAW_IDLE, 1.0f, Position);
		}
	}
	
	// scythe sound & impact to particles
	if (Player.m_Weapon == WEAPON_SCYTHE && !Paused)
	{
		if (pCustomPlayerInfo->MeleeSound())
		{
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_SCYTHE_FIRE, 1.0f, Position);
		}
	
		vec2 p = Position + pCustomPlayerInfo->MeleeOffset();
		float r = 50.0f;
		if (pCustomPlayerInfo->MeleeImpact() > 0)
		{
			CustomStuff()->AddImpact(vec4(p.x-r, p.y-r, p.x, p.y), CCustomStuff::IMPACT_SCYTHE, vec2(0.4f, -1.5f));
			CustomStuff()->AddImpact(vec4(p.x, p.y-r, p.x+r, p.y), CCustomStuff::IMPACT_SCYTHE, vec2(0.7f, -1.2f));
			CustomStuff()->AddImpact(vec4(p.x-r*0.8f, p.y, p.x+r*0.8f, p.y+r), CCustomStuff::IMPACT_SCYTHE, vec2(-0.4f, -1.0f));
		}
		else if (pCustomPlayerInfo->MeleeImpact() < 0)
		{
			CustomStuff()->AddImpact(vec4(p.x-r, p.y-r, p.x, p.y), CCustomStuff::IMPACT_SCYTHE, vec2(-0.4f, -1.5f));
			CustomStuff()->AddImpact(vec4(p.x, p.y-r, p.x+r, p.y), CCustomStuff::IMPACT_SCYTHE, vec2(-0.7f, -1.2f));
			CustomStuff()->AddImpact(vec4(p.x-r*0.8f, p.y, p.x+r*0.8f, p.y+r), CCustomStuff::IMPACT_SCYTHE, vec2(0.4f, -1.0f));
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
		//AnimationSpeed = 0.022f;
		AnimationSpeed = 0.06f;
		
		//if (Direction.x < 0)
		//	pCustomPlayerInfo->Animation()->m_FlipBody = true;
	}
	
	if (Player.m_Anim == -2 && Player.m_Roll > 0)
	{
		WantedAnimation = PANIM_ROLL;
		pCustomPlayerInfo->Animation()->m_Flip = true;
		//AnimationSpeed = 0.022f;
		AnimationSpeed = 0.06f;
		
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
	
	if (Player.m_Action == COREACTION_HANG)
		pCustomPlayerInfo->m_Hang = true;
	else
		pCustomPlayerInfo->m_Hang = false;
	
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
	
	if (Player.m_Anim == 6)
	{
		WantedAnimation = PANIM_JUMPPAD;
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
	

	if (Player.m_Anim == 7 || Player.m_Anim == -7)
	{
		WantedAnimation = PANIM_SLIDEKICK;
		AnimationSpeed = 0.03f;
		
		if (pCustomPlayerInfo->Animation()->m_Time < 0.04f)
		{
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_KICK, 1.0f, Position);
			//m_pClient->m_pEffects->Electrospark(Position+vec2(0, 18), 48);
			//m_pClient->m_pEffects->SpriteSheet(FX_FLAME1, Position+vec2(Player.m_Anim * 4, -18));
		}
		
			//m_pClient->m_pEffects->Electrospark(Position, 48);
		
		if (Player.m_Anim == 7)
			pCustomPlayerInfo->Animation()->m_Flip = false;
		else
			pCustomPlayerInfo->Animation()->m_Flip = true;
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
	//pCustomPlayerInfo->UpdatePhysics(vec2(Player.m_VelX, Player.m_VelY), vec2(Prev.m_VelX, Prev.m_VelY));
	
	// set correct shader
	RenderTools()->SetShadersForPlayer(pCustomPlayerInfo);
	RenderTools()->RenderPlayer(&CustomStuff()->m_aPlayerInfo[pInfo.m_ClientID], &RenderInfo, Player.m_Weapon, Player.m_Emote, Direction, Position);
	
	
	// iron man jetpack
	if (Player.m_HandJetpack)
	{
		// sound
		if (!Paused && pCustomPlayerInfo->m_LastTurboSoundTick <= Client()->GameTick())
		{
			pCustomPlayerInfo->m_LastTurboSoundTick = Client()->GameTick() + 190 * Client()->GameTickSpeed()/1000;
			
			m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_TURBO1+pCustomPlayerInfo->m_LastTurboSound, 1.0f, Position);
				
			if (++pCustomPlayerInfo->m_LastTurboSound > 3)
				pCustomPlayerInfo->m_LastTurboSound = 0;
			
			/*
			if (pCustomPlayerInfo->m_LastTurboSound == 0)
			{
				pCustomPlayerInfo->m_LastJetpackSound = 1;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK1, 1.0f, Position);
			}
			else
			{
				pCustomPlayerInfo->m_LastJetpackSound = 0;
				m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_JETPACK2, 1.0f, Position);
			}
			*/
		}
		
		// info to physics
		pCustomPlayerInfo->m_Turbo = true;
	}
	else
		pCustomPlayerInfo->m_Turbo = false;
	
	
/*
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
*/	

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
		
	
	//if (pInfo.m_Local)
	//	m_pClient->m_pEffects->Light(Position+vec2(0, -32), 512+128);
	
	// electric sparks
	if (pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE] > 0.5f && Client()->GameTick()%5 == 1)
		m_pClient->m_pEffects->Electrospark(Position+vec2((frandom()-frandom())*16.0f, -frandom()*48.0f), 30.0f);
	
	
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
	CustomStuff()->ClearImpacts();
	
	//const int aTeamColors[2] = {2555648, 8912640};
	//const int aTeamFeetColors[2] = {65280, 10354432};
	
	// update RenderInfo for ninja
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		m_aRenderInfo[i] = m_pClient->m_aClients[i].m_RenderInfo;
	}

	CustomStuff()->m_LocalAlive = false;

		
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
					// HACK: delay changing m_LocalWeapons for one frame, because pInfo is outdated compared to NETMSGTYPE_SV_WEAPONPICKUP data
					// m_LocalWeapons will be almost always up to date, except when the server forces new weapon list to the player
					static int LocalWeaponsChangeCounter = 0;
					int NewWeapons = ((const CNetObj_PlayerInfo *)pInfo)->m_Weapons;
					if (CustomStuff()->m_LocalWeapons != NewWeapons)
					{
						LocalWeaponsChangeCounter++;
						if (LocalWeaponsChangeCounter > 2)
							CustomStuff()->m_LocalWeapons = NewWeapons;
					}
					else
					{
						LocalWeaponsChangeCounter = 0;
					}
					CustomStuff()->m_LocalUpgrades = ((const CNetObj_PlayerInfo *)pInfo)->m_Upgrades;
					CustomStuff()->m_LocalUpgrades2 = ((const CNetObj_PlayerInfo *)pInfo)->m_Upgrades2;
					CustomStuff()->m_LocalKits = ((const CNetObj_PlayerInfo *)pInfo)->m_Kits;
					CustomStuff()->m_aLocalItems[0] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item1;
					CustomStuff()->m_aLocalItems[1] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item2;
					CustomStuff()->m_aLocalItems[2] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item3;
					CustomStuff()->m_aLocalItems[3] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item4;
					CustomStuff()->m_aLocalItems[4] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item5;
					CustomStuff()->m_aLocalItems[5] = ((const CNetObj_PlayerInfo *)pInfo)->m_Item6;
					
					CustomStuff()->m_WeaponSlot = ((const CNetObj_PlayerInfo *)pInfo)->m_WeaponSlot;
					
					CustomStuff()->m_aSnapWeapon[0] = ((const CNetObj_PlayerInfo *)pInfo)->m_Weapon1;
					CustomStuff()->m_aSnapWeapon[1] = ((const CNetObj_PlayerInfo *)pInfo)->m_Weapon2;
					CustomStuff()->m_aSnapWeapon[2] = ((const CNetObj_PlayerInfo *)pInfo)->m_Weapon3;
					CustomStuff()->m_aSnapWeapon[3] = ((const CNetObj_PlayerInfo *)pInfo)->m_Weapon4;
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
