#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include "room.h"
#include "gen_layer.h"


CRoom::CRoom(int x, int y, int w, int h)
{
	m_Open = false;
	
	m_X = x;
	m_Y = y;
	m_W = w;
	m_H = h;
	
	m_pChild1 = NULL;
	m_pChild2 = NULL;
	
	int i = 0;
	
	int RoomSize = 6+rand()%6;
	
	while (!(m_pChild1 || TooSmall() || i++ > 24))
	{
		//RoomSize = 8 + min(20, x/5);
		
		//if (m_H + rand()%5 < m_W + rand()%5)
		//if (m_H*(1.0f + frandom()) < m_W*(1.0f + frandom()))
		if (m_H < m_W)
		{
			if (m_W > RoomSize+3)
				Split(false);
			if (m_H > RoomSize)
				Split(true);
		}
		else
		{
			if (m_H > RoomSize)
				Split(true);
			if (m_W > RoomSize+3)
				Split(false);
		}
	}
	
	/*
	if (m_pChild1 && m_pChild1->TooSmall())
	{
		delete m_pChild1;
		m_pChild1 = NULL;
	}
	
	if (m_pChild2 && m_pChild2->TooSmall())
	{
		delete m_pChild2;
		m_pChild2 = NULL;
	}
	*/
}

CRoom::~CRoom()
{
	if (m_pChild1)
		delete m_pChild1;
	if (m_pChild2)
		delete m_pChild2;
}

void CRoom::Split(bool Vertical)
{
	if (TooSmall())
		return;
		
	if (Vertical)
	{
		int h2 = m_H;
		
		if (m_W < 32)
			m_H = 3 + rand()%(m_H-6);
		else
			m_H = m_H/(2 + rand()%2);
		
		if (!m_pChild1)
		{
			m_pChild1 = new CRoom(m_X, m_Y, m_W, m_H);
			m_pChild2 = new CRoom(m_X, m_Y+m_H, m_W, h2-m_H);
		}
	}
	else
	{
		int w2 = m_W;
		
		if (m_H < 32)
			m_W = 3 + rand()%(m_W-6);
		else
			m_W = m_W/(2 + rand()%2);

		if (!m_pChild1)
		{
			m_pChild1 = new CRoom(m_X, m_Y, m_W, m_H);
			m_pChild2 = new CRoom(m_X+m_W, m_Y, w2-m_W, m_H);
		}
	}
}


bool CRoom::Open(int x, int y)
{
	bool c1 = false;
	bool c2 = false;
	
	if (m_pChild1)
		c1 = m_pChild1->Open(x, y);
	
	if (m_pChild2)
		c2 = m_pChild2->Open(x, y);
	
	if (m_X <= x && m_X+m_W >= x &&
		m_Y <= y && m_Y+m_H >= y)
	{
		m_Open = true;
		return (!TooSmall() || c1 || c2);
	}
	
	return false;
}

void CRoom::Generate(CGenLayer *pTiles)
{
	//if (TooSmall())
	//	return;
	
	if (!m_pChild1 && m_Open)
		Fill(pTiles, 0, m_X, m_Y, m_W, m_H);
	
	if (m_pChild1)
		m_pChild1->Generate(pTiles);
	
	if (m_pChild2)
		m_pChild2->Generate(pTiles);
}


void CRoom::Fill(CGenLayer *pTiles, int Index, int x, int y, int w, int h)
{
	for(int py = y; py < y+h; py++)
		for(int px = x; px < x+w; px++)
		{
			pTiles->Set(Index, px, py);
		}
}