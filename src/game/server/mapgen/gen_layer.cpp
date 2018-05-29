#include <base/system.h>
#include <engine/shared/config.h>

#include "gen_layer.h"

CGenLayer::CGenLayer(int w, int h)
{
	m_Width = w;
	m_Height = h;
	m_Size = 0;
	m_EndPos = ivec2(0, 0);
	
	m_pTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pTiles[i] = 1;
	
	m_pBGTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pBGTiles[i] = 0;
	
	m_pDoodadsTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pDoodadsTiles[i] = 0;
	
	m_pObjectTiles = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pObjectTiles[i] = 0;
	
	m_pFlags = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pFlags[i] = 1;
	
	m_pBGFlags = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pBGFlags[i] = 1;
	
	m_pDoodadsFlags = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pDoodadsFlags[i] = 1;
	
	m_pObjectFlags = new int[w*h];
	for (int i = 0; i < w*h; i++)
		m_pObjectFlags[i] = 1;
	
	m_NumPlatforms = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aPlatform[i] = ivec2(0, 0);
	
	m_NumMedPlatforms = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aMedPlatform[i] = ivec2(0, 0);
	
	m_NumOpenAreas = 0;
	for (int i = 0; i < GEN_MAX; i++)
		m_aOpenArea[i] = ivec2(0, 0);
	
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
	
	if (m_pDoodadsTiles)
		delete m_pDoodadsTiles;
	
	if (m_pObjectTiles)
		delete m_pObjectTiles;
	
	if (m_pFlags)
		delete m_pFlags;
	
	if (m_pBGFlags)
		delete m_pBGFlags;
	
	if (m_pDoodadsFlags)
		delete m_pDoodadsFlags;
	
	if (m_pObjectFlags)
		delete m_pObjectFlags;
}


void CGenLayer::CleanTiles()
{
	for (int i = 0; i < m_Width*m_Height; i++)
		m_pTiles[i] = 0;
	
	for (int i = 0; i < m_Width*m_Height; i++)
		m_pFlags[i] = 0;
}



void CGenLayer::GenerateBoxes()
{
	int n = 3 + rand()%14;
		
	for (int k = 0; k < 10000; k++)
	{
		int wx = 10 + rand()%(m_Width - 20);
		int wy = 10 + rand()%(m_Height - 20);
		
		int i = 100;
		
		if (!n)
			break;
		
		if (Used(wx, wy))
			continue;
		
		int b = n;
		
		while (i-- > 0 && n > 0)
		{
			int x = wx + rand()%20 - rand()%20;
			int y = wy + rand()%20 - rand()%20;
			
			int l = 5;
			// to the floor
			while (!Get(x, y+1) && !Get(x, y+1, FGOBJECTS) && l-- > 0)
				y++;
			
			bool Valid = true;
		
			int s = 2;
			int p = 44;
			
			if (b < n+3 || frandom() < 0.5f)
			{
				s = 3;
				p = 25;
			}
			
			// empty area for the box
			for (int xx = x; xx < x+s; xx++)
				for (int yy = y; yy < y-s; yy++)
					if (Used(xx, yy))
						Valid = false;
					
			// floor
			for (int xx = x; xx < x+s; xx++)
				if (!Get(xx, y+1) && !Get(xx, y+1, FGOBJECTS))
					Valid = false;
			
			for (int xx = x; xx < x+s; xx++)
				if (Used(xx, y))
					Valid = false;
			
			// ceiling not too close
			for (int xx = x-1; xx < x+s+1; xx++)
				for (int yy = y-(s+3); yy < y-s; yy++)
					if (Used(xx, yy))
						Valid = false;

			// avoid ramps
			if (!Get(x-1, y) && !Get(x-1, y+1) && Get(x-1, y+2))
				Valid = false;
			if (!Get(x+s, y) && !Get(x+s, y+1) && Get(x+s, y+2))
				Valid = false;
					
			if (Valid)
			{
				ivec2 ts = ivec2(s, s);
				
				y -= ts.y-1;
				
				bool Flip = false;
				
				if (frandom() < 0.5f)
					Flip = !Flip;
				
				//for (int xx = 0; xx < ts.x; xx++)
				//	for (int yy = 0; yy < ts.y; yy++)
				//		Use(x+xx, y+yy);
				
				if (Flip)
					for (int xx = 0; xx < ts.x; xx++)
						for (int yy = 0; yy < ts.y; yy++)
							Set(p+ts.x-1-xx+yy*16, x+xx, y+yy, 1, FGOBJECTS); // TILEFLAG_VFLIP
				else
					for (int xx = 0; xx < ts.x; xx++)
						for (int yy = 0; yy < ts.y; yy++)
							Set(p+xx+yy*16, x+xx, y+yy, 0, FGOBJECTS);
				n--;
			}
		}
	}
}


