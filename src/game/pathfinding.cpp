#include "pathfinding.h"


#include <vector>
#include <game/server/gamecontext.h>

using namespace std;


CWaypointPath *CWaypointPath::GetVisible(CGameContext *pGameServer, vec2 Pos)
{
	if (m_pNext)
	{
		if (m_pNext->m_pNext && abs(m_pNext->m_pNext->m_Pos.x-Pos.x) < 1400 && abs(m_pNext->m_pNext->m_Pos.y-Pos.y) < 1400 && !pGameServer->Collision()->IntersectLine(m_pNext->m_pNext->m_Pos, Pos, NULL, NULL, true, false, false))
			return m_pNext->m_pNext->GetVisible(pGameServer, Pos);
			
		if (abs(m_pNext->m_Pos.x-Pos.x) < 4000 && abs(m_pNext->m_Pos.y-Pos.y) < 4000 && !pGameServer->Collision()->IntersectLine(m_pNext->m_Pos, Pos, NULL, NULL, true, false, false))
			return m_pNext->GetVisible(pGameServer, Pos);
	}
	
	return this;
}
