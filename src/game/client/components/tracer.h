#ifndef GAME_CLIENT_COMPONENTS_TRACER_H
#define GAME_CLIENT_COMPONENTS_TRACER_H
#include <base/math.h>
#include <base/vmath.h>
#include <game/weapons.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>


// particles
struct CTrace
{
	void Set(int ItemID, int Type, vec2 Pos, vec2 StartPos, int StartTick, int Weapon, vec2 Vel)
	{
		// existing tracer
		if (abs(StartTick - m_StartTick) < 2 && ItemID == m_ItemID && Type == m_Type)
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
				m_Pos[i] = mix(Pos, StartPos, i*0.01f);
				m_Vel[i] = Vel;
			}
			
			m_DelayPos = Pos;
			m_StartTick = StartTick;
			
			SetDefault(Type, Weapon);
		}
	}
	
	void Set(vec2 Pos, vec2 Vel)
	{
		m_Pos[0] = Pos;
		m_Vel[0] = Vel;
		m_Life = 0.0f;
	}
	
	void SetDefault(int Type, int Weapon)
	{
		m_Type = Type;
		m_LifeSpan = 0.3f;
		m_Special = 0;
		m_Parts = 12;
		
		if (Type == 1)
		{
			m_Speed = 4.0f;
			m_Size1 = 6.0f * GetProjectileSize(Weapon);
			m_Size2 = 2.0f * GetProjectileSize(Weapon);
			m_LifeSpan = 0.4f;
			m_Color = vec4(0.6f, 0.4f, 0.2f, 0.8f);
			return;
		}
	
		
		if (Type == 2)
		{
			m_Speed = 6.0f;
			m_Size1 = 6.0f * GetProjectileSize(Weapon);
			m_Size2 = 0.0f; // * GetProjectileSize(Weapon);
			m_LifeSpan = 0.4f;
			m_Color = vec4(0.2f, 1.0f, 0.2f, 0.5f);
			return;
		}
		
		if (Type == 3)
		{
			m_Parts = 24;
			m_Speed = 8.0f;
			m_Size1 = 6.0f * GetProjectileSize(Weapon);
			m_Size2 = 0.0f; // * GetProjectileSize(Weapon);
			m_LifeSpan = 0.5f;
			m_Color = vec4(0.2f, 1.0f, 0.2f, 0.5f);
			return;
		}
	
		// grenade 1
		if (Type == 4)
		{
			m_Parts = 24;
			m_Speed = 3.0f;
			m_Size1 = 8.0f;
			m_Size2 = 0.0f;
			m_LifeSpan = 1.5f;
			m_Color = vec4(0.6f, 0.4f, 0.2f, 0.8f);
			return;
		}
		
		// grenade 2
		if (Type == 5)
		{
			m_Parts = 24;
			m_Speed = 3.0f;
			m_Size1 = 8.0f;
			m_Size2 = 0.0f;
			m_LifeSpan = 1.5f;
			m_Color = vec4(0.2f, 1.0f, 1.0f, 0.4f);
			return;
		}
		
		// bouncer
		if (Type == 6)
		{
			m_Parts = 20;
			m_Speed = 3.0f;
			m_Size1 = 10.0f;
			m_Size2 = 5.0f;
			m_LifeSpan = 1.5f;
			m_Color = vec4(0.0f, 1.0f, 0.0f, 0.5f);
			return;
		}
	
		
		if (Type == -1)
		{
			m_Scale = 2.0f;
			m_RotSpeed = 0.5f;
			m_Sprite = 0;
			m_Color = vec4(1, 1, 1, 0.5f);
			m_Parts = 30;
			m_Speed = 3.0f;
			m_Size1 = 20.0f * GetProjectileSize(Weapon);
			m_Size2 = 10.0f * GetProjectileSize(Weapon);
			return;
		}
		
		if (Type == -3)
		{
			m_RotSpeed = 0.5f;
			m_Scale = 2.0f;
			m_Sprite = 2;
			m_Color = vec4(1, 1, 1, 1.0f);
			m_Parts = 30;
			m_Speed = 3.0f;
			m_Size1 = 20.0f * GetProjectileSize(Weapon);
			m_Size2 = 10.0f * GetProjectileSize(Weapon);
			return;
		}
		
		if (Type == -2)
		{
			m_RotSpeed = 0.2f;
			m_Scale = 1.0f;
			m_LifeSpan = 0.3f;
			m_Sprite = 1;
			m_Color = vec4(1, 1, 1, 1.0f);
			m_Parts = 30;
			m_Speed = 6.0f;
			m_Size1 = 80.0f; m_Size2 = 80.0f;
		}
		
		if (Type == -4)
		{
			m_RotSpeed = 0.25f;
			m_Scale = 1.0f;
			m_LifeSpan = 1.0f;
			m_Sprite = 2;
			m_Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
			m_Parts = 20;
			m_Speed = 4.0f;
			m_Size1 = 50.0f; m_Size2 = 20.0f;
		}
	}
	
	int m_Sprite;
	int m_StartTick;
	int m_Type;
	int m_ItemID;
	float m_Life;
	float m_LifeSpan;
	float m_Speed;
	int m_Special;
	
	float m_Size1;
	float m_Size2;
	
	float m_Scale;
	float m_RotSpeed;
	
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

	void Add(int Type, int ItemID, vec2 Pos, vec2 StartPos, int StartTick, int Weapon, vec2 Vel = vec2(0, 0));
	void UpdatePos(int ItemID, vec2 Pos);
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
