#ifndef GAME_SERVER_ENTITIES_SUPEREXPLOSION_H
#define GAME_SERVER_ENTITIES_SUPEREXPLOSION_H


#include <game/server/entity.h>

class CSuperexplosion : public CEntity
{
public:
	static const int ms_PhysSize = 24;
	
	int m_Life;
	int m_MaxLife;
	int m_Player;
	int m_Weapon;
	
	int m_NextIn;
	bool m_Superdamage;
	
	CSuperexplosion(CGameWorld *pGameWorld, vec2 Pos, int Player, int Weapon, int MaxLife, int StartLife = 0, bool Superdamage = false);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
};

#endif
