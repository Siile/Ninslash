#ifndef GAME_GAMECORE_H
#define GAME_GAMECORE_H

#include <base/system.h>
#include <base/math.h>

#include <math.h>
#include "collision.h"
#include <engine/shared/protocol.h>
#include <generated/protocol.h>


class CTuneParam
{
	int m_Value;
public:
	void Set(int v) { m_Value = v; }
	int Get() const { return m_Value; }
	CTuneParam &operator = (int v) { m_Value = (int)(v*100.0f); return *this; }
	CTuneParam &operator = (float v) { m_Value = (int)(v*100.0f); return *this; }
	operator float() const { return m_Value/100.0f; }
};

class CTuningParams
{
public:
	CTuningParams()
	{
		const float TicksPerSecond = 50.0f;
		#define MACRO_TUNING_PARAM(Name,ScriptName,Value) m_##Name.Set((int)(Value*100.0f));
		#include "tuning.h"
		#undef MACRO_TUNING_PARAM
	}

	static const char *m_apNames[];

	#define MACRO_TUNING_PARAM(Name,ScriptName,Value) CTuneParam m_##Name;
	#include "tuning.h"
	#undef MACRO_TUNING_PARAM

	static int Num() { return sizeof(CTuningParams)/sizeof(int); }
	bool Set(int Index, float Value);
	bool Set(const char *pName, float Value);
	bool Get(int Index, float *pValue);
	bool Get(const char *pName, float *pValue);
};


inline vec2 GetDirection(int Angle)
{
	float a = Angle/256.0f;
	return vec2(cosf(a), sinf(a));
}

inline vec2 GetDir(float Angle)
{
	return vec2(cosf(Angle), sinf(Angle));
}

inline float GetAngle(vec2 Dir)
{
	if(Dir.x == 0 && Dir.y == 0)
		return 0.0f;
	float a = atanf(Dir.y/Dir.x);
	if(Dir.x < 0)
		a = a+pi;
	return a;
}

inline void StrToInts(int *pInts, int Num, const char *pStr)
{
	int Index = 0;
	while(Num)
	{
		char aBuf[4] = {0,0,0,0};
		for(int c = 0; c < 4 && pStr[Index]; c++, Index++)
			aBuf[c] = pStr[Index];
		*pInts = ((aBuf[0]+128)<<24)|((aBuf[1]+128)<<16)|((aBuf[2]+128)<<8)|(aBuf[3]+128);
		pInts++;
		Num--;
	}

	// null terminate
	pInts[-1] &= 0xffffff00;
}

inline void IntsToStr(const int *pInts, int Num, char *pStr)
{
	while(Num)
	{
		pStr[0] = (((*pInts)>>24)&0xff)-128;
		pStr[1] = (((*pInts)>>16)&0xff)-128;
		pStr[2] = (((*pInts)>>8)&0xff)-128;
		pStr[3] = ((*pInts)&0xff)-128;
		pStr += 4;
		pInts++;
		Num--;
	}

	// null terminate
	pStr[-1] = 0;
}


/*
inline vec2 CalcPos(vec2 Pos, vec2 Velocity, float Curvature, float Speed, float Time)
{
	vec2 n;
	Time *= Speed;
	n.x = Pos.x + Velocity.x*Time;
	n.y = Pos.y + Velocity.y*Time + Curvature/10000*(Time*Time);
	return n;
}
*/

inline vec2 CalcPos(vec2 Pos, vec2 Velocity, vec2 Velocity2, float Curvature, float Speed, float Time)
{
	vec2 n;
	n.x = Pos.x + sin(Velocity.x)*(Velocity.x > 0.0f ? 1 : -1)*Velocity2.x*Time;
	n.y = Pos.y + sin(Velocity.y)*(Velocity.y > 0.0f ? 1 : -1)*Velocity2.y*Time;
	Time *= Speed;
	//n.x += Velocity.x*Time + sin(Time*0.01f)*min(Time*0.02f, 12.0f)*Velocity.y;
	//n.y += Velocity.y*Time + Curvature/10000*(Time*Time) + sin(Time*0.01f)*min(Time*0.02f, 12.0f)*Velocity.x;
	n.x += Velocity.x*Time;
	n.y += Velocity.y*Time + Curvature/10000*(Time*Time*(1.0f+Time*0.00025f));
	return n;
}

