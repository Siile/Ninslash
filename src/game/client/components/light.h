#ifndef GAME_CLIENT_COMPONENTS_LIGHT_H
#define GAME_CLIENT_COMPONENTS_LIGHT_H
#include <base/vmath.h>
#include <game/client/component.h>

// particles
struct CLightsource
{
	void SetDefault()
	{
		m_Size = 32;
		m_Color = vec4(1,1,1,1);
		m_Life = 0;
	}

	int m_Spr;
	
	vec2 m_Pos;
	float m_Size;
	vec4 m_Color;

	// set by the particle system
	float m_Life;
	int m_PrevPart;
	int m_NextPart;
};

class CLight : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_LIGHTSOURCE=0,
		NUM_GROUPS
	};

	CLight();

	void Add(int Group, CLightsource *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	int m_Count;
	bool m_Rendered;
 
	enum
	{
		MAX_LIGHTSOURCES=1024*2,
	};

	CLightsource m_aLightsource[MAX_LIGHTSOURCES];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CLight *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_LIGHTSOURCE> m_RenderLight;
};
#endif
