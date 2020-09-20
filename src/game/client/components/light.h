#ifndef GAME_CLIENT_COMPONENTS_LIGHT_H
#define GAME_CLIENT_COMPONENTS_LIGHT_H
#include <base/vmath.h>
#include <game/client/component.h>



struct CSimpleLight
{
	vec2 m_Pos;
	vec4 m_Color;
	vec2 m_Size;
	float m_Rot;
	
	void Set(vec2 Pos, vec4 Color, vec2 Size, float Rot = 0.0f)
	{
		m_Pos = Pos;
		m_Color = Color;
		m_Size = Size;
		m_Rot = Rot;
	}
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

	void AddSimpleLight(vec2 Pos, vec4 Color, vec2 Size);
	void AddBoxLight(vec2 Pos, vec4 Color, vec2 Size, float Rot = 0.0f);

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_LIGHTSOURCES=1024*2,
	};
	
	int m_Count;
	int m_SmallCount;
	int m_BoxCount;

	CSimpleLight m_aSimpleLight[MAX_LIGHTSOURCES];
	CSimpleLight m_aSmallSimpleLight[MAX_LIGHTSOURCES];
	CSimpleLight m_aBoxLight[MAX_LIGHTSOURCES];

	void RenderLight(vec2 Pos, vec2 Size, vec4 Color);
	void RenderLight(ivec2 Pos);
	void RenderLight(vec2 Pos1, vec2 Pos2, vec2 Pos3, vec2 Pos4, vec4 Color);
	
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