bool CGenLayer::AddForegroundTile(int x, int y)
{
	if ((Get(x-1, y) || Get(x+1, y)) && (Get(x, y-1) || Get(x, y+1)))
		return true;
	
	return false;
}


bool CGenLayer::AddBackgroundTile(int x, int y)
{
	if ((Get(x-1, y, BACKGROUND) || Get(x+1, y, BACKGROUND)) && (Get(x, y-1, BACKGROUND) || Get(x, y+1, BACKGROUND)))
		return true;
	
	return false;
}
 
void CGenLayer::GenerateMoreForeground()
{
	float a1 = 0.02f + frandom()*0.01f;
	float a2 = 0.02f + frandom()*0.01f;
	
	for (int i = 0; i < 10; i++)
	{
		bool *apTiles = new bool[m_Width*m_Height];
		for (int x = 0; x < m_Width; x++)
			for (int y = 0; y < m_Height; y++)
				if (abs(sin(x*a1)*cos(y*a2)) > 0.5f)
					apTiles[x + y*m_Width] = AddForegroundTile(x, y);
			
		for (int x = 0; x < m_Width; x++)
			for (int y = 0; y < m_Height; y++)
				if (apTiles[x + y*m_Width])
					Set(1, x, y, 0);

		delete apTiles;
	}
}

void CGenLayer::GenerateMoreBackground()
{
	for (int i = 0; i < 10; i++)
	{
		bool *apTiles = new bool[m_Width*m_Height];
		for (int x = 0; x < m_Width; x++)
			for (int y = 0; y < m_Height; y++)
				apTiles[x + y*m_Width] = AddBackgroundTile(x, y);
			
		for (int x = 0; x < m_Width; x++)
			for (int y = 0; y < m_Height; y++)
				if (apTiles[x + y*m_Width])
					Set(1, x, y, 0, BACKGROUND);

		delete apTiles;
	}
}
		
void CGenLayer::GenerateBackground()
{
	// clone foreground
	for (int x = 0; x < m_Width; x++)
		for (int y = 0; y < m_Height; y++)
		{
			if (Get(x, y))// && frandom() < 0.5f)
				Set(1, x, y, 0, BACKGROUND);
			else
				Set(0, x, y, 0, BACKGROUND);
		}
}


