#include "playerinfo.h"


#include <game/generated/client_data.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <game/weapons.h>
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
	m_Charged = true;
	m_ChargeFailed = false;
	
	m_FGHandPos = vec2(0, 0);
	m_Angle = 0;
	m_Turbo = false;
	m_Jetpack = false;
	m_Hang = false;
	m_SpinningAngle = 0.0f;
	
	m_RecoilTick = 0;
	
	m_WeaponColorSwap = 0.0f;
	
	m_ArmPos = vec2(0, 0);
	
	for (int i = 0; i < 20; i++)
		m_aFlameAngle[i] = 0.0f;
	
	m_FlameState = 0;
	m_UpdateTimer = 0;
	m_LastUpdate = 0;
	m_InUse = false;
	
	m_Shield = false;
	m_Heal = 0.0f;
	
	m_DamageTick = 0;
	
	m_LastJetpackSound = 0;
	m_LastJetpackSoundTick = 0;
	m_LastTurboSound = 0;
	m_LastTurboSoundTick = 0;
	m_LastChainsawSoundTick = 0;
	
	m_ToolAngleOffset = 0;
	m_ToolAngleVel = 0;
	
	m_NextSplatter = 0;
	for (int i = 0; i < 8; i++)
	{
		m_aSplatter[i] = 0.0f;
		m_aSplatterColor[i] = vec4();
	}
	
	for (int i = 0; i < 4; i++)
	{
		m_aMuzzleTime[i] = 0.0f;
		m_aMuzzleWeapon[i] = 0;
		m_aMuzzleType[i] = 0;
	}
	
	m_MuzzleTick = 0;
	m_WeaponCharge = 0;
	
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
	
	m_MuzzlePos = vec2(0, 0);
	m_MuzzleDir = vec2(0, 0);
	
	m_Local = false;
	
	for (int i = 0; i < NUM_EFFECTS; i++)
		m_EffectIntensity[i] = 0.0f;
	
	m_LoadInvisibility = false;
	
	// weapon charge
	m_ChargeAngle = 0.0f;
	m_Charge = 0;
	
	m_pAnimation->Reset();
	m_Melee.Reset();
	m_Hand[HAND_WEAPON].Reset();
	m_Hand[HAND_FREE].Reset();
}

void CPlayerInfo::AddMuzzle(int AttackTick, int Weapon)
{
	if (m_MuzzleTick == AttackTick || !AttackTick)
		return;
	
	m_MuzzleTick = AttackTick;
	
	for (int i = 0; i < 4; i++)
	{
		if (!m_aMuzzleWeapon[i])
		{
			m_aMuzzleTime[i] = 0.0f;
			m_aMuzzleWeapon[i] = Weapon;
			//m_aMuzzleType[i] = rand()%4+GetMuzzleType(Weapon)*4;
			m_aMuzzleType[i] = rand()%4;
			return;
		}
	}
}

	
float CPlayerInfo::GetWeaponCharge()
{
	float ChargeLevel = min(m_WeaponCharge*0.013f, 1.0f);
	
	if (ChargeLevel == 1.0f)
		ChargeLevel = 0.7f+cos(m_WeaponCharge*0.4f)*0.3f;
	
	return ChargeLevel;
	
	/*
	if (m_WeaponCharge < 100)
		return m_WeaponCharge*0.01f;
		
	return 0.7f+cos(m_WeaponCharge*0.1f)*0.3f;
	*/
}
	
float CPlayerInfo::ChargeIntensity(int Charge)
{
	m_Charge = Charge;
	
	if (Charge < 100)
	{
		m_ChargeAngle = 0;
		
		if (Charge < 0)
			return -Charge*0.025f;
			
		return Charge*0.01f;
	}

	
	return (0.8f+cos(m_ChargeAngle*3.0f)*0.15f)*min(1.0f, Charge*0.01f);
}

