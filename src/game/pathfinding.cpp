#include "pathfinding.h"


#include <vector>
#include <game/server/gamecontext.h>

using namespace std;


CWaypointPath *CWaypointPath::GetVisible(CGameContext *pGameServer, vec2 Pos)
{
	if (m_pNext)
	{
		if (m_pNext->m_pNext && abs(m_pNext->m_pNext->m_Pos.x-Pos.x) < 1400 && abs(m_pNext->m_pNext->m_Pos.y-Pos.y) < 1400 && !pGameServer->Collision()->IntersectLine(m_pNext->m_pNext->m_Pos, Pos, NULL, NULL))
			return m_pNext->m_pNext->GetVisible(pGameServer, Pos);
			
		if (abs(m_pNext->m_Pos.x-Pos.x) < 4000 && abs(m_pNext->m_Pos.y-Pos.y) < 4000 && !pGameServer->Collision()->IntersectLine(m_pNext->m_Pos, Pos, NULL, NULL))
			return m_pNext->GetVisible(pGameServer, Pos);
	}
	
	return this;
}





void CWaypoint::SetCenter(int Distance)
{
	// set self's distance
	m_PathDistance = Distance;
	
	// set connections' distance
	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			if (m_apConnection[i]->m_PathDistance == 0)
			{
				m_apConnection[i]->m_PathDistance = Distance + m_aDistance[i];
			}
		}
	}
	
	// visit connections
  	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			if (m_apConnection[i]->m_PathDistance >= Distance + m_aDistance[i])
			{
				m_apConnection[i]->SetCenter(Distance + m_aDistance[i]);
			}
		}
   }
}