void CGenLayer::GenerateFences()
{
	for (int x = 4; x < m_Width-4; x++)
	{
		if (frandom() < 0.75f)
			continue;
		
		for (int y = 4; y < m_Height-4; y++)
		{
			// start pos
			if (!Get(x, y) && Get(x, y+1))
			{
				// boundaries
				int i = 0;
				int x1 = 1;
				while (!Get(x-x1, y) && Get(x-x1, y+1) && i++ < 20)
					x1++;
				
				int x2 = 1;
				while (!Get(x+x2, y) && Get(x+x2, y+1) && i++ < 20)
					x2++;
				
				x1 -= 2;
				x2 -= 2;
				
				// correct size
				if (x1+x2 >= 6)
				{
					bool Valid = true;
					
					// area empty?
					for (int yy = -2; yy < 1; yy++)
						for (int xx = -(x1+1); xx <= x2+1; xx++)
							if (Get(x+xx, y+yy, DOODADS) || Get(x+xx, y+yy, FGOBJECTS))
								Valid = false;

					if (frandom() < 0.75f)
						Valid = false;
					
					// avoid door
					if (x - (x1+1) < m_EndPos.x && m_EndPos.x < x + x2+1 &&
						abs(m_EndPos.y - y) < 2)
						Valid = false;
						
					if (Valid)
					{
						Set(10*16+9-16, x-x1, y-1, 0, DOODADS);
						Set(10*16+9, x-x1, y, 0, DOODADS);

						for (int xx = x-(x1-1); xx < x+x2; xx++)
						{
							int t = 10*16+11;
							
							if (xx == x+(-x1+x2)/2 && frandom() < 0.25f)
								t--;
							
							Set(t-16, xx, y-1, 0, DOODADS);
							Set(t, xx, y, 0, DOODADS);
						}
						
						Set(10*16+9-16, x+x2-1, y-1, 1, DOODADS);
						Set(10*16+9, x+x2-1, y, 1, DOODADS);
					}
				}
			}
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
		
		if (!Used(x, y) && (abs(m_EndPos.x - x) > 10 || x+10 < m_EndPos.y))
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
				if (!Get(x-s, yy+1, DOODADS))
				{
					while (!Get(x-s, yy) && Sanity++ < 500)
					{
						Set(1, x-s, yy, 0, DOODADS);
						yy--;
					}
					
					Set(1, x-s, yy, 0, DOODADS);
				}
				
				Sanity = 0;
				yy = y-1;
				if (!Get(x+s-2, yy+1, DOODADS) && !Get(x+s-1, yy+2, DOODADS))
				{
					while (!Get(x+s-2, yy) && Sanity++ < 500)
					{
						Set(1, x+s-2, yy, 0, DOODADS);
						yy--;
					}
					
					Set(1, x+s-2, yy, 0, DOODADS);
				}
			}
		}
		
	}
}


void CGenLayer::RemoveSingles()
{
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (Get(x, y) && !Get(x-1, y) && !Get(x+1, y))
				Set(0, x, y);
			
			if (Get(x, y) && !Get(x, y-1) && !Get(x, y+1) && 
				((!Get(x-1, y) && Get(x+1, y) && Get(x+1, y-1) && Get(x+1, y+1)) || ((!Get(x+1, y) && Get(x-1, y) && Get(x-1, y-1) && Get(x-1, y+1)))))
				Set(0, x, y);
		}
}


void CGenLayer::BaseCleanup()
{
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (Get(x, y) && !Get(x-1, y) && !Get(x+1, y))
				Set(0, x, y);
			
			if (Get(x, y) && !Get(x, y-1) && !Get(x, y+1))
				Set(0, x, y);
		}
}


bool CGenLayer::IsNearSlope(int x, int y)
{
	if (Get(x, y) && Get(x-1, y) && Get(x-1, y+1) && !Get(x-2, y) && Get(x-2, y+1))
		return true;
	
	if (Get(x, y) && Get(x+1, y) && Get(x+1, y+1) && !Get(x+2, y) && Get(x+2, y+1))
		return true;
	
	return false;
}


