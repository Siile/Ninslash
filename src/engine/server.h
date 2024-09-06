

#ifndef ENGINE_SERVER_H
#define ENGINE_SERVER_H
#include "kernel.h"
#include "message.h"

#include <base/tl/array.h>
#include <string>

class IServer : public IInterface
{
	MACRO_INTERFACE("server", 0)
protected:
	int m_CurrentGameTick;
	int m_TickSpeed;

public:
	/*
		Structure: CClientInfo
	*/
	struct CClientInfo
	{
		const char *m_pName;
		int m_Latency;
	};

	int Tick() const { return m_CurrentGameTick; }
	int TickSpeed() const { return m_TickSpeed; }

	virtual int MaxClients() const = 0;
	virtual const char *ClientName(int ClientID) = 0;
	virtual const char *ClientClan(int ClientID) = 0;
	virtual int ClientCountry(int ClientID) = 0;
	virtual bool ClientIngame(int ClientID) = 0;
	virtual int GetClientInfo(int ClientID, CClientInfo *pInfo) = 0;
	virtual void GetClientAddr(int ClientID, char *pAddrStr, int Size) = 0;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags, int ClientID) = 0;

	template<class T>
	int SendPackMsg(T *pMsg, int Flags, int ClientID)
	{
		CMsgPacker Packer(pMsg->MsgID());
		if(pMsg->Pack(&Packer))
			return -1;
		return SendMsg(&Packer, Flags, ClientID);
	}

	virtual void SetBotDefault(int ClientID) = 0;
	virtual void SetClientName(int ClientID, char const *pName) = 0;
	virtual void SetClientClan(int ClientID, char const *pClan) = 0;
	virtual void SetClientCountry(int ClientID, int Country) = 0;
	virtual void SetClientScore(int ClientID, int Score) = 0;

	virtual int SnapNewID() = 0;
	virtual void SnapFreeID(int ID) = 0;
	virtual void *SnapNewItem(int Type, int ID, int Size) = 0;

	virtual void SnapSetStaticsize(int ItemType, int Size) = 0;

	enum
	{
		RCON_CID_SERV=-1,
		RCON_CID_VOTE=-2,
	};
	virtual void SetRconCID(int ClientID) = 0;
	virtual bool IsAuthed(int ClientID) = 0;
	virtual void Kick(int ClientID, const char *pReason) = 0;

	virtual void DemoRecorder_HandleAutoStart() = 0;
	virtual bool DemoRecorder_IsRecording() = 0;
	
	virtual void AddZombie() = 0;
	virtual void GetAISkin(class CAISkin *pAISkin, bool PVP, int Level, int WaveGroup) = 0;

	virtual void ResetGameVoting() = 0;
	virtual bool GetGameVote(class CGameVote *pGameVote, int Players) = 0;
	
	virtual class CPlayerData *GetPlayerData(int ClientID, int ColorID) = 0;
	virtual int GetHighScore() = 0;
	virtual int GetPlayerCount() = 0;
	
	virtual char *GetMapName() = 0;
	bool m_MapGenerated; // MapGen
	char m_aMapInUse[64];

	array<std::string> m_aMapLists;
};

class IGameServer : public IInterface
{
	MACRO_INTERFACE("gameserver", 0)
protected:
public:
	virtual void OnInit() = 0;
	virtual void OnConsoleInit() = 0;
	virtual void OnShutdown() = 0;

	virtual void OnTick() = 0;
	virtual void OnPreSnap() = 0;
	virtual void OnSnap(int ClientID) = 0;
	virtual void OnPostSnap() = 0;

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID) = 0;


	virtual void AddZombie() = 0;
	virtual void GetAISkin(class CAISkin *pAISkin, bool PVP, int Level, int WaveGroup) = 0;
	virtual bool AIInputUpdateNeeded(int ClientID) = 0;
	virtual void AIUpdateInput(int ClientID, int *Data) = 0; // MAX_INPUT_SIZE
	
	virtual void OnClientConnected(int ClientID, bool AI = false) = 0;
	virtual void OnClientEnter(int ClientID) = 0;
	virtual void OnClientDrop(int ClientID, const char *pReason) = 0;
	virtual void OnClientDirectInput(int ClientID, void *pInput) = 0;
	virtual void OnClientPredictedInput(int ClientID, void *pInput) = 0;

	virtual bool IsClientReady(int ClientID) = 0;
	virtual bool IsClientPlayer(int ClientID) = 0;

	virtual const char *GameType() = 0;
	virtual const char *Version() = 0;
	virtual const char *NetVersion() = 0;
};

extern IGameServer *CreateGameServer();
#endif
