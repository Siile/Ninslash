


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


CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;

	m_PathLen = 0;
	m_pPath = 0;
	m_pCenterWaypoint = 0;
	
	for (int i = 0; i < MAX_WAYPOINTS; i++)
		m_apWaypoint[i] = 0;
}

void CCollision::Init(class CLayers *pLayers)
{
	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));
	
	m_LowestPoint = 0;
	m_Time = 0;
	m_GlobalAcid = true;
	
	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index = m_pTiles[i].m_Index;

		if(Index > 128)
			continue;
		m_pTiles[i].m_Index = 0;

		switch(Index)
		{
		case TILE_DEATH:
			m_pTiles[i].m_Index = COLFLAG_DEATH;
			break;
		case TILE_SOLID:
			m_pTiles[i].m_Index = COLFLAG_SOLID;
			break;
		case TILE_INSTADEATH:
			m_pTiles[i].m_Index = COLFLAG_INSTADEATH;
			break;
		case TILE_RAMP_LEFT:
			m_pTiles[i].m_Index |= COLFLAG_RAMP_LEFT;
			break;
		case TILE_RAMP_RIGHT:
			m_pTiles[i].m_Index |= COLFLAG_RAMP_RIGHT;
			break;
		case TILE_ROOFSLOPE_LEFT:
			m_pTiles[i].m_Index |= COLFLAG_ROOFSLOPE_LEFT;
			break;
		case TILE_ROOFSLOPE_RIGHT:
			m_pTiles[i].m_Index |= COLFLAG_ROOFSLOPE_RIGHT;
			break;
		case TILE_DAMAGEFLUID:
			m_pTiles[i].m_Index = COLFLAG_DAMAGEFLUID;
			break;
		case TILE_MOVELEFT:
			m_pTiles[i].m_Index = COLFLAG_MOVELEFT;
			break;
		case TILE_MOVERIGHT:
			m_pTiles[i].m_Index = COLFLAG_MOVERIGHT;
			break;
		case TILE_HANG:
			m_pTiles[i].m_Index = COLFLAG_HANG;
			break;
		default:
			m_pTiles[i].m_Index = 0;
		}
	}
	
	m_pCenterWaypoint = 0;
	
	for (int i = 0; i < MAX_WAYPOINTS; i++)
		m_apWaypoint[i] = 0;
}




vec2 CCollision::GetRandomWaypointPos()
{
	int n = 0;
	while (n++ < 10)
	{
		int i = rand()%m_WaypointCount;
	
		if (m_apWaypoint[i])
			return m_apWaypoint[i]->m_Pos;
	}
	
	return vec2(0, 0);
}



void CCollision::ClearWaypoints()
{
	m_WaypointCount = 0;
	
	for (int i = 0; i < MAX_WAYPOINTS; i++)
	{
		if (m_apWaypoint[i])
			delete m_apWaypoint[i];
		
		m_apWaypoint[i] = NULL;
	}
	
	m_pCenterWaypoint = NULL;
}

/*
void CCollision::WaypointAreaSize(CWaypoint *Area)
{
	
	
}
*/

void CCollision::RemoveClosedAreas()
{
	return;
	for (int i = 0; i < MAX_WAYPOINTS; i++)
	{
		if (m_apWaypoint[i])
		{
			m_apWaypoint[i]->CalculateAreaSize(0);
			
			if (m_apWaypoint[i]->GetAreaSize() < 1000)
			{
				m_apWaypoint[i]->m_ToBeDeleted = true;
			}
		}
	}
	
	for (int i = 0; i < MAX_WAYPOINTS; i++)
	{
		if (m_apWaypoint[i] && m_apWaypoint[i]->m_ToBeDeleted)
		{
			m_WaypointCount--;
			m_apWaypoint[i]->ClearConnections();
			delete m_apWaypoint[i];
			m_apWaypoint[i] = NULL;
		}
	}
}



void CCollision::AddWaypoint(vec2 Position, bool InnerCorner)
{
	if (m_WaypointCount >= MAX_WAYPOINTS)
		return;
	
	m_apWaypoint[m_WaypointCount] = new CWaypoint(Position, InnerCorner);
	m_WaypointCount++;
}





