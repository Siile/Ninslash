#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/shared/config.h>

#include "room.h"
#include "maze.h"


CMaze::CMaze(int w, int h)
{
	m_W = w;
	m_H = h;
	
	m_aOpen = new bool[w * h];
	m_aConnected = new bool[w * h];
	
	for (int i = 0; i < w*h; i++)
	{
		m_aOpen[i] = false;
		m_aConnected[i] = false;
	}
	
	Generate();
}

CMaze::~CMaze()
{
	if (m_aOpen)
		delete m_aOpen;
}


void CMaze::Generate()
{
	m_Rooms = 0;

	// invasion
	if (str_comp(g_Config.m_SvGametype, "coop") == 0)
	{
		int Level = g_Config.m_SvMapGenLevel;
		
		// rising acid
		if (Level%10 == 9)
		{
			int r = 4+min(14, Level/3);


			float s = 0.15f+frandom()*0.15f;
			float sy = 0.4f;
			
			Connect(vec2(m_W*(0.3f-s), m_H*(0.5f+s*sy)), vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy)));
			Connect(vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy)), vec2(m_W*(0.5f+s), m_H*(0.5f))); // W
			
			Connect(vec2(m_W*(0.35f-s), m_H*(0.5f)), vec2(m_W*(0.4f), m_H*(0.5f)));
			Connect(vec2(m_W*(0.6f), m_H*(0.5f)), vec2(m_W*(0.65f+s), m_H*(0.5f)));
			
			Connect(vec2(m_W*(0.5f-s), m_H*(0.5f-s*sy)), vec2(m_W*(0.5f+s), m_H*(0.5f-s*sy)));
			Connect(vec2(m_W*(0.5f), m_H*(0.5f-s*sy)), vec2(m_W*(0.5f), m_H*(0.5f-s*sy*2)));
			Connect(vec2(m_W*(0.5f), m_H*(0.5f-s*sy*2)), vec2(m_W*(0.6f+s), m_H*(0.5f-s*sy*2)));
			
			Connect(vec2(m_W*(0.5f-s), m_H*(0.5f+s*sy*2)), vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy*2)));
			
			if (Level > 10)
				Connect(vec2(m_W*(0.5f-s), m_H*(0.5f+s*sy*3)), vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy*3)));
			
			float x = 0.5f + (frandom()-frandom())*0.2f;
			
			if (Level > 20)
			{
				Connect(vec2(m_W*(x-0.15f-s), m_H*(0.5f-s*sy*3)), vec2(m_W*(x-0.1f), m_H*(0.5f-s*sy*3)));
				Connect(vec2(m_W*(x+0.1f), m_H*(0.5f-s*sy*3)), vec2(m_W*(x+0.15f+s), m_H*(0.5f-s*sy*3)));
			}
			
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s*(frandom()-frandom())), m_H*(0.5f+s*sy*2));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s*(frandom()-frandom())), m_H*(0.5f-s*sy*2));


			// create random rooms
			for (int i = 0; i < r; i++)
				GenerateRoom();
			
			//ConnectRooms();
			ConnectEverything();
		}
		// first rounds
		else if (Level <= 15)
		{
			GenerateLinear(min(50+Level*5, 90), Level);
		}
		// Z
		else if (Level%10 == 8)
		{
			int r = min(20, Level/3);

			float s = 0.12f+frandom()*0.15f;
			float sy = 0.4f+frandom()*0.15f;
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s), m_H*(0.5f+s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s), m_H*(0.5f));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s), m_H*(0.5f-s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f-s*sy));
	
			for (int i = 0; i < m_Rooms - 1; i++)
				Connect(m_aRoom[i], m_aRoom[i+1]);
	
			// create random rooms
			for (int i = 0; i < r; i++)
				GenerateRoom();
			
			ConnectRooms();
			ConnectEverything();
		}
		else if (Level%10 == 6)
		{
			int r = min(14, Level/3);

			float s = 0.12f+frandom()*0.15f;
			float sy = 0.4f+frandom()*0.15f;
			
			Connect(vec2(m_W*(0.3f-s), m_H*(0.5f+s*sy)), vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy)));
			Connect(vec2(m_W*(0.5f+s), m_H*(0.5f+s*sy)), vec2(m_W*(0.5f+s), m_H*(0.5f))); // W
			Connect(vec2(m_W*(0.5f-s), m_H*(0.5f)), vec2(m_W*(0.5f+s), m_H*(0.5f)));
			Connect(vec2(m_W*(0.5f-s), m_H*(0.5f)), vec2(m_W*(0.5f-s), m_H*(0.5f-s*sy))); // w
			Connect(vec2(m_W*(0.5f-s), m_H*(0.5f-s*sy)), vec2(m_W*(0.5f+s), m_H*(0.5f-s*sy)));
			Connect(vec2(m_W*(0.5f), m_H*(0.5f-s*sy)), vec2(m_W*(0.5f), m_H*(0.5f-s*sy*2)));
			Connect(vec2(m_W*(0.5f), m_H*(0.5f-s*sy*2)), vec2(m_W*(0.8f+s), m_H*(0.5f-s*sy*2)));
	
			// create random rooms
			for (int i = 0; i < r; i++)
				GenerateRoom();

			ConnectRooms();
			ConnectEverything();
		}
		else if (Level%10 == 0)
		{
			int r = min(20, Level/3);

			float s = 0.12f+frandom()*0.15f;
			float sy = 0.4f+frandom()*0.15f;
			//m_aRoom[m_Rooms++] = vec2(m_W*(0.5f), m_H*(0.5f));
			//m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s), m_H*(0.5f));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s), m_H*(0.5f+s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f), m_H*(0.5f+s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f), m_H*(0.5f-s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f-s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f), m_H*(0.5f));
	
			for (int i = 0; i < m_Rooms - 1; i++)
				Connect(m_aRoom[i], m_aRoom[i+1]);
	
			// create random rooms
			for (int i = 0; i < r; i++)
				GenerateRoom();
			
			ConnectRooms();
			ConnectEverything();
		}
		else if (Level%10 == 3)
		{
			int r = min(20, Level/3);

			float s = 0.11f+frandom()*0.15f;
			float sy = 0.4f+frandom()*0.15f;
			
			
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f-s), m_H*(0.5f-s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f-s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s), m_H*(0.5f));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f), m_H*(0.5f));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f), m_H*(0.5f+s*sy));
			m_aRoom[m_Rooms++] = vec2(m_W*(0.5f+s*2), m_H*(0.5f+s*sy));
	
			for (int i = 0; i < m_Rooms - 1; i++)
				Connect(m_aRoom[i], m_aRoom[i+1]);
	
			// create random rooms
			for (int i = 0; i < r; i++)
				GenerateRoom();
			
			ConnectRooms();
			ConnectEverything();
		}
		// random room structure
		else
		{
			int r = min(50, 10+Level/3);
			
			m_aRoom[m_Rooms++] = vec2(m_W*0.5f, m_H*(0.1f+frandom()*0.8f));
	
			// create random rooms
			for (int i = 0; i < r; i++)
				GenerateRoom();
			
			int c = min(1+r/4, 5);
			
			for (int i = 0; i < c; i++)
				ConnectRandomRooms();
			
			ConnectRooms();
			ConnectEverything();
		}
	}
	else if (str_comp(g_Config.m_SvGametype, "dm") == 0)
	// dm
	{
		vec2 Start = vec2(m_W*0.3f + frandom()* 0.4f, m_H*0.3f + frandom()* 0.4f);
		
		int r = 6 + rand()%9;
		int s = 30 + rand()%20;
	
		// box shape
		{
			m_aRoom[m_Rooms++] = Start + vec2(-s*(0.5f+frandom()), -s*(0.5f+frandom())/2);
			m_aRoom[m_Rooms++] = Start + vec2(+s*(0.5f+frandom()), -s*(0.5f+frandom())/2);
			m_aRoom[m_Rooms++] = Start + vec2(+s*(0.5f+frandom()), +s*(0.5f+frandom())/2);
			m_aRoom[m_Rooms++] = Start + vec2(-s*(0.5f+frandom()), +s*(0.5f+frandom())/2);
			
			Connect(m_aRoom[0], m_aRoom[1]);
			Connect(m_aRoom[1], m_aRoom[2]);
			Connect(m_aRoom[2], m_aRoom[3]);
			Connect(m_aRoom[3], m_aRoom[0]);
		}
			
		// create random rooms
		for (int i = 0; i < r; i++)
			GenerateRoom();
		
		int c = 2 + rand()%4;
		for (int i = 0; i < c; i++)
			
		ConnectRooms();
		ConnectEverything();
	}
	else if (str_comp(g_Config.m_SvGametype, "ctf") == 0)
	// ctf
	{
		vec2 Start = vec2(m_W*0.5f, m_H*0.3f + frandom()* 0.4f);
		
		int r = 10 + rand()%10;
		int s = 120 + rand()%20;
		
		r = 7;
	
		{
			vec2 p1 = Start + vec2(-s, 0);
			vec2 p2 = Start + vec2(0, -s*(0.5f+frandom()*0.5f)/3);
			vec2 p3 = Start + vec2(s, 0);
			vec2 p4 = Start + vec2(0, +s*(0.5f+frandom()*0.5f)/3);
			
			Connect(p1, p2);
			Connect(p2, p3);
			Connect(p3, p4);
			Connect(p4, p1);
			
			m_aRoom[m_Rooms++] = Start;
		}
			
		// create random rooms
		for (int i = 0; i < 3; i++)
			GenerateRoom();
		
		m_aRoom[m_Rooms++] = Start + vec2(-s, 0);
		m_aRoom[m_Rooms++] = Start + vec2(s, 0);
		
		for (int i = 0; i < r; i++)
			GenerateRoom();
			
		ConnectRooms();
		ConnectEverything();
	}
	else
	// tdm
	{
		vec2 Start = vec2(m_W*0.5f, m_H*0.3f + frandom()* 0.4f);
		
		int r = 9 + rand()%9;
		int s = 80 + rand()%10;
	
		{
			m_aRoom[m_Rooms++] = Start + vec2(-s, 0);
			m_aRoom[m_Rooms++] = Start + vec2(0, -s*(0.5f+frandom()*0.5f)/3);
			m_aRoom[m_Rooms++] = Start + vec2(s, 0);
			m_aRoom[m_Rooms++] = Start + vec2(0, +s*(0.5f+frandom()*0.5f)/3);
			
			Connect(m_aRoom[0], m_aRoom[1]);
			Connect(m_aRoom[1], m_aRoom[2]);
			Connect(m_aRoom[2], m_aRoom[3]);
			Connect(m_aRoom[3], m_aRoom[0]);
			
			m_aRoom[m_Rooms++] = Start;
		}
			
		// create random rooms
		for (int i = 0; i < r; i++)
			GenerateRoom();
		
		int c = 1 + rand()%3;
		for (int i = 0; i < c; i++)
			ConnectRandomRooms();
			
		ConnectRooms();
		ConnectEverything();
	}
}