inline vec2 CalcTPos(vec2 Pos, vec2 Velocity, vec2 Velocity2, float Curvature, float Speed, float Time)
{
	vec2 n;
	n.x = Pos.x + sin(Velocity.x)*(Velocity.x > 0.0f ? 1 : -1)*Velocity2.x*Time + sin(Time*0.03f)*min(Time*0.1f, 48.0f)*Velocity.y;
	n.y = Pos.y + sin(Velocity.y)*(Velocity.y > 0.0f ? 1 : -1)*Velocity2.y*Time + sin(Time*0.04f)*min(Time*0.1f, 48.0f)*Velocity.y;
	Time *= Speed;
	n.x += Velocity.x*Time;
	n.y += Velocity.y*Time + Curvature/10000*(Time*Time);
	return n;
}

/*
inline vec2 CalcTPos(vec2 Pos, vec2 Velocity, float Curvature, float Speed, float Time)
{
	vec2 n;
	Time *= Speed;
	n.x = Pos.x + Velocity.x*Time + sin(Time*0.03f)*min(Time*0.1f, 48.0f)*Velocity.y;
	n.y = Pos.y + Velocity.y*Time + Curvature/10000*(Time*Time) + sin(Time*0.03f)*min(Time*0.1f, 48.0f)*Velocity.x;
	return n;
}
*/

inline vec2 CalcLogPos(vec2 Pos, vec2 Velocity, vec2 Velocity2, float Curvature, float Speed, float Time)
{
	vec2 n;
	n.x = Pos.x + sin(Velocity.x)*(Velocity.x > 0.0f ? 1 : -1)*Velocity2.x*Time;
	n.y = Pos.y + sin(Velocity.y)*(Velocity.y > 0.0f ? 1 : -1)*Velocity2.y*Time;
	Time *= Speed;
	float T = log(1.0f + Time)*150.0f;

	n.x += Velocity.x*T;
	n.y += Velocity.y*T + Curvature/10000*(Time*Time);
	return n;
}

inline vec2 CalcRocketPos(vec2 Pos, vec2 Velocity, vec2 Velocity2, float Curvature, float Speed, float Time)
{
	Velocity2 *= 0.5f;
	
	vec2 n;
	n.x = Pos.x + sin(Velocity.x)*(Velocity.x > 0.0f ? 1 : -1)*Velocity2.x*Time;
	n.y = Pos.y + sin(Velocity.y)*(Velocity.y > 0.0f ? 1 : -1)*Velocity2.y*Time;
	Time *= Speed;
	float T = Time*min(4.0f, 0.1f + Time*0.01f);

	n.x += Velocity.x*T;
	n.y += Velocity.y*T + Curvature/10000*(Time*Time);
	return n;
}


template<typename T>
inline T SaturatedAdd(T Min, T Max, T Current, T Modifier)
{
	if(Modifier < 0)
	{
		if(Current < Min)
			return Current;
		Current += Modifier;
		if(Current < Min)
			Current = Min;
		return Current;
	}
	else
	{
		if(Current > Max)
			return Current;
		Current += Modifier;
		if(Current > Max)
			Current = Max;
		return Current;
	}
}


float VelocityRamp(float Value, float Start, float Range, float Curvature);

// hooking stuff
enum
{
	HOOK_RETRACTED=-1,
	HOOK_IDLE=0,
	HOOK_RETRACT_START=1,
	HOOK_RETRACT_END=3,
	HOOK_FLYING,
	HOOK_GRABBED,
	HOOK_GRABBEDBALL,
	HOOK_GRABBEDDROID,
	
