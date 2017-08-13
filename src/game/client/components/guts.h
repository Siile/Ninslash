#ifndef GAME_CLIENT_COMPONENTS_GUTS_H
#define GAME_CLIENT_COMPONENTS_GUTS_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>

// particles
struct CGutSpill
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
	}

	vec2 m_aPos[5];
	vec2 m_aVel[5];

	bool m_Freeze;
	
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

class CGuts : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_GUTS=0,
		NUM_GROUPS
	};

	CGuts();

	void Add(int Group, CGutSpill *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_GUTS=128,
	};

	void Bounce(vec2 Pos, vec2 Dir, int Group, vec4 Color);
	
	CGutSpill m_aGuts[MAX_GUTS];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CGuts *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_GUTS> m_RenderGuts;
};
#endif