void CMaze::GenerateLinear(int Width, int Rooms)
{
	float y = 0.3f + frandom()*0.4f;
	Connect(vec2(m_W*0.5f-Width, m_H*y), vec2(m_W*0.5f+Width, m_H*y));
	
	if (Rooms > 0)
	{
		m_aRoom[m_Rooms++] = vec2(m_W*0.5f-Width*frandom(), m_H*y);
		m_aRoom[m_Rooms++] = vec2(m_W*0.5f+Width*frandom(), m_H*y);
		
		for (int i = 0; i < Rooms; i++)
			GenerateRoom();
	}
	
	ConnectEverything();
}



void CMaze::GenerateRoom()
{
	// find a free spot to the room
	
	bool Valid = false;
	int i = 0;
	
	while (!Valid && i++ < 100000)
	{
		Valid = true;
		vec2 p = vec2(5 + frandom()*(m_W-10), 5 + frandom()*(m_H-10));
		
		if (m_Rooms > 0)
		{
			vec2 rp = vec2(-1, -1);
			
			float d = -1.0f;
			for (int r = 0; r < m_Rooms; r++)
				if (d < 0.0f || distance(vec2(p.x, p.y*2.25f), vec2(m_aRoom[r].x, m_aRoom[r].y*2.25f)) < d)
				{
					d = distance(vec2(p.x, p.y*2.25f), vec2(m_aRoom[r].x, m_aRoom[r].y*2.25f));
					rp = m_aRoom[r];
				}

			if (abs(p.x - rp.x) > 8 && abs(p.y - rp.y) > 8)
				Valid = false;
			
			if (d < 20.0f || d > 40.0f)
				Valid = false;
		}
				
		if (Valid)
		{
			m_aRoom[m_Rooms] = p;
			Open(m_aRoom[m_Rooms], 1 + rand()%4);
			
			m_Rooms++;
			return;
		}
	}
}