void CCollision::GenerateWaypoints()
{
	ClearWaypoints();
	for(int x = 2; x < m_Width-2; x++)
	{
		for(int y = 2; y < m_Height-2; y++)
		{
			if (m_pTiles[y*m_Width+x].m_Index && m_pTiles[y*m_Width+x].m_Index < 130)
				continue;

			if (IsSawblade(x*32, y*32) || 
				IsSawblade((x-1)*32, (y-1)*32) ||
				IsSawblade((x+0)*32, (y-1)*32) ||
				IsSawblade((x+1)*32, (y-1)*32) ||
				IsSawblade((x-1)*32, (y-0)*32) ||
				IsSawblade((x+0)*32, (y-0)*32) ||
				IsSawblade((x+1)*32, (y-0)*32) ||
				IsSawblade((x-1)*32, (y+1)*32) ||
				IsSawblade((x+0)*32, (y+1)*32) ||
				IsSawblade((x+1)*32, (y+1)*32))
				continue;
			
			// find all outer corners
			if ((IsTileSolid((x-1)*32, (y-1)*32) && !IsTileSolid((x-1)*32, (y-0)*32) && !IsTileSolid((x-0)*32, (y-1)*32)) ||
				(IsTileSolid((x-1)*32, (y+1)*32) && !IsTileSolid((x-1)*32, (y-0)*32) && !IsTileSolid((x-0)*32, (y+1)*32)) ||
				(IsTileSolid((x+1)*32, (y+1)*32) && !IsTileSolid((x+1)*32, (y-0)*32) && !IsTileSolid((x-0)*32, (y+1)*32)) ||
				(IsTileSolid((x+1)*32, (y-1)*32) && !IsTileSolid((x+1)*32, (y-0)*32) && !IsTileSolid((x-0)*32, (y-1)*32)))
			{
				// outer corner found -> create a waypoint
				
				// check validity (solid tiles under the corner)
				/*
				bool Found = false;
				for (int i = 0; i < 20; ++i)
					if (IsTileSolid(x*32, (y+i)*32))
						Found = true;
					*/
					
				bool Found = true;
				
				// count slopes
				int Slopes = 0;
				
				for (int xx = -2; xx <= 2; xx++)
					for (int yy = -2; yy <= 2; yy++)
						if (GetTile((x+xx)*32, (y+yy)*32) >= COLFLAG_RAMP_LEFT) Slopes++;
				
				if (Found && Slopes < 3)
					AddWaypoint(vec2(x, y));
			}
			else
			// find all inner corners
			if ((IsTileSolid((x+1)*32, y*32) || IsTileSolid((x-1)*32, y*32)) && (IsTileSolid(x*32, (y-1)*32) || IsTileSolid(x*32, (y+1)*32)))
			{
				// inner corner found -> create a waypoint
				//AddWaypoint(vec2(x, y), true);
				
				// check validity (solid tiles under the corner)
				bool Found = false;
				for (int i = 0; i < 20; ++i)
					if (IsTileSolid(x*32, (y+i)*32))
						Found = true;
					
				// count slopes
				int Slopes = 0;
				
				for (int xx = -2; xx <= 2; xx++)
					for (int yy = -2; yy <= 2; yy++)
						if (GetTile((x+xx)*32, (y+yy)*32) >= COLFLAG_RAMP_LEFT) Slopes++;
				
				// too tight spots to go
				if ((IsTileSolid((x)*32, (y-1)*32) && IsTileSolid((x)*32, (y+1)*32)) ||
					(IsTileSolid((x-1)*32, (y)*32) && IsTileSolid((x+1)*32, (y)*32)))
					Found = false;
				
				if (Found && Slopes < 3)
					AddWaypoint(vec2(x, y));
			}
		}
	}
		
	bool KeepGoing = true;
	int i = 0;
	
	while (KeepGoing && i++ < 10)
	{
		ConnectWaypoints();
		KeepGoing = GenerateSomeMoreWaypoints();
	}
	ConnectWaypoints();
	
	RemoveClosedAreas();
}


