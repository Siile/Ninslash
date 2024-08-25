// Implementation for server messages 'Player XXX was here 7 minutes ago'
#include <base/system.h>
#include <engine/shared/config.h>

#include "lastseen.h"
#include "gamecontext.h"


CLastSeen CGameContext::m_LastSeen;

CLastSeen::CLastSeen()
{
	m_aSeen[0] = m_aSeen[1] = 0;
	str_copy(m_aName[0], "", sizeof(m_aName[0]));
	str_copy(m_aName[1], "", sizeof(m_aName[1]));
}

void CLastSeen::SendLastSeenMsg(CGameContext *GameServer, int ClientID)
{
	if (!g_Config.m_SvLastSeenMessages || GameServer->m_pController->CountHumans() > 1 || m_aSeen[0] == 0 || m_aSeen[1] == 0)
		return;

	const char *ClientName = GameServer->Server()->ClientName(ClientID);
	int i = (m_aSeen[1] > m_aSeen[0]);
	if (str_comp(m_aName[i], ClientName) == 0)
		i = !i;

	GameServer->SendChatTarget(ClientID, "%s was here %d min ago", m_aName[i], int((time_get() / time_freq() - m_aSeen[i]) / 60));
}

void CLastSeen::OnClientDrop(CGameContext *GameServer, int ClientID)
{
	if (!g_Config.m_SvLastSeenMessages || !GameServer->IsHuman(ClientID))
		return;

	const char *ClientName = GameServer->Server()->ClientName(ClientID);

	int i = 0;
	if (str_comp(m_aName[0], ClientName) == 0)
		i = 0;
	else if (str_comp(m_aName[1], ClientName) == 0)
		i = 1;
	else
		i = (m_aSeen[0] > m_aSeen[1]);

	m_aSeen[i] = time_get() / time_freq();
	str_copy(m_aName[i], ClientName, sizeof(m_aName[i]));

	//dbg_msg("CLastSeen", "OnClientDrop: i %d seen[0]: %s %lld seen[1]: %s %lld", i, m_aName[0], m_aSeen[0], m_aName[1], m_aSeen[1]);
}
