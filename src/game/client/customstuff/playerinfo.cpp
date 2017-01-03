#include "playerinfo.h"


#include <game/generated/client_data.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <game/client/render.h>
#include <engine/shared/config.h>




CPlayerInfo::CPlayerInfo()
{
	m_pAnimation = new CSkeletonAnimation();
	m_Pos = vec2(0, 0);
	m_Weapon = 0;
	m_AirJumpAnimLoaded = true;
	m_FlipFeet = false;
	m_WeaponRecoilLoaded = false;
	
	m_Color = vec4(1, 1, 1, 1);
	m_HideName = false;
	
	Reset();
}


CPlayerInfo *CPlayerInfo::GetIdle()
{
	static CPlayerInfo State;
	static bool Init = true;

	if(Init)
	{
		State.Reset();
		Init = false;
	}

	return &State;
}

void CPlayerInfo::Reset()
{
	m_UpdateTimer = 0;
	m_LastUpdate = 0;
	m_InUse = false;
	
	m_Shield = false;
	m_Heal = 0.0f;
	
	m_DamageTick = 0;
	
	m_LastJetpackSound = 0;
	m_LastJetpackSoundTick = 0;
	m_LastChainsawSoundTick = 0;
	
	m_ToolAngleOffset = 0;
	m_ToolAngleVel = 0;
	
	m_NextSplatter = 0;
	for (int i = 0; i < 8; i++)
	{
		m_aSplatter[i] = 0.0f;
	}
	
	// reset bounciness
	
	m_WeaponRecoil = vec2(0, 0);
	m_WeaponRecoilVel = vec2(0, 0);
	
	m_Weapon2Recoil = vec2(0, 0);
	m_Weapon2RecoilVel = vec2(0, 0);
	
	m_FeetPos = vec2(0, 0);
	m_FeetRecoil = vec2(0, 0);
	m_FeetRecoilVel = vec2(0, 0);
	
	m_BodyTilt = 0;
	
	m_EffectState = frandom();
	
	m_MeleeState = 0;
	m_MeleeTick = 0;
	m_MeleeAnimState = 0;
	
	m_Local = false;
	
	for (int i = 0; i < NUM_EFFECTS; i++)
		m_EffectIntensity[i] = 0.0f;
	
	m_LoadInvisibility = false;
	
	m_pAnimation->Reset();
}





void CPlayerInfo::Update(vec2 Pos)
{
	if (!m_InUse)
		Reset();
	
	m_InUse = true;
	m_UpdateTimer = 0;
	m_Pos = Pos;
}


void CPlayerInfo::SetLocal()
{
	if (!m_Local)
	{
		m_Local = true;
	}
}


void CPlayerInfo::AddSplatter()
{
	if (++m_NextSplatter > 7)
		m_NextSplatter = 0;
	
	m_aSplatter[m_NextSplatter] = 1.0f;
}




