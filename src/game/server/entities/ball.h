#ifndef GAME_SERVER_ENTITIES_BALL_H
#define GAME_SERVER_ENTITIES_BALL_H

#include <game/server/entity.h>
#include <game/generated/game_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>


class CBall : public CEntity
{
	//MACRO_ALLOC_POOL_ID()

public:
	//character's size
	static const int ms_PhysSize = 24;

	CBall(CGameWorld *pWorld);

	virtual void Reset();
	virtual void Destroy();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	void RoundReset();
	
	bool Spawn(vec2 Pos);
	bool Remove();
	
	void AddForce(vec2 Force);
	
	CBallCore GetCore(){ return m_Core; }
	vec2 GetPosition(){ return m_Core.m_Pos; }
	vec2 GetVel(){ return m_Core.m_Vel; }
	
private:
	// the player core for the physics
	CBallCore m_Core;
	bool m_ForceCoreSend;
	vec2 m_OriginalPos;
	
	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CBallCore m_SendCore; // core that we should send
	CBallCore m_ReckoningCore; // the dead reckoning core
};

#endif
