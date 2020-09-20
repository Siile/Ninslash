#ifndef GAME_CLIENT_COMPONENTS_SPARK_H
#define GAME_CLIENT_COMPONENTS_SPARK_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>

// particles
struct CSinglespark
{
	void SetDefault()
	{
		m_Vel = vec2(0,0);
		m_LifeSpan = 0;
		m_Size = 2;
		m_Rot = 0;
		m_Rotspeed = 0;
		m_Color = vec4(1,1,1,1);
	}

	vec2 m_Pos;
	vec2 m_Vel;

	float m_LifeSpan;
	float m_Size;
	float m_Rot;
	float m_Rotspeed;

	vec4 m_Color;

	// set by the particle system
	float m_Life;
	int m_PrevPart;
	int m_NextPart;
};

class CSpark : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_SPARKS=0,
		GROUP_AREA1,
		NUM_GROUPS
	};

	CSpark();

	void Add(int Group, CSinglespark *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_SPARKS=1024*4,
	};

	CSinglespark m_aSpark[MAX_SPARKS];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CSpark *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_SPARKS> m_RenderSpark;
	CRenderGroup<GROUP_AREA1> m_RenderArea1;
};
#endif
