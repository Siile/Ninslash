#include <list>
#include <vector>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <math.h>
#include <engine/map.h>
#include <engine/kernel.h>

#include <game/mapitems.h>
#include <game/layers.h>
#include <game/collision.h>



using namespace std;



	
bool CCollision::AStar(vec2 Start, vec2 End)
{
	vector<CWaypoint*> path;
	
    // Define points to work with
	CWaypoint *StartWP = GetClosestWaypoint(Start);
	CWaypoint *EndWP = GetClosestWaypoint(End);
	CWaypoint *CurrentWP = NULL;
	CWaypoint *ChildWP = NULL;
	
	if (!StartWP || !EndWP)
		return false;
	
	
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (m_apWaypoint[i])
			m_apWaypoint[i]->ClearForAStar();
	}
	
	
	// Define the open and the close list
	list<CWaypoint*> openList;
	list<CWaypoint*> closedList;
	list<CWaypoint*>::iterator i;

	unsigned int n = 0;
	
    // Add the start point to the openList
    openList.push_back(StartWP);
    StartWP->m_Opened = true;

	
	// while (n == 0 || (CurrentWP != EndWP && n < 400))

	while (n < 2000)
    {
        // Look for the smallest F value in the openList and make it the current point
        for (i = openList.begin(); i != openList.end(); ++ i)
        {
            if (i == openList.begin() || (*i)->m_F <= CurrentWP->m_F)
            {
                CurrentWP = (*i);
            }
        }

        // Stop if we reached the end
        if (CurrentWP == EndWP)
            break;

        // Remove the current point from the openList
        openList.remove(CurrentWP);
        CurrentWP->m_Opened = false;

        // Add the current point to the closedList
        closedList.push_back(CurrentWP);
        CurrentWP->m_Closed = true;


		// Get all current's adjacent walkable points
		for (int w = 0; w < CurrentWP->m_ConnectionCount; w++)
		{
			// Get this point
			ChildWP = CurrentWP->m_apConnection[w];
			if (!ChildWP)
				continue;

			if (ChildWP->m_Closed)
				continue;
			
			if (m_GlobalAcid && GetGlobalAcidLevel() < ChildWP->m_Pos.y)
				continue;

			// If it's already in the openList
			if (ChildWP->m_Opened)
			{
				// If it has a worse g score than the one that pass through the current point
				// then its path is improved when it's parent is the current point
				if (ChildWP->m_G > ChildWP->GetGScore(CurrentWP, EndWP->m_Pos))
				{
					// Change its parent and g score
					ChildWP->m_pParent = CurrentWP;
					ChildWP->ComputeScores(EndWP->m_Pos);
				}
			}
			else
			{
				// Add it to the openList with current point as parent
				openList.push_back(ChildWP);
				ChildWP->m_Opened = true;

				// Compute it's g, h and f score
				ChildWP->m_pParent = CurrentWP;
				ChildWP->ComputeScores(EndWP->m_Pos);
			}
		}

        n++;
    }

    // Reset
    for (i = openList.begin(); i != openList.end(); ++ i)
    {
        (*i)->m_Opened = false;
    }
    for (i = closedList.begin(); i != closedList.end(); ++ i)
    {
        (*i)->m_Closed = false;
    }

	if (m_pPath)
	{
		delete m_pPath;
		m_pPath = NULL;
	}

	
    // Resolve the path starting from the end point
    while (CurrentWP->m_pParent && CurrentWP != StartWP)
    {
		/*
		if (!m_pPath)
			m_pPath = new CWaypointPath(CurrentWP->m_Pos);
		else
			m_pPath->PushBack(new CWaypointPath(CurrentWP->m_Pos));
		*/
		
		//m_pPath = new CWaypointPath(CurrentWP->m_Pos, m_pPath);
		
		if (!m_pPath)
			m_pPath = new CWaypointPath(CurrentWP->m_Pos);
		else
			m_pPath->Add(CurrentWP->m_Pos);
		
        //path.push_back(CurrentWP->getPosition());
        CurrentWP = CurrentWP->m_pParent;
        n++;
    }

	// for displaying the chosen waypoints
	for (int w = 0; w < 99; w++)
		m_aPath[w] = vec2(0, 0);
		
	if (m_pPath)
	{
		CWaypointPath *Wp = m_pPath;

		for (int w = 0; w < 80; w++)
		{
			m_PathLen = w;
			m_aPath[w] = vec2(Wp->m_Pos.x, Wp->m_Pos.y);
				
			if (Wp->m_pNext)
				Wp = Wp->m_pNext;
			else
				break;
		}
	}
	
	
	if (m_pPath)
		return true;
	else
		return false;
}