void CMaze::ConnectRandomRooms()
{
	if (m_Rooms < 2)
		return;
	
	int r0 = rand()%(m_Rooms-1);
	int r1 = rand()%(m_Rooms-1);
	
	if (r0 != r1)
		Connect(m_aRoom[r0], m_aRoom[r1]);
}

	
void CMaze::ConnectRooms()
{
	if (m_Rooms < 2)
		return;
	
	//for (int r = 1; r < m_Rooms; r++)
	//	Connect(m_aRoom[r-1], m_aRoom[r]);
	
	// connect to closest room
	for (int r = 0; r < m_Rooms; r++)
	{
		vec2 Closest = vec2(-1000000, 0);
		
		for (int r2 = 0; r2 < m_Rooms; r2++)
			if (r2 != r)
				if (distance(m_aRoom[r], m_aRoom[r2]) < distance(m_aRoom[r], Closest))
					Closest = m_aRoom[r2];
		
		if (Closest.x >= 0.0f)
			Connect(m_aRoom[r], Closest);
	}
}


void CMaze::ConnectEverything()
{
	// find unconnected
	bool Looping = true;
	int i = 0;
	
	while (Looping && i++ < 10000)
	{
		ivec2 n = GetUnconnected();
		
		if (n.x <= 0)
			Looping = false;
		else
		{
			ivec2 np = GetClosestConnected(n);
			if (np.x > 0)
			{
				Connect(vec2(np.x, np.y), vec2(n.x, n.y));
				
				//for (int c = 0; c < m_W*m_H; c++)
				//	m_aConnected[c] = false;
			}

			SetConnections(n);
		}
	}
}


