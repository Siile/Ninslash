#ifndef GAME_CLIENT_COMPONENTS_PARTICLES_H
#define GAME_CLIENT_COMPONENTS_PARTICLES_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/components/effects.h>

// particles
struct CParticle
{
	void SetDefault()
	{
		m_Special = 0;
		m_Frames = 1;
		m_Vel = vec2(0,0);
		m_LifeSpan = 0;
		m_StartSize = 32;
		m_EndSize = 32;
		m_Rot = 0;
		m_Rotspeed = 0;
		m_Gravity = 0;
		m_Friction = 0;
		m_FlowAffected = 1.0f;
		m_Color = vec4(1,1,1,0.75f);
		
		m_StartPos = vec2(0, 0);
		m_EndPos = vec2(0, 0);
		
		m_Height = 0;
		m_Flip = false;
		m_IgnoreCollision = false;
		m_Chunk = false;
	}

	int m_Special;
	
	vec2 m_Pos;
	vec2 m_Vel;
	
	bool m_IgnoreCollision;
	
	int m_Height; // lazer
	bool m_Flip; // mine

	int m_Spr;
	int m_Frames;

	float m_FlowAffected;

	float m_LifeSpan;

	float m_StartSize;
	float m_EndSize;

	float m_Rot;
	float m_Rotspeed;
	bool m_Chunk;

	float m_Gravity;
	float m_Friction;

	vec4 m_Color;
	
	// bullet trail extras
	vec2 m_StartPos;
	vec2 m_EndPos;
	
	vec2 m_TrailDir;

	// set by the particle system
	float m_Life;
	int m_PrevPart;
	int m_NextPart;
};

class CParticles : public CComponent
{
	friend class CGameClient;
public:
	enum
	{
		GROUP_PROJECTILE_TRAIL=0,
		GROUP_COLORTRAIL,
		GROUP_EXPLOSIONS,
		GROUP_GREEN_EXPLOSION,
		GROUP_HITEFFECTS,
		GROUP_SMOKE1,
		GROUP_MINE1,
		GROUP_MINE2,
		GROUP_ELECTROMINE,
		GROUP_ELECTRIC,
		GROUP_GENERAL,
		GROUP_TRIANGLES,
		GROUP_FLAMES,
		GROUP_FLAME1,
		GROUP_SWORDHITS,
		GROUP_DEATH,
		GROUP_LAZER,
		GROUP_LAZERLOAD,
		GROUP_SPARKS,
		GROUP_TAKEOFF,
		GROUP_BLOODFX,
		GROUP_PLAYERSPAWN,
		GROUP_MONSTERSPAWN,
		GROUP_CRAFTING,
		GROUP_DAMAGEIND,
		GROUP_MEAT,
		GROUP_EFFECT1,
		NUM_GROUPS
	};

	CParticles();

	void Add(int Group, CParticle *pPart);

	virtual void OnReset();
	virtual void OnRender();

private:

	enum
	{
		MAX_PARTICLES=1024*8,
	};

	CParticle m_aParticles[MAX_PARTICLES];
	int m_FirstFree;
	int m_aFirstPart[NUM_GROUPS];

	void RenderGroup(int Group);
	void Update(float TimePassed);

	template<int TGROUP>
	class CRenderGroup : public CComponent
	{
	public:
		CParticles *m_pParts;
		virtual void OnRender() { m_pParts->RenderGroup(TGROUP); }
	};

	CRenderGroup<GROUP_PROJECTILE_TRAIL> m_RenderTrail;
	CRenderGroup<GROUP_COLORTRAIL> m_RenderColorTrail;
	CRenderGroup<GROUP_EXPLOSIONS> m_RenderExplosions;
	CRenderGroup<GROUP_GREEN_EXPLOSION> m_RenderGreenExplosion;
	CRenderGroup<GROUP_HITEFFECTS> m_RenderHitEffects;
	CRenderGroup<GROUP_SMOKE1> m_RenderSmoke1;
	CRenderGroup<GROUP_MINE1> m_RenderMine1;
	CRenderGroup<GROUP_MINE2> m_RenderMine2;
	CRenderGroup<GROUP_ELECTROMINE> m_RenderElectromine;
	CRenderGroup<GROUP_ELECTRIC> m_RenderElectric;
	CRenderGroup<GROUP_GENERAL> m_RenderGeneral;
	CRenderGroup<GROUP_TRIANGLES> m_RenderTriangles;
	CRenderGroup<GROUP_FLAMES> m_RenderFlames;
	CRenderGroup<GROUP_FLAME1> m_RenderFlame1;
	CRenderGroup<GROUP_SWORDHITS> m_RenderSwordHits;
	CRenderGroup<GROUP_DEATH> m_RenderDeath;
	CRenderGroup<GROUP_LAZER> m_RenderLazer;
	CRenderGroup<GROUP_LAZERLOAD> m_RenderLazerload;
	CRenderGroup<GROUP_SPARKS> m_RenderSparks;
	CRenderGroup<GROUP_TAKEOFF> m_RenderTakeoff;
	CRenderGroup<GROUP_BLOODFX> m_RenderBloodFX;
	CRenderGroup<GROUP_PLAYERSPAWN> m_RenderPlayerSpawn;
	CRenderGroup<GROUP_MONSTERSPAWN> m_RenderMonsterSpawn;
	CRenderGroup<GROUP_CRAFTING> m_RenderCrafting;
	CRenderGroup<GROUP_DAMAGEIND> m_RenderDamageInd;
	CRenderGroup<GROUP_MEAT> m_RenderMeat;
	CRenderGroup<GROUP_EFFECT1> m_RenderEffect1;
};
#endif
