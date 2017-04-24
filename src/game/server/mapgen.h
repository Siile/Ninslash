#ifndef GAME_MAPGEN_H
#define GAME_MAPGEN_H

#include <engine/shared/noise.h>
#include <game/layers.h>
#include <game/collision.h>


class CMapGen
{
	class CLayers *m_pLayers;
	CCollision *m_pCollision;

	void GenerateRooms();
	
	void GenerateStart(class CGenLayer *pTiles);
	void GenerateEnd(class CGenLayer *pTiles);
	
	void GenerateBarrel(class CGenLayer *pTiles);
	void GenerateAlien(class CGenLayer *pTiles);
	void GenerateHearts(class CGenLayer *pTiles);
	void GenerateAmmo(class CGenLayer *pTiles);
	void GenerateAcid(class CGenLayer *pTiles);
	
	void GenerateSawblade(class CGenLayer *pTiles);
	void GenerateFiretrap(class CGenLayer *pTiles);
	void GenerateDeathray(class CGenLayer *pTiles);
	
	void GenerateWeapon(class CGenLayer *pTiles, int Weapon);

	void ModifTile(ivec2 Pos, int Layer, int Tile);

public:
	CMapGen();
	~CMapGen();

	void FillMap(int Seed);
	void Init(CLayers *pLayers, CCollision *pCollision);
};

inline int PercOf(int Prc, int Total) { return Prc*Total*0.01f; }

#endif