ivec2 CMaze::GetUnconnected()
{
	bool Looping = true;
	int i = 0;
	
	// check random spots
	while (Looping && i++ < 10000)
	{
		ivec2 p = ivec2(1+rand()%(m_W-2), 1+rand()%(m_H-2));
		if (m_aOpen[p.x + p.y*m_W] && !m_aConnected[p.x + p.y*m_W])
			return p;
	}
	
	// check everything if random failed
	for (int x = 1; x < m_W-1; x++)
		for (int y = 1; y < m_H-1; y++)
			if (m_aOpen[x + y*m_W] && !m_aConnected[x + y*m_W])
			 return ivec2(x, y);
		 
	return ivec2(-1, -1);
}


ivec2 CMaze::GetClosestConnected(ivec2 Pos)
{
	vec2 p0 = vec2(Pos.x, Pos.y);
	ivec2 Closest = ivec2(-1, -1);
	float d = 90000;

	for (int x = 1; x < m_W-1; x++)
		for (int y = 1; y < m_H-1; y++)
			if (m_aConnected[x + y*m_W] && m_aOpen[x + y*m_W])
				if (distance(p0, vec2(x, y)) < d)
				{
					Closest = ivec2(x, y);
					d = distance(p0, vec2(x, y));
				}
				
	return Closest;
}


void CMaze::SetConnections(ivec2 Pos)
{
	if (Pos.x < 0 || Pos.y < 0 || Pos.x >= m_W || Pos.y >= m_H)
		return;
	
	if (m_aConnected[Pos.x + Pos.y*m_W])
		return;
	
	if (m_aOpen[Pos.x + Pos.y*m_W])
		m_aConnected[Pos.x + Pos.y*m_W] = true;
	else
		return;
	
	SetConnections(Pos + ivec2(1, 0));
	SetConnections(Pos + ivec2(-1, 0));
	SetConnections(Pos + ivec2(0, 1));
	SetConnections(Pos + ivec2(0, -1));
}


void CMaze::Connect(vec2 Pos0, vec2 Pos1)
{
	float Distance = distance(Pos0, Pos1);
	int End(Distance+1);

	for(int i = 0; i < End; i++)
	{
		float a = i/Distance;
		vec2 Pos = mix(Pos0, Pos1, a);
		
		Open(Pos);
		Open(Pos+vec2(-1, -1));
		Open(Pos+vec2(1, -1));
		Open(Pos+vec2(1, 1));
		Open(Pos+vec2(-1, 1));
	}
}


void CMaze::Open(vec2 Pos, int Size)
{
	Open(Pos.x, Pos.y);

	/*
	for (int x = -(Size-1); x < (Size-1); x++)
		for (int y = -(Size-1); y < (Size-1); y++)
			Open(Pos.x+x, Pos.y+y);
		*/
}

void CMaze::Open(int x, int y)
{
	// set pos within boundaries
	x = max(1, x);
	x = min(m_W-1, x);
	y = max(1, y);
	y = min(m_H-1, y);
	
	m_aOpen[x + y*m_W] = true;
}


void CMaze::OpenRooms(CRoom *pRoom)
{
	for (int x = 0; x < m_W; x++)
		for (int y = 0; y < m_H; y++)
			if (m_aOpen[x + y*m_W])
				pRoom->Open(x, y);
}