#ifndef GAME_SERVER_MAPGEN_GEN_LAYER_H
#define GAME_SERVER_MAPGEN_GEN_LAYER_H

#include <base/vmath.h>

#define GEN_MAX 499

class CGenLayer
{
private:
	int *m_pTiles;
	int *m_pBGTiles;
	int *m_pDoodadsTiles;
	int *m_pObjectTiles;
	int *m_pFlags;
	int *m_pBGFlags;
	int *m_pDoodadsFlags;
	int *m_pObjectFlags;
	int m_Width;
	int m_Height;
	int m_Size;

	ivec2 m_aPlatform[GEN_MAX];
	int m_NumPlatforms;
	
	ivec2 m_aOpenArea[GEN_MAX];
	int m_NumOpenAreas;
	
	// inner top corners
	ivec2 m_aTopCorner[GEN_MAX];
	int m_NumTopCorners;
	
	// sharp corners
	ivec2 m_aCorner[GEN_MAX];
	int m_NumCorners;
	
	// acid pools
	ivec4 m_aPit[GEN_MAX];
	int m_NumPits;
	
	ivec2 m_aWall[GEN_MAX];
	int m_NumWalls;
	
	ivec2 m_aCeiling[GEN_MAX];
	int m_NumCeilings;
	
	ivec3 m_aLongCeiling[GEN_MAX];
	int m_NumLongCeilings;
	
	ivec3 m_aLongPlatform[GEN_MAX];
	int m_NumLongPlatforms;
	
	ivec2 m_aPlayerSpawn[GEN_MAX];
	int m_NumPlayerSpawns;
	
public:
	CGenLayer(int w, int h);
	~CGenLayer();
	
	enum Layer
	{
		FOREGROUND,
		BACKGROUND,
		DOODADS,
		FGOBJECTS,
	};
	
	void Set(int Tile, int x, int y, int Flags = 0, int Layer = FOREGROUND);
	int GetByIndex(int Index, int Layer = FOREGROUND);
	int Get(int x, int y, int Layer = FOREGROUND);
	int GetFlags(int x, int y, int Layer = FOREGROUND);
	bool Used(int x, int y);
	
	bool IsFloor(int x, int y);
	
	void Use(int x, int y);
	
	int Width() { return m_Width; }
	int Height() { return m_Height; }
	
	int NumPlatforms() { return m_NumPlatforms; }
	int NumTopCorners() { return m_NumTopCorners; }

	void GenerateFences();
	void GenerateSlopes();
	void RemoveSingles();
	void BaseCleanup();
	void GenerateBoxes();
	void GenerateBackground();
	void GenerateMoreBackground();
	void GenerateMoreForeground();
	bool AddBackgroundTile(int x, int y);
	bool AddForegroundTile(int x, int y);
	void GenerateAirPlatforms(int Num);
	void Scan();
	int Size();
	
	void CleanTiles();
	
	ivec2 GetPlayerSpawn();
	ivec2 GetOpenArea();
	ivec2 GetLeftPlatform();
	ivec2 GetRightPlatform();
	ivec2 GetBotPlatform();
	ivec2 GetTopPlatform();
	ivec2 GetPlatform();
	ivec2 GetCeiling();
	ivec2 GetLeftCeiling();
	ivec2 GetRightCeiling();
	ivec2 GetWall();
	ivec2 GetTopCorner();
	ivec2 GetSharpCorner();
	ivec4 GetPit();
	
	bool IsNearSlope(int x, int y);
	
	ivec3 GetLongPlatform();
	ivec3 GetLongCeiling();
	
	ivec2 m_EndPos;
};

#endif