	COREEVENT_GROUND_JUMP=0x01,
	COREEVENT_AIR_JUMP=0x02,
	COREEVENT_SLIDEKICK=0x04,
	COREEVENT_BALL_BOUNCE=0x08,
	COREEVENT_HOOK_LAUNCH=0x10,
	COREEVENT_HOOK_ATTACH_PLAYER=0x20,
	COREEVENT_HOOK_ATTACH_GROUND=0x40,
	COREEVENT_HOOK_HIT_NOHOOK=0x80,
	COREEVENT_HOOK_RETRACT=0x100,
};


class CWorldCore
{
public:
	CWorldCore()
	{
		mem_zero(m_apCharacters, sizeof(m_apCharacters));
		//mem_zero(m_pBall, sizeof(m_pBall));
		m_pBall = 0;
		
		ClearImpacts();
		ClearDroids();
		ClearDroidHookImpacts();
	}

	void ClearImpacts()
	{
		m_Impact = 0;
		
		for (int i = 0; i < MAX_IMPACTS; i++)
			m_aImpactPos[i] = vec4(0, 0, 0, 0);
	}
	
	void AddImpact(vec4 Pos)
	{
		if (m_Impact < MAX_IMPACTS)
			m_aImpactPos[m_Impact] = Pos;
	
		m_Impact++;
	}
	
	
	void ClearDroids()
	{
		m_DroidCounter = 0;
		
		for (int i = 0; i < MAX_DROIDS; i++)
		{
			m_aDroidPos[i] = vec2(0, 0);
			m_aDroidVel[i] = vec2(0.0f, 0.0f);
			m_aDroidRadius[i] = 0;
			m_aDroidID[i] = 0;
		}
	}
	
	void ClearDroidHookImpacts()
	{
		m_DroidHookImpactCounter = 0;
		
		for (int i = 0; i < MAX_DROIDS; i++)
		{
			m_aDroidHookImpactID[i] = 0;
			m_aDroidHookImpactVel[i] = vec2(0, 0);
		}
	}
	
	bool CanFindDroid(int ID)
	{
		for (int i = 0; i < m_DroidCounter; i++)
			if (m_aDroidID[i] == ID)
				return true;
			
		return false;
	}
	
	vec2 FindDroidVel(int ID)
	{
		for (int i = 0; i < m_DroidCounter; i++)
			if (m_aDroidID[i] == ID)
				return m_aDroidVel[i];

		return vec2(0, 0);
	}
	
	vec2 FindDroidHookImpactVel(int ID)
	{
		for (int i = 0; i < m_DroidHookImpactCounter; i++)
			if (m_aDroidHookImpactID[i] == ID)
			{
				vec2 rv = m_aDroidHookImpactVel[i];
				m_aDroidHookImpactID[i] = 0;
				m_aDroidHookImpactVel[i] = vec2(0, 0);
				
				return rv;
			}

		return vec2(0, 0);
	}
	
	void AddDroid(int ID, vec2 Pos, vec2 Vel, int Radius)
	{
		if (m_DroidCounter < MAX_DROIDS)
		{
			m_aDroidPos[m_DroidCounter] = Pos;
			m_aDroidVel[m_DroidCounter] = Vel;
			m_aDroidRadius[m_DroidCounter] = Radius;
			m_aDroidID[m_DroidCounter] = ID;
		}
	
		m_DroidCounter++;
	}
	
	void AddDroidHookImpact(int ID, vec2 Vel)
	{
		// check for existing impacts in case multiple players are hooking to single droid
		for (int i = 0; i < m_DroidHookImpactCounter; i++)
			if (m_aDroidHookImpactID[i] == ID)
			{
				m_aDroidHookImpactVel[i] += Vel;
				return;
			}
		
		if (m_DroidHookImpactCounter < MAX_DROIDS)
		{
			m_aDroidHookImpactID[m_DroidHookImpactCounter] = ID;
			m_aDroidHookImpactVel[m_DroidHookImpactCounter] = Vel;
		}
	
		m_DroidHookImpactCounter++;
	}
	
	
	CTuningParams m_Tuning;
	class CCharacterCore *m_apCharacters[MAX_CLIENTS];
	class CBallCore *m_pBall;
	