float CPlayerInfo::ChargeIntensity()
{
	return ChargeIntensity(m_Charge);
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


void CPlayerInfo::AddSplatter(vec4 Color)
{
	if (++m_NextSplatter > 7)
		m_NextSplatter = 0;
	
	Color.b *= 0.2f;
	
	m_aSplatter[m_NextSplatter] = 1.0f;
	m_aSplatterColor[m_NextSplatter] = Color;
}

static float CurveAngle(float From, float To, float Amount)
{
	while (From - To > pi)
		From -= 2*pi;
	
	while (To - From > pi)
		To -= 2*pi;
	
	return From + (To-From) / Amount;
}



float CPlayerInfo::MeleeAngle()
{
	return m_Melee.m_Angle;
}

float CPlayerInfo::MeleeSize()
{
	/*
	return 1.1f + m_WeaponPowerLevel*0.2f;
	
	float s = abs(m_Melee.m_TurnSpeed);
	return 0.8f + (s > 0.2f ? s - 0.2f : 0.0f);
	*/
	
	return 1.0f;
}

bool CPlayerInfo::MeleeFlip()
{
	if (m_Melee.m_Flip)
		return ((m_Melee.m_TurnSpeed < 0.0f) ? false : true);
	
	return ((m_Melee.m_TurnSpeed < 0.0f) ? true : false);
}

bool CPlayerInfo::MeleeSound()
{
	if (abs(m_Melee.m_TurnSpeed) < 0.3f)
		return false;
	
	if (m_Melee.m_EffectFrame != 3 && m_Melee.m_EffectFrame != 9)
		return false;
	
	return true;
}

bool CPlayerInfo::MeleeEffectFlip()
{
	//if (m_Melee.m_Flip)
	//	return ((m_Melee.m_TurnSpeed < 0.0f) ? false : true);
	
	return ((m_Melee.m_TurnSpeed < 0.0f) ? true : false);
}

vec2 CPlayerInfo::MeleeOffset()
{
	vec2 p = vec2(0, -12);
	
	p.x += (MeleeFlip() ? -1 : 1)*(10.0f+((m_Melee.m_FireTimer > 0) ? 6 : 0));
	p.x += sin(m_Melee.m_Angle*1.0f)*8.0f*(MeleeFlip() ? 1 : -1);
	//p.x *= m_Melee.m_Flip ? -1.0f : 1.0f;
	
	p.y += (m_Melee.m_FireTimer > 0) ? -4 : 0;
	p.y += cos(m_Melee.m_Angle)*4.0f*(MeleeFlip() ? -1.0f : 1.0f);
	
	return p;
}

vec2 CPlayerInfo::MeleeEffectOffset()
{
	vec2 p = vec2(0, -15);
	
	p.x += (MeleeFlip() ? -1 : 1)*(11.0f+((m_Melee.m_FireTimer > 0) ? 6 : 0));
	p.y += (m_Melee.m_FireTimer > 0) ? -4 : 0;
	
	return p;
}

bool CPlayerInfo::MeleeFront()
{
	return m_Melee.m_Front;
}

int CPlayerInfo::MeleeFrame()
{
	return min(int(abs(m_Melee.m_TurnSpeed*7)), 3);
	
	if (abs(m_Melee.m_TurnSpeed) > 0.1f)
		return 1;
	
	return 0;
}

int CPlayerInfo::MeleeEffectFrame()
{
	if (!m_Melee.m_EffectFrame)
		return -1;
	
	return m_Melee.m_EffectFrame / 3;
}

int CPlayerInfo::MeleeImpact()
{
	if (m_Melee.m_TurnSpeed > 0.3f)
		return 1;
	else if (m_Melee.m_TurnSpeed < -0.3f)
		return -1;
	
	return 0;
}


void CPlayerInfo::FireMelee()
{
	m_Melee.m_FireTimer = 6;
	
	if (m_Melee.m_EffectFrame == 0)
		m_Melee.m_EffectFrame++;
				
	if (m_Melee.m_EffectFrame > 5*3)
		m_Melee.m_EffectFrame = 1*3;
}


vec2 CPlayerInfo::HandOffset(int Hand)
{
	vec2 p = m_Hand[Hand].m_Pos + m_Hand[Hand].m_Offset + vec2(0, 16);
	
	return p;
}

void CPlayerInfo::SetHandTarget(int Hand, vec3 Pos)
{
	if (Hand == HAND_FREE)
	{
		if (!m_Turbo)
			m_Hand[Hand].m_TargetPos = vec2(Pos.x, Pos.y);
		
		if (m_Hang)
			m_Hand[Hand].m_TargetPos = vec2(0, -46);
	}
	else if (GetWeaponRenderType(m_Weapon) != WRT_WEAPON2 && GetWeaponRenderType(m_Weapon) != WRT_ITEM1)
		m_Hand[Hand].m_TargetPos = (m_Hand[Hand].m_TargetPos + vec2(Pos.x, Pos.y)) / 2.0f;
		//m_Hand[Hand].m_TargetPos = vec2(Pos.x, Pos.y);
}


int CPlayerInfo::HandFrame(int Hand)
{
	if (m_Turbo && Hand == HAND_FREE)
		return 2;
	
	return 0;
}


void CPlayerInfo::PhysicsTick(vec2 PlayerVel, vec2 PrevVel)
{
	// weapon charge
	if (m_Charge > 0)
		m_ChargeAngle += 0.1f;
	
	
	// hands
	for (int i = 0; i < 2; i++)
	{
		m_Hand[i].m_Pos += (m_Hand[i].m_TargetPos - m_Hand[i].m_Pos) / 3.0f;
		
		if (m_Turbo && i == HAND_FREE)
		{
			m_Hand[i].m_TargetPos = -vec2(cos(m_Angle), sin(m_Angle)) * 20.0f;
			m_Hand[i].m_TargetPos += m_ArmPos + vec2(0, -16);
		}
		
		// gun aim
		if (i == HAND_WEAPON)
		{
			if (GetWeaponRenderType(m_Weapon) == WRT_WEAPON2)
			{
				m_Hand[i].m_TargetPos = vec2(cos(m_Angle), sin(m_Angle)) * 20.0f;
				m_Hand[i].m_TargetPos += m_ArmPos + vec2(0, -16);
				m_Hand[i].m_TargetPos += m_WeaponRecoil;
				m_Hand[i].m_Pos += (m_Hand[i].m_TargetPos - m_Hand[i].m_Pos) / 2.0f;
			}
			else if (GetWeaponRenderType(m_Weapon) == WRT_ITEM1)
			{
				if (GetWeaponFiringType(m_Weapon) == WFT_THROW)
					m_WeaponRecoil -= vec2(cos(m_Angle), sin(m_Angle)) * m_Charge * 0.12f;
		
				m_Hand[i].m_TargetPos = vec2(cos(m_Angle), sin(m_Angle)) * 13.0f;
				m_Hand[i].m_TargetPos += m_ArmPos + vec2(0, -16);
				m_Hand[i].m_TargetPos += m_WeaponRecoil;
				m_Hand[i].m_Pos += (m_Hand[i].m_TargetPos - m_Hand[i].m_Pos) / 2.0f;
			}
			else
				m_Hand[i].m_TargetPos += m_WeaponRecoil;
		}
		
		m_Hand[i].m_Vel *= 0.9f;
		m_Hand[i].m_Vel.x -= (((PrevVel.x-PlayerVel.x)/2000)*((PrevVel.x-PlayerVel.x)/2000))/2.0f;
		m_Hand[i].m_Vel.y += (((PrevVel.y-PlayerVel.y)/2000)*((PrevVel.y-PlayerVel.y)/2000))/2.0f;
		m_Hand[i].m_Vel += (-m_Hand[i].m_Offset)/14.0f;
		
		if ((!m_Hang || i == HAND_WEAPON) && length(m_Hand[i].m_Vel) > 3.0f)
			m_Hand[i].m_Vel = normalize(m_Hand[i].m_Vel) * 3.0f;
		
		m_Hand[i].m_Offset += m_Hand[i].m_Vel;
		
		if ((!m_Hang || i == HAND_WEAPON) && length(m_Hand[i].m_Offset) > 20.0f)
			m_Hand[i].m_Offset = normalize(m_Hand[i].m_Offset) * 20.0f;
		

	}
	
	// spinning melee weapon
	float TurnSpeedCap = 0.15f;
	float TurnAmount = 0.03f;
	
	/*
	if (m_Melee.m_EffectFrame > 0)
	{
		if (++m_Melee.m_EffectFrame > 7*3)
			m_Melee.m_EffectFrame = 0;
	}
	
	if (m_Melee.m_FireTimer > 0)
	{
		m_Melee.m_FireTimer--;
		TurnSpeedCap = 0.5f + m_WeaponPowerLevel*0.1f;
		TurnAmount = 0.04f;
		
		if ((Animation()->m_Flip && m_Melee.m_TurnSpeed > 0.0f) ||
			(!Animation()->m_Flip && m_Melee.m_TurnSpeed < 0.0f))
			m_Melee.m_Flip = true;
			
		if ((Animation()->m_Flip && m_Melee.m_TurnSpeed < 0.0f) ||
			(!Animation()->m_Flip && m_Melee.m_TurnSpeed > 0.0f))
			m_Melee.m_Flip = false;
	}
	else
		m_Melee.m_Flip = false;
	
	
	if ((Animation()->m_Flip && !m_Melee.m_Flip) || (!Animation()->m_Flip && m_Melee.m_Flip))
		m_Melee.m_TurnSpeed = max(m_Melee.m_TurnSpeed-TurnAmount, -TurnSpeedCap);
	else
		m_Melee.m_TurnSpeed = min(m_Melee.m_TurnSpeed+TurnAmount, TurnSpeedCap);
	
	m_Melee.m_Angle += m_Melee.m_TurnSpeed;
	
	if (abs(m_Melee.m_FrontChangeAngle-m_Melee.m_Angle) >= pi*2)
	{
		m_Melee.m_FrontChangeAngle += pi*2.0f * ((m_Melee.m_Angle > m_Melee.m_FrontChangeAngle) ? 1.0f : -1.0f);
		m_Melee.m_Front = !m_Melee.m_Front;
	}
	*/


	// adjust skeleton a bit
	int Anim = Animation()->GetAnimation();
	
	if (Anim != PANIM_SLIDE && Anim != PANIM_SLIDEDOWN && Anim != PANIM_SLIDEUP && Anim != PANIM_SLIDEKICK)
	{
		if (Animation()->m_Flip)
		{
			Animation()->m_BodyTilt = -PlayerVel.x*0.000075f;
			Animation()->m_HeadTiltCorrect = +PlayerVel.x*0.000035f;
		}
		else
		{
			Animation()->m_BodyTilt = PlayerVel.x*0.000075f;
			Animation()->m_HeadTiltCorrect = -PlayerVel.x*0.000035f;
		}
}
	
	// flame motion, adjust the angle a bit
	m_aFlameAngle[0] = m_Angle + (m_Angle < pi/2 ? 0.05f : -0.05f);
	
	for (int i = 1; i < 20; i++)
		m_aFlameAngle[i] = CurveAngle(m_aFlameAngle[i], m_aFlameAngle[i-1], 1.7f);
	

	// weapon recoil
 	m_WeaponRecoilVel.x -= m_WeaponRecoil.x / 6.0f;
	m_WeaponRecoilVel.y -= m_WeaponRecoil.y / 6.0f;
	m_WeaponRecoilVel *= 0.7f;
			
	m_WeaponRecoil += m_WeaponRecoilVel;
	
	m_Weapon2RecoilVel.x -= (PlayerVel.x-PrevVel.x)/2000.0f;
	m_Weapon2RecoilVel.y -= (PlayerVel.y-PrevVel.y)/2000.0f;
		
	m_Weapon2RecoilVel.x -= m_Weapon2Recoil.x / 16.0f;
	m_Weapon2RecoilVel.y -= m_Weapon2Recoil.y / 16.0f;
		
	m_Weapon2RecoilVel *= 0.85f;

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
	
	//Animation()->m_HeadForce -= m_FeetRecoil.y * 0.002f;
	
	
	// muzzle
	for (int i = 0; i < 4; i++)
	{
		if (m_aMuzzleWeapon[i])
		{
			m_aMuzzleTime[i] += 0.15f;
			
			if (m_aMuzzleTime[i] > 1.0f)
				m_aMuzzleWeapon[i] = 0;
		}
	}
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
		
		if (i++ > 1)
		{
			m_LastUpdate = currentTime;
			break;
		}
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
			
			if (i == EFFECT_DEATHRAY)
				m_EffectIntensity[i] -= 0.1f;
			
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
	
	m_SpinningAngle += 1.5f * min(20, m_WeaponCharge);
	
	if (m_SpinningAngle > 2000.0f*pi)
		m_SpinningAngle -= 2000.0f*pi;
	
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
	
	if (m_UpdateTimer++ > 200)
		m_InUse = false;
	
	if (m_MeleeAnimState > 0.0f)
	{
		m_MeleeAnimState += 0.2f;
		if (int(m_MeleeAnimState) > 3)
			m_MeleeAnimState = 0.0f;
	}
	
	// flamethrower frame animation
	if (m_FlameState > 0)
	{
		if (++m_FlameState > 13*4)
			m_FlameState = 0;
	}
	
	if (m_EffectIntensity[EFFECT_DEATHRAY] <= 0.0f)
		m_pAnimation->Tick();
}

