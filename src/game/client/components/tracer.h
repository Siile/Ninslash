#ifndef GAME_CLIENT_COMPONENTS_TRACER_H
#define GAME_CLIENT_COMPONENTS_TRACER_H
#include <base/vmath.h>
#include <game/weapons.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>


// particles
struct CTrace
{
	void Set(int ItemID, int Type, vec2 Pos, int StartTick, int PowerLevel, vec2 Vel)
	{
		// existing tracer
		if (StartTick == m_StartTick && ItemID == m_ItemID && Type == m_Type)
		{
			m_Life = 0.0f;
			m_Pos[0] = Pos;
			m_Vel[0] = Vel;
		}
		// new one
		else
		{
			m_Life = 0.0f;
			m_ItemID = ItemID;
			
			for (int i = 0; i < 99; i++)
			{
				m_Pos[i] = Pos;
				m_Vel[i] = Vel;
			}
			
			m_DelayPos = Pos;
			m_StartTick = StartTick;
			
			SetDefault(Type, PowerLevel);
		}
	}
	
	void Set(vec2 Pos, vec2 Vel)
	{
		m_Pos[0] = Pos;
		m_Vel[0] = Vel;
		m_Life = 0.0f;
	}
	
	void SetDefault(int Type, int PowerLevel)
	{
		m_Type = Type;
		m_LifeSpan = 0.3f;
		m_Special = 0;
		m_Parts = 8;
		
		switch (Type)
		{
			case W_RIFLE:
				m_Speed = 4.0f;
				m_Size1 = 3.0f+PowerLevel; m_Size2 = 0.0f;
				
				if (PowerLevel == 3)
				{
					m_Speed = 2.0f;
					m_Parts = 24;
					m_Size2 = m_Size1*2;
				}
				
				if (PowerLevel > 4)
				{
					m_Special = 1;
					m_Speed = 22.0f;
					m_LifeSpan = 0.4f;
				}
				
				m_Color = vec4(0, 0.4f+PowerLevel*0.1f, 1.0f, 0.3f);
				break;
				
			case W_SHOTGUN:
				m_Color = vec4(0.6f, 0.35f, 0.25f, 0.7f);
				m_Speed = 8.0f;
				m_Size1 = 4.0f; m_Size2 = 0.0f;
				break;
				
			case W_ELECTRIC:
				if (PowerLevel == 0)		m_Color = vec4(0.0f, 0.3f, 1.0f, 0.3f);
				else if (PowerLevel == 1)	m_Color = vec4(0.0f, 0.6f, 1.0f, 0.3f);
				else						m_Color = vec4(0.0f, 1.0f, 1.0f, 0.3f);
				
				m_Parts = 7;
				m_Speed = 6.0f;
				m_Size1 = 7.0f; m_Size2 = 4.0f;
				break;
			
			case -1:
				//m_Color = vec4(0, 1.0f, 0.5f, 0.2f);
				m_Color = vec4(1, 1, 1, 1);
				m_Parts = 20;
				m_Speed = 6.0f;
				m_Size1 = 100.0f; m_Size2 = 100.0f;
				break;
				
			default:
				m_Color = vec4(0, 0.5f, 1.0f, 0.3f);
				m_Speed = 16.0f;
				m_Size1 = 4.0f; m_Size2 = 0.0f;
		};
	}
	
	int m_StartTick;
	int m_Type;
	int m_ItemID;
	float m_Life;
	float m_LifeSpan;
	float m_Speed;
	int m_Special;
	
	float m_Size1;
	float m_Size2;
	
	int m_Parts;
	
	vec4 m_Color;
	vec2 m_Pos[99];
	vec2 m_Vel[99];
	vec2 m_DelayPos;

	float m_Gravity;
	float m_Friction;

	// set by the particle system
	int m_PrevPart;
	int m_NextPart;
};

class CTracer : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_TRACERS=0,
		GROUP_SPRITETRACERS,
		NUM_GROUPS
	};

	CTracer();

	void Add(int Type, int ItemID, vec2 Pos, int StartTick, int PowerLevel, vec2 Vel = vec2(0, 0));
	void Tick();

	virtual void OnReset();
	virtual void OnRender();

private:
 
	enum
	{
		MAX_TRACERS=128,
	};
	
	CTrace m_aTracer[MAX_TRACERS];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CTracer *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_TRACERS> m_RenderTracers;
	CRenderGroup<GROUP_SPRITETRACERS> m_RenderSpriteTracers;
};
#endif
