#ifndef GAME_CLIENT_COMPONENTS_SPLATTER_H
#define GAME_CLIENT_COMPONENTS_SPLATTER_H
#include <base/vmath.h>
#include <game/client/component.h>

// particles
struct CBloodspill
{
	void SetDefault()
	{
		m_LifeSpan = 0;
		m_Size = 32;
		m_Rot = 0;
		m_FlowAffected = 1.0f;
		m_Color = vec4(1,1,1,1);
		m_Freeze = false;
	}

	vec2 m_Pos;

	bool m_Freeze;
	
	int m_Spr;

	float m_FlowAffected;

	float m_LifeSpan;

	float m_Size;

	float m_Rot;

	vec4 m_Color;

	// set by the particle system
	float m_Life;
	int m_PrevPart;
	int m_NextPart;
};

class CSplatter : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_SPLATTER=0,
		NUM_GROUPS
	};

	CSplatter();

	void Add(int Group, CBloodspill *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_SPLATTER=1024*8,
	};

	CBloodspill m_aSplatter[MAX_SPLATTER];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CSplatter *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_SPLATTER> m_RenderSplatter;
};
#endif
