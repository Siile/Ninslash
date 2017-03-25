#ifndef GAME_CLIENT_COMPONENTS_BLOOD_H
#define GAME_CLIENT_COMPONENTS_BLOOD_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>

// particles
struct CBlooddrop
{
	void SetDefault()
	{
		m_Vel = vec2(0,0);
		m_LifeSpan = 0;
		m_StartSize = 32;
		m_EndSize = 32;
		m_Rot = 0;
		m_Rotspeed = 0;
		m_Gravity = 0;
		m_Friction = 0;
		m_FlowAffected = 1.0f;
		m_Color = vec4(1,1,1,1);
		m_Freeze = false;
	}

	vec2 m_Pos;
	vec2 m_Vel;

	bool m_Freeze;
	
	int m_Spr;

	float m_FlowAffected;

	float m_LifeSpan;

	float m_StartSize;
	float m_EndSize;

	float m_Rot;
	float m_Rotspeed;

	float m_Gravity;
	float m_Friction;

	vec4 m_Color;

	// set by the particle system
	float m_Life;
	int m_PrevPart;
	int m_NextPart;
};

class CBlood : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_BLOOD=0,
		GROUP_ACID,
		GROUP_ACIDLAYER,
		NUM_GROUPS
	};

	CBlood();

	void Add(int Group, CBlooddrop *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_BLOOD=1024*8,
	};

	void Bounce(vec2 Pos, vec2 Dir, int Group, vec4 Color);
	
	CBlooddrop m_aBlood[MAX_BLOOD];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CBlood *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_BLOOD> m_RenderBlood;
	CRenderGroup<GROUP_ACID> m_RenderAcid;
	CRenderGroup<GROUP_ACIDLAYER> m_RenderAcidLayer;
};
#endif
