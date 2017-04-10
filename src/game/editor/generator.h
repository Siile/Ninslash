#ifndef GAME_EDITOR_GENERATOR_H
#define GAME_EDITOR_GENERATOR_H

class CGenerator
{
private:
	class CRoom *m_pRoom;
	
public:
	CGenerator();

	void Fill(class CLayerTiles *pLayer, int Index, int x, int y, int w, int h);
	
	void Generate(class CLayerTiles *pLayer);
	
	void GenerateWalls(class CLayerTiles *pLayer);
	void ConnectRooms(class CLayerTiles *pLayer);
	
	int Tile(class CLayerTiles *pLayer, int x, int y);
};


#endif
