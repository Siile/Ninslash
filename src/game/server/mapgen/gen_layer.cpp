#include "gen_layer.h"

CGenLayer::CGenLayer(int w, int h)
{
	m_Width = w;
	m_Height = h;
	m_Size = 0;
	
	m_pTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pTiles[i] = 1;
	
	m_NumPlatforms = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aPlatform[i] = ivec2(0, 0);
	
	m_NumPits = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aPit[i] = ivec4(0, 0, 0, 0);
	
	m_NumTopCorners = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aTopCorner[i] = ivec2(0, 0);
	
	m_NumCorners = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aCorner[i] = ivec2(0, 0);
	
	m_NumWalls = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aWall[i] = ivec2(0, 0);
	
	m_NumCeilings = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aCeiling[i] = ivec2(0, 0);
	
	m_NumPlayerSpawns = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aPlayerSpawn[i] = ivec2(0, 0);
}

CGenLayer::~CGenLayer()
{
	if (m_pTiles)
		delete m_pTiles;
}

void CGenLayer::GenerateAirPlatforms(int Num)
{
	int b = 5;
	int x, y;
	
	int i = 0;
	
	while (Num > 0 && i++ < 10000)
	{
		x = b+rand()%(m_Width-b*2);
		y = b+rand()%(m_Height-b*2);
		
		if (!Used(x, y))
		{
			bool Valid = true;
			for (int xx = -7; xx < 7; xx++)
				for (int yy = -5; yy < 5; yy++)
					if (Used(x+xx, y+yy))
						Valid = false;
					
			if (Valid)
			{
				Num--;
				int s = 2+rand()%2;
				for (int xx = -s; xx < s; xx++)
				{
					Set(-1, x+xx, y-1);
					Set(1, x+xx, y);
				}
			}
		}
		
	}
}


