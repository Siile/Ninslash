#ifndef GAME_SERVER_LASTSEEN_H
#define GAME_SERVER_LASTSEEN_H

// Implementation for server messages 'Player XXX was here 7 minutes ago'

#include <engine/shared/protocol.h>

class CGameContext;

class CLastSeen
{
public:
	CLastSeen();

	void SendLastSeenMsg(CGameContext *GameServer, int ClientID);
	void OnClientDrop(CGameContext *GameServer, int ClientID);

private:

	char m_aName[2][MAX_NAME_LENGTH];
	int64 m_aSeen[2];
};

#endif
