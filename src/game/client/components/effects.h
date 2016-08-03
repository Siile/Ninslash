

#ifndef GAME_CLIENT_COMPONENTS_EFFECTS_H
#define GAME_CLIENT_COMPONENTS_EFFECTS_H
#include <game/client/component.h>

class CEffects : public CComponent
{
	bool m_Add50hz;
	bool m_Add100hz;
public:
	CEffects();

	virtual void OnRender();

	void BulletTrail(vec2 Start, vec2 End, vec4 Color);
	void BulletTrail(vec2 Pos);
	void SmokeTrail(vec2 Pos, vec2 Vel);
	void SkidTrail(vec2 Pos, vec2 Vel);
	void SpriteSheet(int FX, vec2 Pos);
	void Lazer(vec2 Pos, int Height);
	void Explosion(vec2 Pos);
	void FlameExplosion(vec2 Pos);
	void HammerHit(vec2 Pos);
	void SwordHit(vec2 Pos, float Angle, bool Flip);
	void AirJump(vec2 Pos);
	void Blood(vec2 Pos, vec2 Dir);
	void Triangle(vec2 Pos, vec2 Vel);
	void Flame(vec2 Pos, vec2 Vel, float Alpha = 1.0f);
	void Splatter(vec2 Pos, float Angle, float Size = -1);
	void Spark(vec2 Pos);
	void DamageIndicator(vec2 Pos, vec2 Dir);
	void PlayerSpawn(vec2 Pos);
	void PlayerDeath(vec2 Pos, int ClientID);
	void PowerupShine(vec2 Pos, vec2 Size);
	void Light(vec2 Pos, float Size);
	void Electrospark(vec2 Pos, float Size);

	void Update();
};
#endif