void CGenLayer::Scan()
{
	// find player spawn spots
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (m_NumPlayerSpawns < 4)
			{
				if (!Used(x-1, y) && !Used(x, y) && !Used(x+1, y) &&
					Get(x-1, y+1) && Get(x, y+1) && Get(x+1, y+1))
				{
					m_aPlayerSpawn[m_NumPlayerSpawns++] = ivec2(x, y);
					Set(-1, x-1, y);
					Set(-1, x, y);
					Set(-1, x+1, y);
				}
			}
			else
				break;
		}
		
	// safe zonens
	for (int i = 0; i < m_NumPlayerSpawns; i++)
	{
		ivec2 p = m_aPlayerSpawn[i];
		
		for (int x = -20; x <= 20; x++)
			for (int y = -20; y <= 20; y++)
				if (!Used(p.x+x, p.y+y))
					Set(-1, p.x+x, p.y+y);
	}
		
	
	// find pits
	for (int x = 1; x < m_Width-1; x++)
		for (int y = 1; y < m_Height-1; y++)
		{
			// bot left
			if (!Used(x, y) && !Used(x+1, y) && !Used(x, y-1) &&
				Get(x-1, y) && Get(x-1, y+1) && Get(x, y+1))
			{
				int px = x+1;
				int py = y-1;
				int py2 = y-1;
				
				// check to top from left
				int n = 0;
				while (!Used(x, py) && Get(x-1, py) && n++ < 10)
					py--;
				
				// check to right
				n = 0;
				while (!Used(px, y) && Get(px, y+1) && n++ < 40)
					px++;
				
				// check to top from right
				n = 0;
				while (!Used(px-1, py2) && Get(px+1-1, py2) && n++ < 10)
					py2--;
				
				py = max(py, py2);			
				
				bool Valid = true;
				
				// check the whole area
				for (int ax = x; ax < px; ax++)
					for (int ay = py; ay < y; ay++)
						if (Used(ax, ay))
						{
							Valid = false;
							break;
						}
				
				if (abs(x-px) < 3 || abs(y-py) < 2)
					Valid = false;
				
				if (Valid && m_NumPits < GEN_MAX)
				{
					m_aPit[m_NumPits++] = ivec4(x, py+2, px, y+1);
				}
			}
		}
	
	// find platforms
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Used(x-1, y) && !Used(x, y) && !Used(x+1, y) &&
				Get(x-1, y+1) && Get(x, y+1) && Get(x+1, y+1))
			{
				if (m_NumPlatforms < GEN_MAX)
				{
					m_aPlatform[m_NumPlatforms++] = ivec2(x, y);
					Set(-1, x-1, y);
					Set(-1, x, y);
					Set(-1, x+1, y);
				}
			}
		}
		
	// find inner corners
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			// top left
			if (!Used(x, y) && !Used(x+1, y) && !Used(x, y+1) && !Used(x+2, y) && !Used(x, y+2) &&
				Get(x-1, y) && Get(x-1, y+1) && Get(x-1, y+2) && Get(x, y-1) && Get(x+1, y-1) && Get(x+2, y-1))
			{
				if (m_NumTopCorners < GEN_MAX)
				{
					m_aTopCorner[m_NumTopCorners++] = ivec2(x, y);
					Set(-1, x, y);
					Set(-1, x, y+1);
					Set(-1, x+1, y);
				}
			}
			
			// top right
			if (!Used(x, y) && !Used(x-1, y) && !Used(x, y+1) && !Used(x-2, y) && !Used(x, y+2) &&
				Get(x+1, y) && Get(x+1, y+1) && Get(x+1, y+2) && Get(x, y-1) && Get(x-1, y-1) && Get(x-2, y-1))
			{
				if (m_NumTopCorners < GEN_MAX)
				{
					m_aTopCorner[m_NumTopCorners++] = ivec2(x, y);
					Set(-1, x, y);
					Set(-1, x, y+1);
					Set(-1, x+1, y);
				}
			}
		}
		
	// find sharp corners
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			// top left
			if (Get(x, y) && Get(x-1, y) && Get(x, y+1) &&
				!Used(x+1, y) && !Used(x+1, y-1) && !Used(x, y-1))
			{
				if (m_NumCorners < GEN_MAX)
					m_aCorner[m_NumCorners++] = ivec2(x, y);
				continue;
			}
			
			// top right
			if (Get(x, y) && Get(x+1, y) && Get(x, y+1) &&
				!Used(x-1, y) && !Used(x-1, y-1) && !Used(x, y-1))
			{
				if (m_NumCorners < GEN_MAX)
					m_aCorner[m_NumCorners++] = ivec2(x, y);
				continue;
			}
			
			// bot left
			if (Get(x, y) && Get(x-1, y) && Get(x, y-1) &&
				!Used(x+1, y) && !Used(x+1, y+1) && !Used(x, y+1))
			{
				if (m_NumCorners < GEN_MAX)
					m_aCorner[m_NumCorners++] = ivec2(x, y);
				continue;
			}
			
			// bot right
			if (Get(x, y) && Get(x+1, y) && Get(x, y-1) &&
				!Used(x-1, y) && !Used(x-1, y+1) && !Used(x, y+1))
			{
				if (m_NumCorners < GEN_MAX)
					m_aCorner[m_NumCorners++] = ivec2(x, y);
				continue;
			}
		}
		
	// find walls
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Used(x, y) && !Used(x, y-1) && !Used(x, y+1) && 
				((Get(x-1, y) && Get(x-1, y-1) && Get(x-1, y+1)) || (Get(x+1, y) && Get(x+1, y-1) && Get(x+1, y+1))) )
			{
				if (m_NumWalls < GEN_MAX)
				{
					m_aWall[m_NumWalls++] = ivec2(x, y);
					Set(-1, x, y-1);
					Set(-1, x, y);
					Set(-1, x, y+1);
				}
			}
		}
		
	// find ceilings
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Used(x-1, y) && !Used(x, y) && !Used(x+1, y) &&
				Get(x-1, y-1) && Get(x, y-1) && Get(x+1, y-1))
			{
				if (m_NumCeilings < GEN_MAX)
				{
					m_aCeiling[m_NumCeilings++] = ivec2(x, y);
					Set(-1, x-1, y);
					Set(-1, x, y);
					Set(-1, x+1, y);
				}
			}
		}
}

