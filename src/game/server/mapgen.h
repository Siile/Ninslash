#ifndef GAME_MAPGEN_H
#define GAME_MAPGEN_H

#include <engine/shared/noise.h>
#include <game/layers.h>
#include <game/collision.h>

enum
{
	CAVE_LEVEL = 40,
	TUNNEL_LEVEL = 25,
};

class CMapGen
{
	class CLayers *m_pLayers;
	CCollision *m_pCollision;
	class CPerlinOctave *m_pNoise;

	void GenerateCaves(int FillBlock);
	void GenerateTunnels(int Num);
	void GenerateSkip();

	void ModifTile(ivec2 Pos, int Layer, int Tile);

public:
	CMapGen();
	~CMapGen();

	void FillMap(int Seed);
	void Init(CLayers *pLayers, CCollision *pCollision);
	class CPerlinOctave *GetNoise() { return m_pNoise; }
};

inline int PercOf(int Prc, int Total) { return Prc*Total*0.01f; }

#endif
