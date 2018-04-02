#ifndef GAME_CLIENT_COMPONENTS_BRAINS_H
#define GAME_CLIENT_COMPONENTS_BRAINS_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>

// brain particles
struct CBrainSpill
{
	void SetDefault()
	{
		m_LifeSpan = 0;
		m_StartSize = 32;
		m_EndSize = 32;
		m_Gravity = 0;
		m_Friction = 0;
		m_FlowAffected = 1.0f;
		m_Color = vec4(1,1,1,1);
		m_Freeze = false;
		m_Parts = 6;
		m_Angle = 0.0f;
	}

	void SetPosition(vec2 Pos)
	{
		m_aPos[0] = Pos;
	
		for (int p = 1; p < m_Parts+1; p++)
		{
			float a = 2*pi / float(m_Parts) * (p-1);
			m_aPos[p] = Pos + vec2(sin(a), cos(a))*20;
		}
	}
	
	vec2 m_aPos[19];
	vec2 m_aVel[19];

	bool m_Freeze;
	
	float m_Angle;
	
	int m_Parts;
	int m_Spr;

	float m_FlowAffected;

	float m_LifeSpan;

	float m_StartSize;
	float m_EndSize;

	float m_Gravity;
	float m_Friction;

	vec4 m_Color;

	// set by the particle system
	float m_Life;
	int m_PrevPart;
	int m_NextPart;
};

class CBrains : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_BRAINS=0,
		NUM_GROUPS
	};

	CBrains();

	void Add(int Group, CBrainSpill *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_BRAINS=128,
	};
	
	CBrainSpill m_aBrains[MAX_BRAINS];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CBrains *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_BRAINS> m_RenderBrains;
};
#endif