void CGenLayer::GenerateSlopes()
{
	// top ramp
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 6; y < m_Height-2; y++)
		{
			bool Valid = true;
			bool Found = false;
			
			if (Get(x, y) && frandom() < 0.5f)
			{
				int s = 0;
				int MaxSize = 70 + rand()%8;

				for (int i = 0; i < MaxSize-1; i++)
				{
					Valid = true;
					s = MaxSize-i;
					for (int xx = x; xx < x + s; xx++)
						if (!Get(xx, y) || Get(xx, y+1))
							Valid = false;
						
					for (int yy = y-s; yy < y; yy++)
						if (!Get(x, yy) || Get(x-1, yy))
							Valid = false;
						
					if (Valid)
						break;
				}
				
				if (Valid)
					for (int xx = x; xx < x + s; xx++)
						for (int yy = y-s; yy < y; yy++)
							if (!Get(xx, yy))
								Valid = false;
							
				if (Valid)
					for (int yy = y-s; yy < y; yy++)
						if (!Get(x + s + 1, yy))
							Valid = false;
						
				if (Valid)
					for (int xx = x; xx < x + s; xx++)
						if (!Get(xx, y-s-1))
							Valid = false;

				if (Valid)
					for (int xx = 0; xx <= s; xx++)
						for (int yy = 0; yy < xx; yy++)
							Set(-1, x+s-xx, y-yy);
						
				if (Valid)
					Found = true;
			}
			
			if (!Found && Get(x, y) && frandom() < 0.75f)
			{
				int s = 0;
				int MaxSize = 7 + rand()%8;

				for (int i = 0; i < MaxSize-1; i++)
				{
					Valid = true;
					s = MaxSize-i;
					for (int xx = x-s; xx < x; xx++)
						if (!Get(xx, y) || Get(xx, y+1))
							Valid = false;
					
					for (int yy = y-s; yy < y; yy++)
						if (!Get(x, yy) || Get(x+1, yy))
							Valid = false;
						
					if (Valid)
						break;
				}
				
				if (Valid)
					for (int xx = x-s; xx < x; xx++)
						for (int yy = y-s; yy < y; yy++)
							if (!Get(xx, yy))
								Valid = false;
							
				if (Valid)
					for (int yy = y-s; yy < y; yy++)
						if (!Get(x - s - 1, yy))
							Valid = false;
						
				if (Valid)
					for (int xx = x-s; xx < x; xx++)
						if (!Get(xx, y-s-1))
							Valid = false;

				if (Valid)
					for (int xx = -s; xx <= 0; xx++)
						for (int yy = 0; yy < xx+s; yy++)
							Set(-1, x+xx, y-yy);
			}
			
		}
	
	
	
	// bottom ramp
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			bool Found = false;
			bool Valid = true;
			if (Get(x, y) && frandom() < 0.75f)
			{
				int s = 0;
				int MaxSize = 7 + rand()%8;

				for (int i = 0; i < MaxSize-1; i++)
				{
					Valid = true;
					s = MaxSize-i;
					for (int xx = x; xx < x + s; xx++)
						if (!Get(xx, y) || Get(xx, y-1))
							Valid = false;
					
					for (int yy = y; yy < y + s; yy++)
						if (!Get(x, yy) || Get(x-1, yy))
							Valid = false;
						
					if (Valid)
						break;
				}
				
				if (Valid)
					for (int xx = x; xx < x + s; xx++)
						for (int yy = y; yy < y + s; yy++)
							if (!Get(xx, yy))
								Valid = false;
							
				if (Valid)
					for (int yy = y; yy < y + s; yy++)
						if (!Get(x + s + 1, yy))
							Valid = false;
						
				if (Valid)
					for (int xx = x; xx < x + s; xx++)
						if (!Get(xx, y+s+1))
							Valid = false;

				if (Valid)
					for (int xx = 0; xx <= s; xx++)
						for (int yy = 0; yy < xx; yy++)
							Set(-1, x+s-xx, y+yy);

				if (Valid)
					Found = true;
			}
			
			if (!Found && Get(x, y) && frandom() < 0.75f)
			{
				int s = 0;
				int MaxSize = 7 + rand()%8;

				for (int i = 0; i < MaxSize-1; i++)
				{
					Valid = true;
					s = MaxSize-i;
					for (int xx = x-s; xx < x; xx++)
						if (!Get(xx, y) || Get(xx, y-1))
							Valid = false;
					
					for (int yy = y; yy < y + s; yy++)
						if (!Get(x, yy) || Get(x+1, yy))
							Valid = false;
						
					if (Valid)
						break;
				}
				
				/*
				for (int xx = x-s; xx < x; xx++)
					if (!Get(xx, y) || Get(xx, y-1))
						Valid = false;
				
				for (int yy = y; yy < y + s; yy++)
					if (!Get(x, yy) || Get(x+1, yy))
						Valid = false;
					*/
				
				if (Valid)
					for (int xx = x-s; xx < x; xx++)
						for (int yy = y; yy < y + s; yy++)
							if (!Get(xx, yy))
								Valid = false;
							
				if (Valid)
					for (int yy = y; yy < y + s; yy++)
						if (!Get(x - s - 1, yy))
							Valid = false;
						
				if (Valid)
					for (int xx = x-s; xx < x; xx++)
						if (!Get(xx, y+s+1))
							Valid = false;

				if (Valid)
					for (int xx = -s; xx <= 0; xx++)
						for (int yy = 0; yy < xx+s; yy++)
							Set(-1, x+xx, y+yy);
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
					for (int ay = py+1; ay < y; ay++)
						if (Used(ax, ay))
						{
							Valid = false;
							break;
						}
				
				// check borders
				/*
				for (int ay = py; ay < y; ay++)
				{
					if (!Get(x-1, ay))
						Valid = false;
					else if (!Get(px+1, ay))
						Valid = false;
				}
				*/
				
				if (abs(x-px) < 2 || abs(y-py) < 1)
					Valid = false;
				
				if (Valid && m_NumPits < GEN_MAX)
				{
					for (int ax = x-1; ax < px+2; ax++)
						for (int ay = py; ay < y+2; ay++)
							Use(ax, ay);
					
					m_aPit[m_NumPits++] = ivec4(x, py+2, px, y+1);
				}
			}
		}
	
	// find player spawn spots
	if (str_comp(g_Config.m_SvGametype, "coop") == 0)
	{
		if (g_Config.m_SvMapGenLevel%10 == 9)
		{
			for (int y = m_Height-2; y > 2; y--)
				for (int x = 2; x < m_Width-2; x++)
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
		}
		else
		{
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
		}
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
	
	
	// find long platforms (conveyor belts)
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Used(x, y-1) && Get(x, y) && Get(x, y+1) && (!Get(x-1, y)))
			{
				int x1 = 1;
				bool Valid = false;
				
				while (x1 < 40)
				{
					//if (Used(x+x1, y-1) || !Get(x+x1, y))
					
					if (Used(x+x1, y-1))
						break;
					
					if (!Get(x+x1, y))
					{
						x1--;
						Valid = true;
						break;
					}
					x1++;
				}
				
				// avoid ramps
				if (!Get(x-1, y-1) && !Get(x-1, y) && Get(x-1, y+1))
				{
					x++;
					x1--;
				}
				
				while (!Get(x+x1+1, y-1) && !Get(x+x1+1, y) && Get(x+x1+1, y+1) && x1 > 0)
					x1--;

				if (x1 > 6 && Valid)
				{
					if (m_NumLongPlatforms < GEN_MAX)
					{
						Set(-1, x, y-1);
						Use(x, y);
						m_aLongPlatform[m_NumLongPlatforms++] = ivec3(x, y, x+x1);
					}
				}
			}
		}
		
		
	// find platforms
	for (int x = 2; x < m_Width-2; x++)
		for (int y = 2; y < m_Height-2; y++)
		{
			if (!Used(x-2, y) && !Used(x-1, y) && !Used(x, y) && !Used(x+1, y) && !Used(x+2, y) &&
				Get(x-2, y+1) && Get(x-1, y+1) && Get(x, y+1) && Get(x+1, y+1) && Get(x+2, y+1) && !IsNearSlope(x, y+1))
			{
				if (m_NumMedPlatforms < GEN_MAX)
				{
					m_aMedPlatform[m_NumMedPlatforms++] = ivec2(x, y);
					Set(-1, x-2, y);
					Set(-1, x-1, y);
					Set(-1, x, y);
					Set(-1, x+1, y);
					Set(-1, x+2, y);
				}
			}
			
			if (!Used(x-1, y) && !Used(x, y) && !Used(x+1, y) &&
				Get(x-1, y+1) && Get(x, y+1) && Get(x+1, y+1) && !IsNearSlope(x, y+1))
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
		
	// find open areas
	for (int x = 7; x < m_Width-7; x++)
		for (int y = 7; y < m_Height-7; y++)
		{
			bool Valid = true;
			for (int xx = -3; xx <= 3 && Valid; xx++)
				for (int yy = -3; yy <= 3 && Valid; yy++)
					if (Used(x+xx, y+yy))
						Valid = false;
		
			if (Valid)
			{
				if (m_NumOpenAreas < GEN_MAX)
				{
					m_aOpenArea[m_NumOpenAreas++] = ivec2(x, y);
					
					for (int xx = -2; xx <= 2; xx++)
						for (int yy = -2; yy <= 2; yy++)
							Set(-1, x+xx, y+yy);
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
	
	if (n >= 9999)
		return ivec3(0, 0, 0);
	
	ivec3 p = m_aLongPlatform[i];
	
	for (int x = p.x; x <= p.z; x++)
	{
		Use(x, p.y);
		Use(x, p.y-1);
	}
	
	m_aLongPlatform[i] = ivec3(0, 0, 0);
	
	return p;
}

ivec2 CGenLayer::GetMedPlatform()
{
	if (m_NumMedPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumMedPlatforms;
	
	while (m_aMedPlatform[i].x == 0 && n++ < 999)
		i = rand()%m_NumMedPlatforms;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
	ivec2 p = m_aMedPlatform[i];
	m_aMedPlatform[i] = ivec2(0, 0);
	
	return p;
}


ivec2 CGenLayer::GetPlatform()
{
	if (m_NumPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumPlatforms;
	
	while (m_aPlatform[i].x == 0 && n++ < 999)
		i = rand()%m_NumPlatforms;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
	ivec2 p = m_aPlatform[i];
	m_aPlatform[i] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetLeftPlatform()
{
	if (m_NumPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	
	for (int i = 0; i < m_NumPlatforms; i++)
	{
		if (m_aPlatform[i].x != 0)
			if (m_aPlatform[n].x == 0 || m_aPlatform[i].x < m_aPlatform[n].x)
				n = i;
	}

	ivec2 p = m_aPlatform[n];
	m_aPlatform[n] = ivec2(0, 0);
	
	return p;
}


ivec2 CGenLayer::GetRightPlatform()
{
	if (m_NumPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	
	for (int i = 0; i < m_NumPlatforms; i++)
	{
		if (m_aPlatform[i].x != 0)
			if (m_aPlatform[n].x == 0 || m_aPlatform[i].x > m_aPlatform[n].x)
				n = i;
	}

	ivec2 p = m_aPlatform[n];
	m_aPlatform[n] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetTopPlatform()
{
	if (m_NumPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	
	for (int i = 0; i < m_NumPlatforms; i++)
	{
		if (m_aPlatform[i].x != 0)
			if (m_aPlatform[n].y == 0 || m_aPlatform[i].y < m_aPlatform[n].y)
				n = i;
	}

	ivec2 p = m_aPlatform[n];
	m_aPlatform[n] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetBotPlatform()
{
	if (m_NumPlatforms <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	
	for (int i = 0; i < m_NumPlatforms; i++)
	{
		if (m_aPlatform[i].x != 0)
			if (m_aPlatform[n].y == 0 || m_aPlatform[i].y > m_aPlatform[n].y)
				n = i;
	}

	ivec2 p = m_aPlatform[n];
	m_aPlatform[n] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetOpenArea()
{
	if (m_NumOpenAreas <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumOpenAreas;
	
	while (m_aOpenArea[i].x == 0 && n++ < 999)
		i = rand()%m_NumOpenAreas;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
	ivec2 p = m_aOpenArea[i];
	m_aOpenArea[i] = ivec2(0, 0);
	
	return p;
}

ivec3 CGenLayer::GetLongCeiling()
{
	if (m_NumLongCeilings <= 0)
		return ivec3(0, 0, 0);
	
	int n = 0;
	int i = rand()%m_NumLongCeilings;
	
	while (m_aLongCeiling[i].x == 0 && n++ < 999)
		i = rand()%m_NumLongCeilings;
	
	if (n >= 9999)
		return ivec3(0, 0, 0);
	
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
	
	while (m_aCeiling[i].x == 0 && n++ < 999)
		i = rand()%m_NumCeilings;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
	ivec2 p = m_aCeiling[i];
	m_aCeiling[i] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetLeftCeiling()
{
	if (m_NumCeilings <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	
	for (int i = 0; i < m_NumCeilings; i++)
	{
		if (m_aCeiling[i].x != 0)
			if (m_aCeiling[n].x == 0 || m_aCeiling[i].x < m_aCeiling[n].x)
				n = i;
	}
	
	ivec2 p = m_aCeiling[n];
	m_aCeiling[n] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetRightCeiling()
{
	if (m_NumCeilings <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	
	for (int i = 0; i < m_NumCeilings; i++)
	{
		if (m_aCeiling[i].x != 0)
			if (m_aCeiling[n].x == 0 || m_aCeiling[i].x > m_aCeiling[n].x)
				n = i;
	}
	
	ivec2 p = m_aCeiling[n];
	m_aCeiling[n] = ivec2(0, 0);
	
	return p;
}

ivec2 CGenLayer::GetWall()
{
	if (m_NumWalls <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumWalls;
	
	while (m_aWall[i].x == 0 && n++ < 999)
		i = rand()%m_NumWalls;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
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
	
	// try random
	while (m_aPit[i].x == 0 && n++ < 99)
		i = rand()%m_NumPits;
	
	if (m_aPit[i].x == 0)
	{
		ivec4 p = m_aPit[i];
		m_aPit[i] = ivec4(0, 0, 0, 0);
		return p;
	}
	
	// try every pit if random failed
	for (int l = 0; l < m_NumPits; l++)
		if (m_aPit[l].x != 0)
		{
			ivec4 p = m_aPit[l];
			m_aPit[l] = ivec4(0, 0, 0, 0);
			return p;
		}
	
	// :(
	return ivec4(0, 0, 0, 0);
}

ivec2 CGenLayer::GetTopCorner()
{
	if (m_NumTopCorners <= 0)
		return ivec2(0, 0);
	
	int n = 0;
	int i = rand()%m_NumTopCorners;
	
	while (m_aTopCorner[i].x == 0 && n++ < 999)
		i = rand()%m_NumTopCorners;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
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
	
	while (m_aTopCorner[i].x == 0 && n++ < 999)
		i = rand()%m_NumCorners;
	
	if (n >= 9999)
		return ivec2(0, 0);
	
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
	else if (Layer == FGOBJECTS)
	{
		m_pObjectTiles[x + y*m_Width] = Tile;
		m_pObjectFlags[x + y*m_Width] = Flags;
	}
	else if (Layer == DOODADS)
	{
		m_pDoodadsTiles[x + y*m_Width] = Tile;
		m_pDoodadsFlags[x + y*m_Width] = Flags;
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
	else if (Layer == FGOBJECTS)
		i = m_pObjectTiles[x + y*m_Width];
	else if (Layer == DOODADS)
		i = m_pDoodadsTiles[x + y*m_Width];
	
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
	else if (Layer == FGOBJECTS)
		return m_pObjectFlags[x + y*m_Width];
	else if (Layer == DOODADS)
		return m_pDoodadsFlags[x + y*m_Width];
	
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
	else if (Layer == FGOBJECTS)
		i = m_pObjectTiles[Index];
	else if (Layer == DOODADS)
		i = m_pDoodadsTiles[Index];
	
	if (i < 0)
		i = 0;
	
	return i;
}

bool CGenLayer::Used(int x, int y)
{
	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return true;
	
	if (m_pTiles[x + y*m_Width] != 0 || m_pObjectTiles[x + y*m_Width] != 0)
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
		
	for (int i = 0; i < m_NumMedPlatforms; i++)
		if (abs(m_aMedPlatform[i].x - x) < 3 && m_aMedPlatform[i].y == y)
			m_aMedPlatform[i] = ivec2(0, 0);
		
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