// create a new waypoints between connected, far apart ones
bool CCollision::GenerateSomeMoreWaypoints()
{
	bool Result = false;
	
	for (int i = 0; i < m_WaypointCount; i++)
	{
		for (int j = 0; j < m_WaypointCount; j++)
		{
			if (m_apWaypoint[i] && m_apWaypoint[j] && m_apWaypoint[i]->Connected(m_apWaypoint[j]))
			{
				if (abs(m_apWaypoint[i]->m_X - m_apWaypoint[j]->m_X) > 20 && m_apWaypoint[i]->m_Y == m_apWaypoint[j]->m_Y)
				{
					int x = (m_apWaypoint[i]->m_X + m_apWaypoint[j]->m_X) / 2;
					
					if (IsTileSolid(x*32, (m_apWaypoint[i]->m_Y+1)*32) || IsTileSolid(x*32, (m_apWaypoint[i]->m_Y-1)*32))
					{
						AddWaypoint(vec2(x, m_apWaypoint[i]->m_Y));
						Result = true;
					}
				}
				
				if (abs(m_apWaypoint[i]->m_Y - m_apWaypoint[j]->m_Y) > 30 && m_apWaypoint[i]->m_X == m_apWaypoint[j]->m_X)
				{
					int y = (m_apWaypoint[i]->m_Y + m_apWaypoint[j]->m_Y) / 2;
					
					if (IsTileSolid((m_apWaypoint[i]->m_X+1)*32, y*32) || IsTileSolid((m_apWaypoint[i]->m_X-1)*32, y*32))
					{
						AddWaypoint(vec2(m_apWaypoint[i]->m_X, y));
						Result = true;
					}
				}
				
				m_apWaypoint[i]->Unconnect(m_apWaypoint[j]);
			}
		}
	}
	
	return Result;
}



CWaypoint *CCollision::GetWaypointAt(int x, int y)
{
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (m_apWaypoint[i])
		{
			if (m_apWaypoint[i]->m_X == x && m_apWaypoint[i]->m_Y == y)
				return m_apWaypoint[i];
		}
	}
	return NULL;
}


void CCollision::ConnectWaypoints()
{
	m_ConnectionCount = 0;
	
	// clear existing connections
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (!m_apWaypoint[i])
			continue;
		
		m_apWaypoint[i]->ClearConnections();
	}
		
	
		
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (!m_apWaypoint[i])
			continue;
		
		int x, y;
		
		x = m_apWaypoint[i]->m_X - 1;
		y = m_apWaypoint[i]->m_Y;
		
		// find waypoints at left
		while (!m_pTiles[y*m_Width+x].m_Index || m_pTiles[y*m_Width+x].m_Index >= 128)
		{
			CWaypoint *W = GetWaypointAt(x, y);
			
			if (W)
			{
				if (m_apWaypoint[i]->Connect(W))
					m_ConnectionCount++;
				break;
			}
			
			//if (!IsTileSolid(x*32, (y-1)*32) && !IsTileSolid(x*32, (y+1)*32))
			if (!IsTileSolid(x*32, (y+1)*32))
				break;
			
			x--;
		}
		
		x = m_apWaypoint[i]->m_X;
		y = m_apWaypoint[i]->m_Y - 1;
		
		int n = 0;
		
		// find waypoints at up
		//bool SolidFound = false;
		while ((!m_pTiles[y*m_Width+x].m_Index || m_pTiles[y*m_Width+x].m_Index >= 128) && n++ < 10)
		{
			CWaypoint *W = GetWaypointAt(x, y);
			
			//if (IsTileSolid((x+1)*32, y*32) || IsTileSolid((x+1)*32, y*32))
			//	SolidFound = true;
			
			//if (W && SolidFound)
			if (W)
			{
				if (m_apWaypoint[i]->Connect(W))
					m_ConnectionCount++;
				break;
			}
			
			y--;
		}
	}
	
	
	// connect to near, visible waypoints
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (m_apWaypoint[i] && m_apWaypoint[i]->m_InnerCorner)
			continue;
		
		for (int j = 0; j < m_WaypointCount; j++)
		{
			if (m_apWaypoint[j] && m_apWaypoint[j]->m_InnerCorner)
				continue;
			
			if (m_apWaypoint[i] && m_apWaypoint[j] && !m_apWaypoint[i]->Connected(m_apWaypoint[j]))
			{
				float Dist = distance(m_apWaypoint[i]->m_Pos, m_apWaypoint[j]->m_Pos);
				
				if (Dist < 1000 && !IntersectLine(m_apWaypoint[i]->m_Pos, m_apWaypoint[j]->m_Pos, NULL, NULL, true) &&
					m_apWaypoint[i]->m_Pos.y != m_apWaypoint[j]->m_Pos.y)
				{
					if (m_apWaypoint[i]->Connect(m_apWaypoint[j]))
						m_ConnectionCount++;
				}
			}
		}
	}
}

