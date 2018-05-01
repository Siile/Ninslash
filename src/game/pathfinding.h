#ifndef GAME_WAYPOINT_H
#define GAME_WAYPOINT_H


#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>


#define MAX_WAYPOINTS 1000
#define MAX_WAYPOINTCONNECTIONS 10


class CWaypointPath
{
private:
	int m_DistanceToNext;

public:
	CWaypointPath *m_pNext;
	CWaypointPath *m_pParent;
	vec2 m_Pos;
	
	void PushBack(CWaypointPath *P)
	{
		if (m_pNext)
			m_pNext->PushBack(P);
		else
			m_pNext = P;		
	}
	
	CWaypointPath(vec2 Pos, CWaypointPath *Next = 0)
	{
		m_Pos = Pos;
		m_pNext = Next;
		m_pParent = 0;		
		
		if (Next)
			Next->m_pParent = this;
		
		if (m_pNext)
			m_DistanceToNext = distance(Pos, m_pNext->m_Pos);
		else
			m_DistanceToNext = 0;
	}
	
	~CWaypointPath()
	{
		if (m_pNext)
			delete m_pNext;
	}
	
	int Length(int i = 0)
	{
		if (m_pNext)
			//return m_pNext->Length(i+m_DistanceToNext);
			return m_pNext->Length(i+1);
		else
			return i;
	}
	
	CWaypointPath *GetVisible(class CGameContext *pGameServer, vec2 Pos);
};



class CWaypoint
{
private:
	int m_aDistance[MAX_WAYPOINTCONNECTIONS];
	
	
public:
	int m_X, m_Y; // tileset position
	vec2 m_Pos; // world position
	
	// for A*
	bool m_Opened, m_Closed;
	int m_F, m_G, m_H;
	CWaypoint *m_pParent;
	
	int m_Size;
	bool m_ToBeDeleted;
	
	int GetGScore(CWaypoint* P, vec2 Target)
	{
		if (!P)
			return 0;
		
		//return P->m_G + frandom()*500;
		//return P->m_G + 100;
		return P->m_G + distance(m_Pos, Target);
		//return P->m_G + distance(m_Pos, P->m_Pos);
		//return P->m_G + (m_Pos.y > P->m_Pos.y - 40 ? 100 : 130);
	}
	
	void ComputeScores(vec2 TargetPos)
	{
		m_G = GetGScore(m_pParent, TargetPos);
		m_H = distance(m_Pos, TargetPos);
		
		m_F = m_H + m_G;
	}
	
	
	void ClearForAStar()
	{
		m_F = m_G = m_H = 0;
		m_Opened = false; m_Closed = false;
		m_pParent = 0;
	}
	
	
	bool m_InnerCorner;
	
	CWaypoint *m_apConnection[MAX_WAYPOINTCONNECTIONS];
	int m_ConnectionCount;
	
	void SetCenter(int Distance = 1);
	
	// preventing loops and finding the shortest way to target
	int m_PathDistance;
	
	CWaypoint(vec2 Pos, bool InnerCorner = false)
	{
		// A*
		m_F = m_G = m_H = 0;
		m_Opened = false; m_Closed = false;
		m_pParent = 0;
		
		m_InnerCorner = InnerCorner;
		m_PathDistance = 0;
		m_X = Pos.x;
		m_Y = Pos.y;
		m_Pos = vec2(Pos.x*32+16, Pos.y*32+16);
		
		m_Size = -1;
		m_ToBeDeleted = false;
		
		m_ConnectionCount = 0;
		for (int i = 0; i < MAX_WAYPOINTCONNECTIONS; i++)
			m_apConnection[i] = 0;
	}
	
	
	void CalculateAreaSize(int Size)
	{
		//if (Size != -1)
		//	return;
		
		m_Size = Size;
		
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i])
			{
				if (m_apConnection[i]->m_Size == -1)
					m_apConnection[i]->CalculateAreaSize(Size + m_aDistance[i]);
			}
		}
	}

	
	int GetAreaSize()
	{
		int MaxSize = m_Size;
		
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i])
			{
				if (m_apConnection[i]->m_Size > MaxSize)
				{
					int Size = m_apConnection[i]->GetAreaSize();
					//if (MaxSize < Size)
					MaxSize = Size;
				}
			}
		}
		
		return MaxSize;
	}
	
	
	bool Connected(CWaypoint *Waypoint)
	{
		if (!Waypoint || Waypoint == this)
			return false;
		
		// check if we're connected already
		for (int i = 0; i < MAX_WAYPOINTCONNECTIONS; i++)
		{
			if (Waypoint == m_apConnection[i])
				return true;
		}
		
		return false;
	}
	
	void AddWeight(int Weight, bool Kill = false)
	{
		m_PathDistance += Weight;
		
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i])
			{
				if (!Kill)
					m_apConnection[i]->AddWeight(Weight / 2, true);
				else
					m_apConnection[i]->m_PathDistance += Weight;
			}
		}
	}
	
	
	// create a two way connection between this and given waypoint
	bool Connect(CWaypoint *Waypoint)
	{
		if (!Waypoint || Waypoint == this || m_ConnectionCount >= MAX_WAYPOINTCONNECTIONS)
			return false;
		
		// check if we're connected already
		if (Connected(Waypoint))
			return false;
		
		// connect
		m_apConnection[m_ConnectionCount] = Waypoint;
		m_aDistance[m_ConnectionCount] = distance(m_Pos, Waypoint->m_Pos);
		m_ConnectionCount++;
		
		Waypoint->Connect(this);
		return true;
	}
	
	
	
	void ClearConnections()
	{
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i])
			{
				m_apConnection[i]->Unconnect(this);
				m_apConnection[i] = 0;
			}
		}
		
		m_ConnectionCount = 0;
	}
	
	void Unconnect(CWaypoint *Target)
	{
		if (Target == 0)
			return;
		
		for (int i = 0; i < m_ConnectionCount; i++)
		{
			if (m_apConnection[i] == Target)
			{
				m_apConnection[i] = 0;
				Target->Unconnect(this);
			}
		}
	}
	
	
};


#endif
