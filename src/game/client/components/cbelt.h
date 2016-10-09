#ifndef GAME_CLIENT_COMPONENTS_CBELT_H
#define GAME_CLIENT_COMPONENTS_CBELT_H
#include <base/vmath.h>
#include <game/client/component.h>

struct CCBeltSingle
{
	vec2 m_Pos;
	vec2 m_Size;
	
	int m_Dir;
	
	float m_Roll;
	
	CCBeltSingle()
	{
		Reset();
	}
	
	void Reset()
	{
		m_Roll = frandom();
		m_Dir = 0;
		m_Pos = vec2(0, 0);
		m_Size = vec2(0, 0);
	}
	
	
	void Update()
	{
		//m_Roll -= m_Dir * 0.055f;
		m_Roll -= m_Dir * 0.026f;
	}
};


class CCBelt : public CComponent
{
	friend class CGameClient;
public:
	CCBelt();
	
	enum { MAX_CBELTS = 99 };

	void Add(vec2 Pos);

	void Generate();
	
	virtual void OnReset();
	virtual void OnRender();
	
	void AddForce(vec2 Pos, vec2 Vel);

private:
	int m_Count;
	CCBeltSingle m_aCBelt[MAX_CBELTS];
	
	void RenderCBelt(int i);
 
	void Update(float TimePassed);
};

#endif