vec2 CCollision::GetClosestWaypointPos(vec2 Pos)
{
	CWaypoint *Wp = GetClosestWaypoint(Pos);
	
	if (Wp)
		return Wp->m_Pos;
	
	return vec2(0, 0);
}

CWaypoint *CCollision::GetClosestWaypoint(vec2 Pos)
{
	CWaypoint *W = NULL;
	float Dist = 9000;
	
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (m_apWaypoint[i])
		{
			if (m_GlobalAcid && GetGlobalAcidLevel() < Pos.y)
				continue;
			
			int d = distance(m_apWaypoint[i]->m_Pos, Pos);
			
			if (d < Dist && d < 800)
			{
				if (!FastIntersectLine(m_apWaypoint[i]->m_Pos, Pos) || Dist == 9000)
				{
					W = m_apWaypoint[i];
					Dist = d;
				}
			}
		}
	}
	
	return W;
}


void CCollision::SetWaypointCenter(vec2 Position)
{
	m_pCenterWaypoint = GetClosestWaypoint(Position);
	
	// clear path weights
	for (int i = 0; i < m_WaypointCount; i++)
	{
		if (m_apWaypoint[i])
			m_apWaypoint[i]->m_PathDistance = 0;
	}
	
	if (m_pCenterWaypoint)
		m_pCenterWaypoint->SetCenter();
	
}


void CCollision::AddWeight(vec2 Pos, int Weight)
{
	CWaypoint *Wp = GetClosestWaypoint(Pos);
	
	if (Wp)
		Wp->AddWeight(Weight);
}


int CCollision::GetTile(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

	if (m_pTiles[Ny*m_Width+Nx].m_Index == ENTITY_SAWBLADE + ENTITY_OFFSET)
		return COLFLAG_SOLID;
	
	if (m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_MOVELEFT || m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_MOVERIGHT)
		return COLFLAG_SOLID;
	
	return m_pTiles[Ny*m_Width+Nx].m_Index > 128 ? 0 : m_pTiles[Ny*m_Width+Nx].m_Index;
}


int CCollision::GetLowestPoint()
{
	if (!m_LowestPoint)
		for (int y = m_Height-1; y > 0; y--)
			for (int x = 0; x < m_Width; x++)
			{
				if (!IsTileSolid(x*32, y*32))
				{
					m_LowestPoint = (y+1)*32;
					return m_LowestPoint;
				}
			}
		
	return m_LowestPoint;
}
	

float CCollision::GetGlobalAcidLevel()
{
	return GetLowestPoint() + m_Time;
}


int CCollision::ForceState(int x, int y)
{
	int Nx = clamp(x/32, 0, m_Width-1);
	int Ny = clamp(y/32, 0, m_Height-1);

	if (m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_MOVELEFT)
		return -1;
	
	if (m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_MOVERIGHT)
		return 1;
	
	return 0;
}


bool CCollision::IsHangTile(float x, float y)
{
	int Nx = clamp(round_to_int(x)/32, 0, m_Width-1);
	int Ny = clamp(round_to_int(y)/32, 0, m_Height-1);
	
	if (m_pTiles[Ny*m_Width+Nx].m_Index == COLFLAG_HANG)
		return true;
	
	return false;
}

