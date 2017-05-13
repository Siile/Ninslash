#include "gen_layer.h"

CGenLayer::CGenLayer(int w, int h)
{
	m_Width = w;
	m_Height = h;
	m_Size = 0;
	
	m_pTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pTiles[i] = 1;
	
	m_pBGTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pBGTiles[i] = 0;
	
	m_pFlags = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pFlags[i] = 1;
	
	m_pBGFlags = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pBGFlags[i] = 1;
	
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
	
	m_NumLongCeilings = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aLongCeiling[i] = ivec3(0, 0, 0);
	
	m_NumLongPlatforms = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aLongPlatform[i] = ivec3(0, 0, 0);
	
	m_NumPlayerSpawns = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aPlayerSpawn[i] = ivec2(0, 0);
}

CGenLayer::~CGenLayer()
{
	if (m_pTiles)
		delete m_pTiles;
	
	if (m_pBGTiles)
		delete m_pBGTiles;
	
	if (m_pFlags)
		delete m_pFlags;
	
	if (m_pBGFlags)
		delete m_pBGFlags;
}

void CGenLayer::GenerateBackground()
{
	if (m_Width < 30)
		return;

	int n = 4;

	while (n-- > 0)
	{
		int nx = 10 + rand()%(m_Width - 20);
		
		int s = 5 + rand()%6;
		
		bool Valid = true;
		
		// check if the area is empty first
		for (int x = nx - s; x < nx + s; x++)
			for (int y = 0; y < m_Height; y++)
				if (Get(x, y, BACKGROUND))
					Valid = false;
				
		// fill the area
		if (Valid)
		{
			for (int x = nx - s; x < nx + s; x++)
				for (int y = 0; y < m_Height; y++)
					Set(1, x, y, 0, BACKGROUND);
		}
	}
}
	
void CGenLayer::GenerateAirPlatforms(int Num)
{
	int b = 5;
	int x, y;
	
	int i = 0;
	
	// don't stack platforms (temporary solution to crappy auto mapper rules)
	bool XUsed[99999];
	
	for (int p = 0; p < 99999; p++)
		XUsed[p] = false;
	
	while (Num > 0 && i++ < 10000)
	{
		x = b+rand()%(m_Width-b*2);
		y = b+rand()%(m_Height-b*2);
		
		if (!Used(x, y))
		{
			bool Valid = true;
			for (int xx = -7; xx < 7; xx++)
				for (int yy = -5; yy < 5; yy++)
				{
					if (Used(x+xx, y+yy))
						Valid = false;
					
					if (x+xx >= 0 && x+xx < m_Width && XUsed[x+xx])
						Valid = false;
				}
					
			if (Valid)
			{
				Num--;
				int s = 3+rand()%3;
				for (int xx = -s; xx < s-1; xx++)
				{
					Set(-1, x+xx, y-1);
					Set(1, x+xx, y);
					
					if (x+xx >= 0 && x+xx < m_Width)
						XUsed[x+xx] = true;
				}
				
				// chains
				int Sanity = 0;
				int yy = y-1;
				if (!Get(x-s, yy+1, BACKGROUND))
				{
					while (!Get(x-s, yy) && Sanity++ < 500)
					{
						Set(1, x-s, yy, 0, BACKGROUND);
						yy--;
					}
				}
				
				Sanity = 0;
				yy = y-1;
				if (!Get(x+s-2, yy+1, BACKGROUND) && !Get(x+s-1, yy+2, BACKGROUND))
				{
					while (!Get(x+s-2, yy) && Sanity++ < 500)
					{
						Set(1, x+s-2, yy, 0, BACKGROUND);
						yy--;
					}
				}
			}
		}
		
	}
}