void CPlayerInfo::PhysicsTick(vec2 PlayerVel, vec2 PrevVel)
{
	if (Animation()->m_Flip)
	{
		Animation()->m_BodyTilt = -PlayerVel.x*0.00005f;
		Animation()->m_HeadTiltCorrect = +PlayerVel.x*0.00003f;
	}
	else
	{
		Animation()->m_BodyTilt = PlayerVel.x*0.00005f;
		Animation()->m_HeadTiltCorrect = -PlayerVel.x*0.00003f;
	}
	
	//float b = 1.5f - g_Config.m_GoreTeeBounciness / 100.0f;
	
	
	// weapon recoil
 	m_WeaponRecoilVel.x -= m_WeaponRecoil.x / 6.0f;
	m_WeaponRecoilVel.y -= m_WeaponRecoil.y / 6.0f;
	m_WeaponRecoilVel *= 0.82f;
			
	m_WeaponRecoil += m_WeaponRecoilVel;
	
	m_Weapon2RecoilVel.x += (PlayerVel.x-PrevVel.x)/2000.0f;
	m_Weapon2RecoilVel.y += (PlayerVel.y-PrevVel.y)/2000.0f;
		
	m_Weapon2RecoilVel.x -= m_Weapon2Recoil.x / 12.0f;
	m_Weapon2RecoilVel.y -= m_Weapon2Recoil.y / 12.0f;
		
	m_Weapon2RecoilVel *= 0.8f;

	m_Weapon2Recoil += m_Weapon2RecoilVel;
	
	m_ToolAngleOffset += m_ToolAngleVel;
	m_ToolAngleVel -= m_ToolAngleOffset / 8.0f;
	m_ToolAngleVel *= 0.8f;
	
	
	// inair feet physics
	m_FeetRecoilVel -= m_FeetRecoil / 14.0f;
	m_FeetRecoilVel *= 0.95f;
	
	m_FeetRecoil += m_FeetRecoilVel;
	
	m_FeetRecoilVel += (PlayerVel-PrevVel)/1200.0f;
	
	//Animation()->m_FeetDir = vec2(m_FeetRecoil.y, m_FeetRecoil.x);
	Animation()->m_FeetDir = m_FeetRecoil;
	Animation()->m_FeetAngle = 0.0f;
	
	Animation()->m_HeadOffset = vec2(m_FeetRecoil.y, m_FeetRecoil.x) * 0.6f;
}
	
	
void CPlayerInfo::UpdatePhysics(vec2 PlayerVel, vec2 PrevVel)
{
	int64 currentTime = time_get();
	if ((currentTime-m_LastUpdate > time_freq()) || (m_LastUpdate == 0))
		m_LastUpdate = currentTime;
		
	int step = time_freq()/60;
	
	if (step <= 0)
		step = 1;
	
	int i = 0;
	
	for (;m_LastUpdate < currentTime; m_LastUpdate += step)
	{
		PhysicsTick(PlayerVel, PrevVel);
		
		if (i++ > 20)
			break;
	}
}
	
void CPlayerInfo::Tick()
{
	for (int i = 0; i < 8; i++)
		m_aSplatter[i] = max(0.0f, m_aSplatter[i] - 0.001f);
	
	for (int i = 0; i < NUM_EFFECTS; i++)
	{
		if (m_EffectIntensity[i] > 0.0f)
		{
			m_EffectIntensity[i] -= 0.01f;
			
			if (i == EFFECT_SPAWNING)
				m_EffectIntensity[i] -= 0.01f;
			
			if (i == EFFECT_DAMAGE)
				m_EffectIntensity[i] -= 0.01f;
			
			if (i != EFFECT_INVISIBILITY && i != EFFECT_RAGE && i != EFFECT_FUEL)
				m_EffectIntensity[EFFECT_INVISIBILITY] -= 0.05f;
		}
		
		if (m_EffectIntensity[i] < 0.0f)
			m_EffectIntensity[i] = 0.0f;
	}
	
	if (m_LoadInvisibility)
	{
		m_EffectIntensity[EFFECT_INVISIBILITY] = min(1.0f, m_EffectIntensity[EFFECT_INVISIBILITY] + 0.03f);
	}
	
	if (m_Heal > 0.0f)
	{
		m_Heal += 0.015f;
		if (m_Heal >= 2.0f)
			m_Heal = 0.0f;
	}
	
	m_EffectState += 0.03f;
	if (m_EffectState >= 1.0f)
		m_EffectState -= 1.0f;
	
	if (m_UpdateTimer++ > 5)
	{
		m_InUse = false;
	}
	
	if (m_MeleeAnimState > 0.0f)
	{
		m_MeleeAnimState += 0.2f;
		if (int(m_MeleeAnimState) > 3)
			m_MeleeAnimState = 0.0f;
	}
	
	if (m_EffectIntensity[EFFECT_DEATHRAY] <= 0.0f)
		m_pAnimation->Tick();
}

