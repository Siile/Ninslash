#ifndef GAME_CLIENT_CUSTOMSTUFF_DROIDANIM_H
#define GAME_CLIENT_CUSTOMSTUFF_DROIDANIM_H

#include <base/tl/string.h>
#include <base/vmath.h>
#include <map>

enum DroidAnimation
{
	DANIM_IDLE,
	NUM_DANIMS
};


const string DroidAnim[NUM_DANIMS] = {
	"idle",
};


class CDroidAnim
{
	int64 m_LastUpdate;
	
	CGameClient *m_pClient;
	class CCollision *Collision() const { return m_pClient->Collision(); }
	
public:
	enum DROID_VALUE_NAME
	{
		VEL_X,
		BODY_ANGLE,
		BOTTOM_ANGLE,
		TURRET_ANGLE,
		TURRET_OFFSET,
		ANIM_TIME,
		NUM_DROID_VALUE
	};
	
	enum DROID_VECTOR_VALUE_NAME
	{
		ATTACH1_POS,
		ATTACH2_POS,
		THRUST1_POS,
		THRUST2_POS,
		THRUST1_VEL,
		THRUST2_VEL,
		NUM_DROID_VECTOR_VALUE
	};
	
	float m_aValue[NUM_DROID_VALUE];
	vec2 m_aVectorValue[NUM_DROID_VECTOR_VALUE];

	int m_Dir;
	vec2 m_Pos;
	vec2 m_Vel;
	int m_Anim;
	int m_Type;
	
	int m_Status;
	
	// crawlers
	vec2 m_aLegTargetPos[4];
	vec2 m_aLegPos[4];
	float m_aLegAngle[4];
	float m_aLegTargetAngle[4];
	vec2 m_aLegVel[4];
	
	float m_Angle;
	float m_DisplayAngle;
	float m_TargetDisplayAngle;
	
	CDroidAnim(class CGameClient *pClient = NULL);
	~CDroidAnim();
	
	void Reset();
	void Tick();
};

#endif
