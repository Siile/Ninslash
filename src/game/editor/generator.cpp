#include <stdio.h>	// sscanf

#include <engine/console.h>
#include <engine/storage.h>
#include <engine/shared/linereader.h>

#include "room.h"
#include "generator.h"
#include "editor.h"

CGenerator::CGenerator()
{
	m_pRoom = NULL;
}

void CGenerator::Fill(CLayerTiles *pLayer, int Index, int x, int y, int w, int h)
{
	if (x < 0 || y < 0 || x+w > pLayer->m_Width || y+h > pLayer->m_Height)
		return;
	
	for(int py = y; py < y+h; py++)
		for(int px = x; px < x+w; px++)
		{
			CTile *pTile = &(pLayer->m_pTiles[py*pLayer->m_Width+px]);
			pTile->m_Index = Index;
		}
}


int CGenerator::Tile(CLayerTiles *pLayer, int x, int y)
{
	if (x < 0 || y < 0 || x > pLayer->m_Width || y > pLayer->m_Height)
		return 0;
	
	
	return pLayer->m_pTiles[y*pLayer->m_Width+x].m_Index;
}
	

void CGenerator::Generate(CLayerTiles *pLayer)
{
	if (pLayer->m_Width > 32 && pLayer->m_Height > 32)
		GenerateWalls(pLayer);
}


void CGenerator::GenerateWalls(CLayerTiles *pLayer)
{
	// generate layer
	if (pLayer->m_Readonly)
		return;
	
	Fill(pLayer, 1, 0, 0, pLayer->m_Width, pLayer->m_Height);
	
	if (m_pRoom)
		delete m_pRoom;
	
	m_pRoom = new CRoom(3, 3, pLayer->m_Width-6, pLayer->m_Height-6);
	
	// unlock rooms from start to end pos
	int y = pLayer->m_Height / 2;
	
	float a = frandom()*pi*2;
	float c = frandom()*0.1f;
	float s = 2.5f + frandom()*2.0f;
	
	for (int x = 8; x < pLayer->m_Width-8; x++)
	{
		m_pRoom->Open(x-1, y-1+sin(a+x*c)*(pLayer->m_Height/s));
		m_pRoom->Open(x, y+sin(a+x*c)*(pLayer->m_Height/s));
		m_pRoom->Open(x+1, y+1+sin(a+x*c)*(pLayer->m_Height/s));
	}
	
	for (int x = 8; x < pLayer->m_Width-8; x++)
	{
		m_pRoom->Open(x-1, y-1-sin(a+x*c)*(pLayer->m_Height/s));
		m_pRoom->Open(x, y-sin(a+x*c)*(pLayer->m_Height/s));
		m_pRoom->Open(x+1, y+1-sin(a+x*c)*(pLayer->m_Height/s));
	}
	
	m_pRoom->Generate(pLayer);
	ConnectRooms(pLayer);
}


void CGenerator::ConnectRooms(CLayerTiles *pLayer)
{
	// do a bit cleaning first
	for(int y = 3; y < pLayer->m_Height-4; y++)
		for(int x = 3; x < pLayer->m_Width-4; x++)
		{
			if (!Tile(pLayer, x-1, y) && Tile(pLayer, x, y) && Tile(pLayer, x+1, y) && !Tile(pLayer, x+2, y))
				Fill(pLayer, 0, x, y, 2, 1);
			
			if (!Tile(pLayer, x, y-1) && Tile(pLayer, x, y) && Tile(pLayer, x, y+1) && !Tile(pLayer, x, y+2))
				Fill(pLayer, 0, x, y, 1, 2);
		}
}

