bool CCollision::IsSawblade(float x, float y)
{
	int Nx = clamp(round_to_int(x)/32, 0, m_Width-1);
	int Ny = clamp(round_to_int(y)/32, 0, m_Height-1);
	
	if (m_pTiles[Ny*m_Width+Nx].m_Index == ENTITY_SAWBLADE + ENTITY_OFFSET)
		return true;
	
	return false;
}


int CCollision::SolidState(int x, int y, bool IncludeDeath)
{
	unsigned char sol = GetTile(x, y);

	if(sol& COLFLAG_SOLID || (IncludeDeath && (sol&COLFLAG_DEATH || sol&COLFLAG_INSTADEATH)))
		return true;
	else if(sol&COLFLAG_RAMP_LEFT) {
		//return ((31-x%32) > (31-y%32));
		return ((31-x%32) > (31-y%32) ? SS_COL : ((31-x%32) == (31-y%32) ? SS_COL_RL : SS_NOCOL));
	}
	else if(sol&COLFLAG_RAMP_RIGHT) {
		//return (x%32 > (31-y%32));
		return (x%32 > (31-y%32) ? SS_COL : (x%32 == (31-y%32) ? SS_COL_RR : SS_NOCOL));
	}
	else if(sol&COLFLAG_ROOFSLOPE_LEFT) {
		//return ((31-x%32)> y%32);
		return ((31-x%32) > y%32 ? SS_COL : ((31-x%32) == y%32 ? SS_COL_HL : SS_NOCOL));
	}
	else if(sol&COLFLAG_ROOFSLOPE_RIGHT) {
		return (x%32 > y%32 ? SS_COL : (x%32 == y%32 ? SS_COL_HR : SS_NOCOL));
	}
	else
		return 0;
	//return GetTile(x, y)&COLFLAG_SOLI
}


bool CCollision::IsTileSolid(int x, int y, bool IncludeDeath)
{
	/*
	int t = GetTile(x, y);
	if (IncludeDeath && GetTile(x, y)&COLFLAG_DEATH)
		return true;
	
	return GetTile(x, y)&COLFLAG_SOLID;
	*/
	
	return SolidState(x, y) != SS_NOCOL;
}



int CCollision::IsInFluid(float x, float y)
{
	if (m_GlobalAcid && y > GetGlobalAcidLevel())
		return true;
	
	return GetTile(round_to_int(x), round_to_int(y)) == CCollision::COLFLAG_DAMAGEFLUID;
}

int CCollision::FastIntersectLine(vec2 Pos0, vec2 Pos1)
{
	//float Distance = distance(Pos0, Pos1) / 4.0f;
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
			return GetCollisionAt(Pos.x, Pos.y);
	}
	return 0;
}


// TODO: rewrite this smarter!
int CCollision::IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, bool IncludeDeath)
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);
	vec2 Last = Pos0;

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y, IncludeDeath))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(Pos.x, Pos.y);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

// TODO: OPT: rewrite this smarter!
bool CCollision::MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces, bool IgnoreCollision)
{
	bool Bounced = false;
	
	if(pBounces)
		*pBounces = 0;

	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	if(!IgnoreCollision && CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
			
			Bounced = true;
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				(*pBounces)++;
			Affected++;
			
			Bounced = true;
		}

		if(Affected == 0)
		{
			pInoutVel->x *= -Elasticity;
			pInoutVel->y *= -Elasticity;
		}
	}
	else
	{
		*pInoutPos = Pos + Vel;
	}
	
	return Bounced;
}