void CGenLayer::Scan()
{
	// find long ceilings (hangables)
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Used(x, y+1) && Get(x, y) && (Get(x-1, y+1) || !Get(x-1, y)))
			{
				int x1 = 1;
				bool Valid = false;
				
				while (x1 < 20)
				{
					if (Get(x+x1, y+1) || !Get(x+x1, y))
					{
						x1--;
						Valid = true;
						break;
					}
					x1++;
				}
				
				if (x1 > 3 && Valid)
				{
					if (m_NumLongCeilings < GEN_MAX)
					{
						m_aLongCeiling[m_NumLongCeilings++] = ivec3(x, y, x+x1);
						Set(-1, x, y+1);
						Use(x, y);
					}
				}
			}
		}
	
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
		
	
	// find pits / pools
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
				while (!Used(px, y) && Get(px, y+1) && n++ < 60)
					px++;
				
				// check to top from right
				n = 0;
				while (!Used(px-1, py2) && Get(px+1-1, py2) && n++ < 10)
					py2--;
				
				py = max(py, py2);			
				
				bool Valid = true;
				
				// check the whole area
				for (int ax = x; ax < px; ax++)
					for (int ay = py-3; ay < y; ay++)
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
	
	// find long platforms (conveyor belts)
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Get(x, y-1) && Get(x, y) && Get(x, y+1) && (!Get(x-1, y)))
			{
				int x1 = 1;
				bool Valid = false;
				
				while (x1 < 40)
				{
					//if (Used(x+x1, y-1) || !Get(x+x1, y))
					
					if (Get(x+x1, y-1))
						break;
					
					if (!Get(x+x1, y))
					{
						x1--;
						Valid = true;
						break;
					}
					x1++;
				}
				
				
				if (x1 > 7 && Valid)
				{
					if (m_NumLongPlatforms < GEN_MAX)
					{
						m_aLongPlatform[m_NumLongPlatforms++] = ivec3(x, y, x+x1);
						Set(-1, x, y-1);
						Use(x, y);
					}
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

ivec3 CGenLayer::GetLongPlatform()
{
	if (m_NumLongPlatforms <= 0)
		return ivec3(0, 0, 0);
	
	int n = 0;
	int i = rand()%m_NumLongPlatforms;
	
	while (m_aLongPlatform[i].x == 0 && n++ < 9999)
		i = rand()%m_NumLongPlatforms;
	
	ivec3 p = m_aLongPlatform[i];
	
	for (int x = p.x; x <= p.z; x++)
	{
		Use(x, p.y);
		Use(x, p.y-1);
	}
	
	m_aLongPlatform[i] = ivec3(0, 0, 0);
	
	return p;
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

ivec3 CGenLayer::GetLongCeiling()
{
	if (m_NumLongCeilings <= 0)
		return ivec3(0, 0, 0);
	
	int n = 0;
	int i = rand()%m_NumLongCeilings;
	
	while (m_aLongCeiling[i].x == 0 && n++ < 9999)
		i = rand()%m_NumLongCeilings;
	
	ivec3 p = m_aLongCeiling[i];
	
	for (int x = p.x; x <= p.z; x++)
	{
		Use(x, p.y);
		Use(x, p.y+1);
		Use(x, p.y-1);
	}
	
	m_aLongCeiling[i] = ivec3(0, 0, 0);
	
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


void CGenLayer::Set(int Tile, int x, int y, int Flags, int Layer)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return;
	
	if (Layer == FOREGROUND)
	{
		m_pTiles[x + y*m_Width] = Tile;
		m_pFlags[x + y*m_Width] = Flags;
	}
	else if (Layer == BACKGROUND)
	{
		m_pBGTiles[x + y*m_Width] = Tile;
		m_pBGFlags[x + y*m_Width] = Flags;
	}
}

int CGenLayer::Get(int x, int y, int Layer)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return 1;

	int i = 0;
	
	if (Layer == FOREGROUND)
		i = m_pTiles[x + y*m_Width];
	else if (Layer == BACKGROUND)
		i = m_pBGTiles[x + y*m_Width];
	
	if (i < 0)
		i = 0;
	
	return i;
}

int CGenLayer::GetFlags(int x, int y, int Layer)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return 0;
	
	if (Layer == FOREGROUND)
		return m_pFlags[x + y*m_Width];
	else if (Layer == BACKGROUND)
		return m_pBGFlags[x + y*m_Width];
	
	return 0;
}

int CGenLayer::GetByIndex(int Index, int Layer)
{
	if (Index < 0 || Index >= m_Width*m_Height)
		return 1;
	
	int i = 0;
	
	if (Layer == FOREGROUND)
		i = m_pTiles[Index];
	else if (Layer == BACKGROUND)
		i = m_pBGTiles[Index];
	
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
		
	for (int i = 0; i < m_NumTopCorners; i++)
		if (abs(m_aTopCorner[i].x - x) < 2 && abs(m_aTopCorner[i].y - y) < 2)
			m_aTopCorner[i] = ivec2(0, 0);
		
		/*
	for (int i = 0; i < m_NumLongPlatforms; i++)
		if (m_aLongPlatform[i].x <= x && m_aLongPlatform[i].z >= x && abs(m_aLongPlatform[i].y - y) < 2)
			m_aLongPlatform[i] = ivec3(0, 0, 0);
		
	for (int i = 0; i < m_NumLongCeilings; i++)
		if (m_aLongCeiling[i].x <= x && m_aLongCeiling[i].z >= x && abs(m_aLongCeiling[i].y - y) < 2)
			m_aLongCeiling[i] = ivec3(0, 0, 0);
		*/
		
	if (!Used(x, y))
		Set(-1, x, y);
}