	// jumppad impact to gore
	int m_Impact;
	vec4 m_aImpactPos[MAX_IMPACTS];
	
	// droid hooking
	int m_DroidCounter;
	vec2 m_aDroidPos[MAX_DROIDS];
	vec2 m_aDroidVel[MAX_DROIDS];
	int m_aDroidRadius[MAX_DROIDS];
	int m_aDroidID[MAX_DROIDS];
	
	// hackish way to return hook impact to server
	int m_DroidHookImpactCounter;
	int m_aDroidHookImpactID[MAX_DROIDS];
	vec2 m_aDroidHookImpactVel[MAX_DROIDS];
};


class CBallCore
{
	CWorldCore *m_pWorld;
	CCollision *m_pCollision;
	
public:
	vec2 m_Pos;
	vec2 m_Vel;
	float m_Angle;
	float m_AngleForce;
	int m_Status;
	bool m_ForceCoreSend;
	
	int m_TriggeredEvents;
	
	CBallCore();
	void Init(CWorldCore *pWorld, CCollision *pCollision);
	void Reset();
	void Tick();
	void Move();
	void PlayerHit();
	
	void Read(const CNetObj_BallCore *pObjCore);
	void Write(CNetObj_BallCore *pObjCore);
	void Quantize();
	
	bool PlatformState();
	const float BallSize();
	
	bool Status(int i)
	{
		if (m_Status & (1<<i))
			return true;

		return false;
	}
};


class CCharacterCore
{
	CWorldCore *m_pWorld;
	CCollision *m_pCollision;
	void Roll();
	void Slide();
	
	void SetAction(int Action, int State = 0)
	{
		m_Action = Action;
		m_ActionState = State;
	}
	
public:
	vec2 m_Pos;
	vec2 m_Vel;
	
	vec2 m_HookPos;
	vec2 m_HookDir;
	int m_HookTick;
	int m_HookState;
	int m_HookedPlayer;
	
	vec2 m_BallHitVel;
	
	bool m_PlayerCollision;
	bool m_MonsterDamage;
	bool m_FluidDamage;
	bool m_HandJetpack;

	// used for getting kick damage owner
	int m_ClientID;
	int m_KickDamage;
	
	int m_Jumped;
	
	int m_Health;

	// for animation, not syncronized
	int m_JumpTimer;
	
	int m_DashTimer;
	int m_DashAngle;
	
	int m_Direction;
	int m_Down;
	int m_Angle;
	int m_Anim;
	int m_LockDirection;
	int m_Jetpack;
	int m_JetpackPower;
	int m_Wallrun;
	bool m_OnWall;
	int m_Roll;
	int m_Slide;
	
	int m_Charge;
	int m_ChargeLevel;
	
	bool FullCharge() { return m_ChargeLevel == 100; }
	
	int m_DamageTick;
			
	int m_Action;
	int m_ActionState;
	
	int m_Status;
	
	bool m_Sliding;
	
	CNetObj_PlayerInput m_Input;

	int m_TriggeredEvents;

	CCharacterCore();
	void Init(CWorldCore *pWorld, CCollision *pCollision);
	void Reset();
	void Tick(bool UseInput);
	void Move();
	
	bool PlatformState();
	bool IsGrounded();
	int IsOnForceTile();
	bool IsInFluid();
	int SlopeState();

	void Jumppad();
	
	void Read(const CNetObj_CharacterCore *pObjCore);
	void Write(CNetObj_CharacterCore *pObjCore);
	void Quantize();
	
	bool Status(int i)
	{
		int s = m_Status;
		if (s & (1<<i))
			return true;
		
		return false;
	}
};

#endif
