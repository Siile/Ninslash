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
	bool m_AlwaysOn;
	bool m_DisplayLevel;
	
	// invasion
	int m_MinLevel;
	int m_MaxLevel;
	
	CGameVote()
	{
		m_Valid = false;
		m_AlwaysOn = false;
		m_DisplayLevel = false;
		m_MinPlayers = 0;
		m_MaxPlayers = 0;
		m_MinLevel = 0;
		m_MaxLevel = 0;
	}
};

#endif