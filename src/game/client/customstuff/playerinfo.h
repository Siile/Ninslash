#ifndef GAME_CLIENT_CUSTOMSTUFF_PLAYERINFO_H
#define GAME_CLIENT_CUSTOMSTUFF_PLAYERINFO_H

#include <base/vmath.h>
#include <base/system.h>

#include <engine/graphics.h>
#include <game/client/gameclient.h>
#include <game/client/animdata.h>

enum MeleeState
{
	MELEE_UP,
	MELEE_DOWN,
};


class CPlayerInfo
{
private:
	
	// if > 5, !InUse
	int m_UpdateTimer;
	
	int64 m_LastUpdate;
	CSkeletonAnimation *m_pAnimation;
	
	struct CMelee
	{
		bool m_Front;
		bool m_Flip;
		float m_FrontChangeAngle;
		float m_Angle;
		float m_TurnSpeed;
		
		int m_FireTimer;
		int m_EffectFrame;
		
		void Reset()
		{
			m_EffectFrame = 0;
			m_FireTimer = 0;
			m_Flip = false;
			m_Front = false;
			m_FrontChangeAngle = 0.0f;
			m_Angle = 0.0f;
			m_TurnSpeed = 0.0f;
		}
	};
	
	CMelee m_Melee;
	
	
	struct CHand
	{
		bool m_Front;
		bool m_Flip;
		float m_Angle;
		vec2 m_Pos;
		vec2 m_TargetPos;
		vec2 m_Vel;
		vec2 m_Offset;
		
		void Reset()
		{
			m_Vel = vec2(0, 0);
			m_Pos = vec2(0, 0);
			m_Offset = vec2(0, 0);
			m_TargetPos = vec2(0, 0);
			m_Flip = false;
			m_Front = false;
			m_Angle = 0.0f;
		}
	};
	
	CHand m_Hand;
	
public:
	CPlayerInfo();
	
	bool m_Jetpack;
	vec2 m_ArmPos;
	
	bool m_Turbo;
	vec2 HandOffset();
	void SetHandTarget(vec3 Pos);
	int HandFrame();
	
	float MeleeAngle();
	float MeleeSize();
	bool MeleeFlip();
	bool MeleeEffectFlip();
	vec2 MeleeOffset();
	vec2 MeleeEffectOffset();
	bool MeleeFront();
	int MeleeFrame();
	int MeleeEffectFrame();
	bool MeleeSound();
	int MeleeImpact();
	
	void FireMelee();
	
	CTeeRenderInfo m_RenderInfo;
	
	vec2 m_Pos;
	vec2 m_Vel;
	
	float m_Angle;
	
	float m_aFlameAngle[20];
	
	int m_FlameState;
	
	bool m_HideName;
	
	CSkeletonAnimation *Animation() { return m_pAnimation; }
	
	bool m_Shield;
	float m_Heal;
	
	int m_NextSplatter;
	float m_aSplatter[8];
	vec4 m_aSplatterColor[8];
	
	int m_LastJetpackSound;
	int m_LastJetpackSoundTick;
	int m_LastChainsawSoundTick;
	
	int m_DamageTick;
	
	float m_EffectState;
	float m_EffectIntensity[NUM_EFFECTS];

	bool m_LoadInvisibility;
	
	void Reset();
	
	int m_Weapon;
	
	int m_MeleeState;
	int m_MeleeTick;
	float m_MeleeAnimState;
	
	float m_BodyTilt;
	
	bool m_AirJumpAnimLoaded;
	
	float m_ToolAngleOffset;
	float m_ToolAngleVel;
	
	// called from CCustomStuff
	void Tick();
	
	// called from CPlayers
	void UpdatePhysics(vec2 PlayerVel, vec2 PrevVel);
	void PhysicsTick(vec2 PlayerVel, vec2 PrevVel);
	
	void Update(vec2 Pos);
	
	vec2 Pos(){ return m_Pos; }
	
	bool m_FlipFeet;
	
	vec2 m_FeetPos;
	vec2 m_FeetRecoil, m_FeetRecoilVel;
	
	vec2 m_WeaponRecoil, m_WeaponRecoilVel;
	vec2 m_Weapon2Recoil, m_Weapon2RecoilVel;
	
	bool m_WeaponRecoilLoaded;

	void AddSplatter(vec4 Color);
	
	void SetLocal();
	bool m_Local;
	
	vec4 m_Color;
	
	bool m_InUse;

	static CPlayerInfo *GetIdle();
};



#endif
