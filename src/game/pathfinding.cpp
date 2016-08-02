#include "pathfinding.h"


#include <vector>
#include <game/server/gamecontext.h>

using namespace std;


CWaypointPath *CWaypointPath::GetVisible(CGameContext *pGameServer, vec2 Pos)
{
	if (m_pNext)
	{
		if (!pGameServer->Collision()->FastIntersectLine(m_pNext->m_Pos, Pos))
			return m_pNext->GetVisible(pGameServer, Pos);
	}
	
	return this;
	
	if (!pGameServer->Collision()->FastIntersectLine(m_Pos, Pos))
	{
		return this;
	}
	else
	{
		if (m_pParent)
		{
			//if (!pGameServer->Collision()->FastIntersectLine(m_pParent->m_Pos, Pos))
			//	return m_pParent;
		}
		
	}

	return 0;
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


CWaypointPath *CWaypoint::FindPathToCenter(CWaypointPath *Path)
{
	// not connected
	if (m_PathDistance == 0)
		return NULL;
	
	CWaypointPath *P = Path;
	
	// first
	if (!P)
		P = new CWaypointPath(m_Pos);
	
	if (m_PathDistance == 1)
	{
		return new CWaypointPath(m_Pos, P);
	}
	
	
	// find closest to center
	CWaypoint *Closest = NULL;
	
  	for (int i = 0; i < m_ConnectionCount; i++)
	{
		if (m_apConnection[i])
		{
			if (m_apConnection[i]->m_PathDistance < m_PathDistance)
			{
				if (!Closest)
					Closest = m_apConnection[i];
				else
				{
					if (Closest->m_PathDistance > m_apConnection[i]->m_PathDistance)
						Closest = m_apConnection[i];
				}
			}
		}
	}
	
	if (Closest)
		return Closest->FindPathToCenter(new CWaypointPath(m_Pos, P));
	
	if (Path)
		delete Path;
	
	return NULL;
}
