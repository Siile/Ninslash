#ifndef GAME_CLIENT_COMPONENTS_FLUID_H
#define GAME_CLIENT_COMPONENTS_FLUID_H
#include <base/vmath.h>
#include <game/client/component.h>

struct CPool
{
	vec2 m_Pos;
	vec2 m_Size;
	
	float m_aSurfaceY[4*32];
	float m_aSurfaceVel[4*32];
	
	float m_Wave;
	
	CPool()
	{
		Reset();
	}
	
	void Reset()
	{
		for (int i = 0; i < 4*32; i++)
		{
			m_aSurfaceY[i] = 0.0f;
			m_aSurfaceVel[i] = 0.0f;
		}
		
		m_Pos = vec2(0, 0);
		m_Size = vec2(0, 0);
		m_Wave = 0.0f;
	}
	
	void UpdateTension()
	{
		float A = 0.05f;
		float B = 0.95f;
		float C = 0.02f;
		
		m_Wave += (frandom()-frandom())*0.75f;
		
		for (int i = 0; i <= m_Size.x/16; i++)
		{
			if (i >= 4*32)
				break;
			
			m_aSurfaceVel[i] -= (m_aSurfaceY[i]-sin(m_Wave+i*0.6f)+cos((m_Wave+i*0.6f)/7.0f)*6.0f*frandom()) * C;
			m_aSurfaceVel[i] *= B;
			
			if (i > 0)
				m_aSurfaceVel[i] += (m_aSurfaceY[i-1] - m_aSurfaceY[i]) * A;
			else
				m_aSurfaceVel[i] += (- m_aSurfaceY[i]) * A;
			
			if (i < m_Size.x/16)
				m_aSurfaceVel[i] += (m_aSurfaceY[i+1] - m_aSurfaceY[i]) * A;
			else
				m_aSurfaceVel[i] += (- m_aSurfaceY[i]) * A;
				
			
			m_aSurfaceVel[i] = clamp(m_aSurfaceVel[i], -15.0f, 15.0f);
		}
	}
	
	void UpdateSurface()
	{
		for (int i = 0; i <= m_Size.x/16; i++)
		{
			m_aSurfaceY[i] += m_aSurfaceVel[i];
			m_aSurfaceY[i] = clamp(m_aSurfaceY[i], -32.0f, 32.0f);
		}
	}
};


class CFluid : public CComponent
{
	friend class CGameClient;
public:
	CFluid();
	
	enum { MAX_POOLS = 9 };

	void Add(vec2 Pos);

	void Generate();
	
	virtual void OnReset();
	virtual void OnRender();
	
	void AddForce(vec2 Pos, vec2 Vel);

private:
	int m_PoolCount;
	CPool m_aPool[MAX_POOLS];
	
	void RenderPool(int i);
 
	void Update(float TimePassed);
};

#endif