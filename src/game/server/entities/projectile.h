#ifndef GAME_SERVER_ENTITIES_PROJECTILE_H
#define GAME_SERVER_ENTITIES_PROJECTILE_H

enum ProjectileExtraInfo
{
	PROJ_BIGEXPLOSION,
	PROJ_SUPEREXPLOSION,
	PROJ_SLEEPEFFECT
};

enum ExplosionType
{
	NO_EXPLOSION,
	EXPLOSION_EXPLOSION,
	EXPLOSION_FLAME,
	EXPLOSION_ELECTRIC,
};

class CProjectile : public CEntity
{
public:
	CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, int Explosive, float Force, int SoundImpact, int Weapon, int ExtraInfo = -1);

	vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

private:
	vec2 m_Direction;
	int m_LifeSpan;
	int m_Owner;
	int m_Type;
	int m_Damage;
	int m_SoundImpact;
	int m_Weapon;
	float m_Force;
	int m_StartTick;
	int m_Explosive;
	int m_ExtraInfo;
	
	int m_ElectroTimer;
	
	// so neutral buildings wont hit themselves
	int m_Time;
};

#endif