int CCollision::TestBox(vec2 Pos, vec2 Size)
{
	Size *= 0.5f;
	int r;
	for(int x = 0; x <= Size.x; x++) {
		if( (r = CheckPoint(Pos.x+x, Pos.y-Size.y)) )
			return r;
		if( (r = CheckPoint(Pos.x+x, Pos.y+Size.y)) )
			return r;
		
		if( (r = CheckPoint(Pos.x-x, Pos.y-Size.y)) )
			return r;
		if( (r = CheckPoint(Pos.x-x, Pos.y+Size.y)) )
			return r;
	}
	
	for(int y = 0; y <= Size.y; y++) {
		int r;
		if( (r = CheckPoint(Pos.x-Size.x, Pos.y+y)) )
			return r;
		if( (r = CheckPoint(Pos.x+Size.x, Pos.y+y)) )
			return r;
		
		if( (r = CheckPoint(Pos.x-Size.x, Pos.y-y)) )
			return r;
		if( (r = CheckPoint(Pos.x+Size.x, Pos.y-y)) )
			return r;
	}
			
	/*if(CheckPoint(Pos.x-Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x-Size.x, Pos.y+Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y+Size.y))
		return true;*/
	return 0;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity, bool check_speed)
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;

	float Distance = length(Vel);
	int Max = (int)Distance;

	if(Distance > 0.00001f)
	{
		//vec2 old_pos = pos;
		float Fraction = 1.0f/(float)(Max+1);
		for(int i = 0; i <= Max; i++)
		{
			//float amount = i/(float)max;
			//if(max == 0)
				//amount = 0;

			vec2 NewPos = Pos + Vel*Fraction; // TODO: this row is not nice
			int rr = TestBox(vec2(NewPos.x, NewPos.y), Size);
			
			/*if (rr == SS_COL_RL || rr == SS_COL_RR) {
				std::cerr << "COL: " << rr << std::endl;
				int r = 0;
				if(rr == SS_COL_RL) {
					//Vel.x *= invsqrt2;
					Vel.y = Vel.x;
				}
				else if(rr == SS_COL_RR) {
					//Vel.x *= invsqrt2;
					Vel.y = -Vel.x;
				}
				//NewPos = Pos;
			}
			else*/ if(rr)
			{
				int Hits = 0;
				int r = 0;
				
				if( (r = TestBox(vec2(Pos.x, NewPos.y), Size)) )
				{
					//bool taken_care = false;
					NewPos.y = Pos.y;
					if(r == SS_COL_RR && Vel.x >= -Vel.y && (!check_speed || fabs(Vel.x) > 0.005f)) {
						float new_force = Vel.x * invsqrt2 - Vel.y * invsqrt2; 
						//if(new_force/Distance < 0.95f) {
							Vel.y = -new_force * invsqrt2;
							Vel.x = new_force  * invsqrt2;
							//std::cerr << "C1 " << new_force/Distance << std::endl;
							//taken_care = true;
						//}
					}
					else if(r == SS_COL_RL && Vel.x <= Vel.y && (!check_speed || fabs(Vel.x) > 0.005f)) {
						float new_force = -Vel.x * invsqrt2 - Vel.y * invsqrt2;
						//std::cerr << "C2pre " << Vel.x << ", " << check_speed << std::endl;
						//if(new_force/Distance < 0.95f) {
							Vel.y = -new_force * invsqrt2;
							Vel.x = -new_force * invsqrt2;
							//std::cerr << "C2 " << new_force/Distance << std::endl;
							//taken_care = true;
						//}
					}
					else
						Vel.y *= -Elasticity;
					Hits++;
					//Vel.y *= -Elasticity;
					//NewPos.y = Pos.y;
				}

				if( (r = TestBox(vec2(NewPos.x, Pos.y), Size)) )
				{
					
					/*bool climbing = false;
					//std::cerr << "Oh" << std::endl;
					for(int y = 1; y <= 2; y++) {
						if(!TestBox(vec2(NewPos.x, NewPos.y-y), Size)) {
							//std::cerr << "WUI " << y << std::endl;
							NewPos = vec2(NewPos.x, NewPos.y-y);
							climbing = true;
							break;
						}
					}
					if(!climbing) {*/
					//bool taken_care = false;
					NewPos.x = Pos.x;
					if(r == SS_COL_RR && Vel.x >= -Vel.y && (!check_speed || fabs(Vel.x) > 0.005f)) {
						float new_force = Vel.x * invsqrt2 - Vel.y * invsqrt2; 
						//if(new_force/Distance < 0.95f) {
							Vel.y = -new_force * invsqrt2;
							Vel.x = new_force  * invsqrt2;
							//std::cerr << "D1 " << new_force/Distance << std::endl;
							//taken_care = true;
						//}
					}
					else if(r == SS_COL_RL && Vel.x <= Vel.y && (!check_speed || fabs(Vel.x) > 0.005f)) {
						float new_force = -Vel.x * invsqrt2 - Vel.y * invsqrt2;
						//if(new_force/Distance < 0.95f) {
							Vel.y = -new_force * invsqrt2;
							Vel.x = -new_force * invsqrt2;
							//std::cerr << "D2 " << new_force/Distance << std::endl;
							//taken_care = true;
						//}
					}
					else
						Vel.x *= -Elasticity;
							
					//} else {
						//Vel.x *= 0.85f;
						//float newvely = -abs(Vel.x/2.0f);
						
						//if(Vel.y > newvely)
						//	Vel.y = newvely;
					//}
					
					Hits++;
					//Vel.x *= -Elasticity;
					//NewPos.x = Pos.x;
				}

				// neither of the tests got a collision.
				// this is a real _corner case_!
				if(Hits == 0)
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
				}
			}
			/*else if(rr != 0) {
				NewPos.y = Pos.y;
				Vel.y *= -Elasticity;
				NewPos.x = Pos.x;
				Vel.x *= -Elasticity;
			}*/
			
			Pos = NewPos;
		}
	}
	
	/*if(Vel.y >= 0) {
		bool was_hitting = false;
		for(int y = 3; y >= 0; y--) {
			bool hitting = TestBox(vec2(Pos.x, Pos.y+y), Size);
			if(!hitting && was_hitting) {
				Pos.y+=y;
				break;
			}
			else if (hitting) {
				was_hitting = true;
			}
		}
	}*/

	*pInoutPos = Pos;
	*pInoutVel = Vel;
}

