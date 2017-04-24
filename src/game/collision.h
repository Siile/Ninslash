

#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>
#include "pathfinding.h"




class CCollision
{
	class CTile *m_pTiles;
	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	int SolidState(int x, int y, bool IncludeDeath = false);
	int ForceState(int x, int y);
	
	int GetTile(int x, int y);
	
	int m_WaypointCount;
	int m_ConnectionCount;
	
	void ClearWaypoints();
	
	void RemoveClosedAreas();
	void AddWaypoint(vec2 Position, bool InnerCorner = false);
	CWaypoint *GetWaypointAt(int x, int y);
	void ConnectWaypoints();
	CWaypoint *GetClosestWaypoint(vec2 Pos);

	CWaypoint *m_apWaypoint[MAX_WAYPOINTS];
	CWaypoint *m_pCenterWaypoint;
	
	CWaypointPath *m_pPath;
	
public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_INSTADEATH=4,
		
		COLFLAG_RAMP_LEFT=8,
		COLFLAG_RAMP_RIGHT=16,
		COLFLAG_ROOFSLOPE_LEFT=32,
		COLFLAG_ROOFSLOPE_RIGHT=64,
		COLFLAG_DAMAGEFLUID=128,
		
		// 256 = out of range for unsigned char, ugly!
		COLFLAG_MOVELEFT=129,
		COLFLAG_MOVERIGHT=130,
	};
	
	enum
	{
		SS_NOCOL=0,
		SS_COL=1,
		SS_COL_RL=2,
		SS_COL_RR=3,
		SS_COL_HL=4,
		SS_COL_HR=5,
	};
	
	bool IsTileSolid(int x, int y, bool IncludeDeath = false);
	
	void GenerateWaypoints();
	bool GenerateSomeMoreWaypoints();
	int WaypointCount() { return m_WaypointCount; }
	int ConnectionCount() { return m_ConnectionCount; }
	
	void SetWaypointCenter(vec2 Position);
	void AddWeight(vec2 Pos, int Weight);
	
	vec2 GetRandomWaypointPos();
	
	//CWaypointPath *AStar(vec2 Start, vec2 End);
	bool AStar(vec2 Start, vec2 End);
	
	CWaypointPath *GetPath(){ return m_pPath; }
	void ForgetAboutThePath(){ m_pPath = 0; }

	// for testing
	vec2 m_aPath[99];
	int m_PathLen;
	
	CCollision();
	void Init(class CLayers *pLayers);
	//bool CheckPoint(float x, float y, bool IncludeDeath = false) { return IsTileSolid(round_to_int(x), round_to_int(y), IncludeDeath); }
	
	// -1=left, 0=nope, 1=right
	int IsForceTile(float x, float y){ return ForceState(round_to_int(x), round_to_int(y)); }
	int IsForceTile(vec2 Pos){ return IsForceTile(Pos.x, Pos.y); }
	
	bool IsSawblade(float x, float y);
	bool IsSawblade(vec2 Pos){ return IsSawblade(Pos.x, Pos.y); }
	
	int CheckPoint(float x, float y, bool IncludeDeath = false) { return SolidState(round_to_int(x), round_to_int(y), IncludeDeath); }
	bool CheckPoint(vec2 Pos) { return CheckPoint(Pos.x, Pos.y); }
	int GetCollisionAt(float x, float y) { return GetTile(round_to_int(x), round_to_int(y)); }
	int GetWidth() { return m_Width; };
	int GetHeight() { return m_Height; };
	int FastIntersectLine(vec2 Pos0, vec2 Pos1);
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision, bool IncludeDeath = false);
	
	// return bounced (true) or not (false)
	bool MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces, bool IgnoreCollision = false);
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity, bool check_speed = true);
	int TestBox(vec2 Pos, vec2 Size);

	// MapGen
	bool ModifTile(ivec2 pos, int group, int layer, int tile, int flags, int reserved);
};

#endif
