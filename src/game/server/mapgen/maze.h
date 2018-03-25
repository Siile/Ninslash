#ifndef GAME_SERVER_MAPGEN_MAZE_H
#define GAME_SERVER_MAPGEN_MAZE_H

class CMaze
{
private:
	int m_W, m_H;
	
	vec2 m_aRoom[999];
	int m_Rooms;
	
	bool *m_aOpen;
	bool *m_aConnected;
	
	void Generate();
	void GenerateRoom(bool AutoConnect = false, bool MirrorMode = false);
	void ConnectRandomRooms();
	void ConnectRooms();
	void ConnectEverything();
	void SetConnections(ivec2 Pos);
	ivec2 GetClosestConnected(ivec2 Pos);
	vec2 GetClosestRoom(vec2 Pos);
	ivec2 GetUnconnected();
	
	void GenerateLinear(int Width, int Rooms = 0);
	
	void Open(vec2 Pos, int Size = 1);
	void Open(int x, int y);
	
	void Connect(vec2 Pos0, vec2 Pos1);
	
public:
	CMaze(int w, int h);
	~CMaze();
	
	void OpenRooms(class CRoom *pRoom);
};


#endif
