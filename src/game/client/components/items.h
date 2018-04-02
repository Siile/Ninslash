#ifndef GAME_CLIENT_COMPONENTS_ITEMS_H
#define GAME_CLIENT_COMPONENTS_ITEMS_H
#include <base/system.h>
#include <game/client/component.h>

class CItems : public CComponent
{
	enum
	{
		MAX_EXTRA_PROJECTILES=32,
	};

	CNetObj_Projectile m_aExtraProjectiles[MAX_EXTRA_PROJECTILES];
	int m_NumExtraProjectiles;

	void RenderProjectile(const CNetObj_Projectile *pCurrent, int ItemID);
	void RenderPickup(const CNetObj_Pickup *pPrev, const CNetObj_Pickup *pCurrent);
	void RenderFlag(const CNetObj_Flag *pPrev, const CNetObj_Flag *pCurrent, const CNetObj_GameData *pPrevGameData, const CNetObj_GameData *pCurGameData);
	void RenderLaser(const struct CNetObj_Laser *pCurrent);
	void RenderLaserFail(const struct CNetObj_LaserFail *pCurrent);
	
	// flag animation
	int64 m_LastUpdate[2];
	int64 m_LastTiltUpdate[2];
	
	int m_FlagFrame[2];
	
	bool m_FlagMirror[2];
	vec2 m_FlagPos[2];
	vec2 m_FlagOldPos[2];
	
	vec2 m_FlagOffset[2];
	vec2 m_FlagTargetOffset[2];
	
	float m_FlagTilt[2];
	
	void UpdateProjectileTrace(const CNetObj_Projectile *pCurrent, int ItemID);
	
public:
	virtual void OnReset();
	virtual void OnRender();

	void UpdateTraces();
	void AddExtraProjectile(CNetObj_Projectile *pProj);
};

#endif
