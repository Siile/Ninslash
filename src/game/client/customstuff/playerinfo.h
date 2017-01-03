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
	
public:
	CPlayerInfo();
	
	vec2 m_Pos;
	vec2 m_Vel;
	
	bool m_HideName;
	
	CSkeletonAnimation *Animation() { return m_pAnimation; }
	
	bool m_Shield;
	float m_Heal;
	
	int m_NextSplatter;
	float m_aSplatter[8];
	
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

	void AddSplatter();
	
	void SetLocal();
	bool m_Local;
	
	vec4 m_Color;
	
	bool m_InUse;

	static CPlayerInfo *GetIdle();
};



#endif