ivec2 CGenLayer::GetPlayerSpawn()
{
	if (m_NumPlayerSpawns <= 0)
		return ivec2(0, 0);
		
	m_NumPlayerSpawns--;

	return m_aPlayerSpawn[m_NumPlayerSpawns];
}
ivec2 CGenLayer::GetPlatform()
{
	if (m_NumPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumPlatforms;
	
	while (m_aPlatform[i].x == 0 && n++ < 9999)
		i = rand()%m_NumPlatforms;
	
	ivec2 p = m_aPlatform[i];
	m_aPlatform[i] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetCeiling()
{
	if (m_NumCeilings <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumCeilings;
	
	while (m_aPlatform[i].x == 0 && n++ < 9999)
		i = rand()%m_NumCeilings;
	
	ivec2 p = m_aCeiling[i];
	m_aCeiling[i] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetWall()
{
	if (m_NumWalls <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumPlatforms;
	
	while (m_aWall[i].x == 0 && n++ < 9999)
		i = rand()%m_NumWalls;
	
	ivec2 p = m_aWall[i];
	m_aWall[i] = ivec2(0, 0);
	
	return p;
}

ivec4 CGenLayer::GetPit()
{
	if (m_NumPits <= 0)
		return ivec4(0, 0, 0, 0);
	
	int n = 0;
	int i = rand()%m_NumPits;
	
	while (m_aPit[i].x == 0 && n++ < 9999)
		i = rand()%m_NumPits;
	
	ivec4 p = m_aPit[i];
	m_aPit[i] = ivec4(0, 0, 0, 0);
	
	return p;
}

ivec2 CGenLayer::GetTopCorner()
{
	if (m_NumTopCorners <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumTopCorners;
	
	while (m_aTopCorner[i].x == 0 && n++ < 9999)
		i = rand()%m_NumTopCorners;
	
	ivec2 p = m_aTopCorner[i];
	m_aTopCorner[i] = ivec2(0, 0);
	
	return p;
}


ivec2 CGenLayer::GetSharpCorner()
{
	if (m_NumCorners <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumCorners;
	
	while (m_aTopCorner[i].x == 0 && n++ < 9999)
		i = rand()%m_NumCorners;
	
	ivec2 p = m_aCorner[i];
	m_aCorner[i] = ivec2(0, 0);
	
	return p;
}


void CGenLayer::Set(int Tile, int x, int y)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return;
	
	m_pTiles[x + y*m_Width] = Tile;
}

int CGenLayer::Get(int x, int y)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return 1;
	
	int i = m_pTiles[x + y*m_Width];
	
	if (i < 0)
		i = 0;
	
	return i;
}

bool CGenLayer::Used(int x, int y)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return true;
	
	if (m_pTiles[x + y*m_Width] != 0)
		return true;
	
	return false;
}

int CGenLayer::Size()
{
	if (m_Size > 0)
		return m_Size;
	
	
	for (int x = 0; x < m_Width; x++)
		for (int y = 0; y < m_Height; y++)
		{
			if (m_pTiles[x + y*m_Width] == 0)
				m_Size++;
		}
	
	return m_Size;
}

void CGenLayer::Use(int x, int y)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return;
	
	for (int i = 0; i < m_NumPits; i++)
		if (x >= m_aPit[i].x && x <= m_aPit[i].z && y >= m_aPit[i].y && y <= m_aPit[i].w)
			m_aPit[i] = ivec4(0, 0, 0, 0);
		
	for (int i = 0; i < m_NumPlatforms; i++)
		if (abs(m_aPlatform[i].x - x) < 2 && m_aPlatform[i].y == y)
			m_aPlatform[i] = ivec2(0, 0);
		
	for (int i = 0; i < m_NumCeilings; i++)
		if (abs(m_aCeiling[i].x - x) < 2 && m_aCeiling[i].y == y)
			m_aCeiling[i] = ivec2(0, 0);
		
	for (int i = 0; i < m_NumWalls; i++)
		if (abs(m_aWall[i].y - y) < 2 && m_aWall[i].x == x)
			m_aWall[i] = ivec2(0, 0);
		
	for (int i = 0; i < m_NumCorners; i++)
		if (abs(m_aCorner[i].x - x) < 2 && abs(m_aCorner[i].y - y) < 2)
			m_aCorner[i] = ivec2(0, 0);
		
	if (!Used(x, y))
		Set(-1, x, y);
}










