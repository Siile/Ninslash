#ifndef GAME_SERVER_MAPGEN_ROOM_H
#define GAME_SERVER_MAPGEN_ROOM_H

class CRoom
{
private:
	CRoom *m_pChild1, *m_pChild2;
	int m_X, m_Y, m_W, m_H;
	
	bool m_Open;
	
public:
	CRoom(int x, int y, int w, int h);
	~CRoom();
	
	bool TooSmall()
	{
		if (m_W < 8 || m_H < 8)
			return true;
		
		return false;
	}
	
	bool Open(int x, int y);
	
	void Split(bool Vertical);
	
	void Generate(class CGenLayer *pTiles);
	
	void Fill(class CGenLayer *pTiles, int Index, int x, int y, int w, int h);
};


#endif
