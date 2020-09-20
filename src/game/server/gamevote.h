#ifndef GAME_SERVER_GAMEVOTE_H
#define GAME_SERVER_GAMEVOTE_H

struct CGameVote
{
	char m_aName[32];
	char m_aDescription[64];
	char m_aConfig[32];
	char m_aImage[32];
	
	bool m_Valid;
	int m_MinPlayers;
	int m_MaxPlayers;
	
	CGameVote()
	{
		m_Valid = false;
		m_MinPlayers = 0;
		m_MaxPlayers = 0;
	}
};

#endif