bool CCollision::ModifTile(ivec2 pos, int group, int layer, int tile, int flags, int reserved)
{
    CMapItemGroup *pGroup = m_pLayers->GetGroup(group);
    CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer+layer);
    if (pLayer->m_Type != LAYERTYPE_TILES)
        return false;

    CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);
    int TotalTiles = pTilemap->m_Width*pTilemap->m_Height;
    int tpos = (int)pos.y*pTilemap->m_Width+(int)pos.x;
    if (tpos < 0 || tpos >= TotalTiles)
        return false;


    if (pTilemap != m_pLayers->GameLayer())
    {
        CTile *pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(pTilemap->m_Data));
        pTiles[tpos].m_Flags = flags;
        pTiles[tpos].m_Index = tile;
        pTiles[tpos].m_Reserved = reserved;
    }
    else
    {
        m_pTiles[tpos].m_Index = tile;
        m_pTiles[tpos].m_Flags = flags;
        m_pTiles[tpos].m_Reserved = reserved;

        switch(tile)
        {
        case TILE_DEATH:
            m_pTiles[tpos].m_Index = COLFLAG_DEATH;
            break;
        case TILE_SOLID:
            m_pTiles[tpos].m_Index = COLFLAG_SOLID;
            break;
        case TILE_DAMAGEFLUID:
            m_pTiles[tpos].m_Index = COLFLAG_DAMAGEFLUID;
            break;
        case TILE_MOVELEFT:
            m_pTiles[tpos].m_Index = COLFLAG_MOVELEFT;
            break;
        case TILE_MOVERIGHT:
            m_pTiles[tpos].m_Index = COLFLAG_MOVERIGHT;
            break;
        case TILE_RAMP_LEFT:
            m_pTiles[tpos].m_Index = COLFLAG_RAMP_LEFT;
            break;
        case TILE_RAMP_RIGHT:
            m_pTiles[tpos].m_Index = COLFLAG_RAMP_RIGHT;
            break;
		case TILE_ROOFSLOPE_LEFT:
			m_pTiles[tpos].m_Index = COLFLAG_ROOFSLOPE_LEFT;
			break;
		case TILE_ROOFSLOPE_RIGHT:
			m_pTiles[tpos].m_Index = COLFLAG_ROOFSLOPE_RIGHT;
			break;
        case TILE_HANG:
            m_pTiles[tpos].m_Index = COLFLAG_HANG;
            break;
        default:
            if(tile <= 128)
                m_pTiles[tpos].m_Index = 0;
        }
    }

    return true;
}
