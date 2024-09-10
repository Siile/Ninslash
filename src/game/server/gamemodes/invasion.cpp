#include <engine/shared/config.h>

#include <game/mapitems.h>
#include <game/questinfo.h>

#include <game/server/entities/character.h>
#include <game/server/entities/radar.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "invasion.h"

#include <game/server/playerdata.h>
#include <game/server/ai.h>
#include <game/server/ai/inv/robot1_ai.h>
#include <game/server/ai/inv/robot2_ai.h>
#include <game/server/ai/inv/alien1_ai.h>
#include <game/server/ai/inv/alien2_ai.h>
#include <game/server/ai/inv/bunny1_ai.h>
#include <game/server/ai/inv/bunny2_ai.h>
#include <game/server/ai/inv/pyro1_ai.h>
#include <game/server/ai/inv/pyro2_ai.h>



CGameControllerInvasion::CGameControllerInvasion(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "INV";
	m_GameFlags = GAMEFLAG_COOP;
	m_GameState = STATE_STARTING;
	
	m_BotSpawnTick = 0;
	
	if (g_Config.m_SvMapGenRandSeed)
	{
		g_Config.m_SvMapGenSeed = rand()%32767;
		g_Config.m_SvMapGenRandSeed = 0;
	}
	
	srand(g_Config.m_SvMapGenLevel + g_Config.m_SvMapGenSeed);
	
	for (int i = 0; i < MAX_ENEMIES; i++)
		m_aEnemySpawnPos[i] = vec2(0, 0);
	
	m_RoundOverTick = 0;
	m_RoundWinTick = 0;
	
	m_RoundWin = false;
	m_QuestsCompleted = 0;
	
	m_QuestWaveSize = 0;
	m_QuestWaveEndTick = 0;
	m_QuestWaveEnemiesLeft = 0;
	m_Quest = QUEST_NONE;
	m_NextQuest = QUEST_NONE;
	m_QuestChangeTick = 0;
	m_QuestProgressCounter = 0;
	
	int Level = g_Config.m_SvMapGenLevel;
	m_LevelQuestsLeft = min(int(Level*0.6f+1), 4);
	
	
	m_QuestWaveType = WAVE_NONE;
	
	m_TriggerLevel = 0;
	m_GroupSpawnPos = vec2(0, 0);
	SpawnNewWave(false);
		
	m_AutoRestart = false;
	
	m_NumEnemySpawnPos = 0;
	m_SpawnPosRotation = 0;
	m_TriggerTick = 0;
	
	// force some settings
	g_Config.m_SvRandomWeapons = 0;
	g_Config.m_SvOneHitKill = 0;
	g_Config.m_SvWarmup = 0;
	g_Config.m_SvTimelimit = 0;
	g_Config.m_SvScorelimit = 0;
	g_Config.m_SvSurvivalTime = 0;
	g_Config.m_SvEnableBuilding = 1;
	g_Config.m_SvDisablePVP = 1;
	
	if (g_Config.m_SvEnableBuilding)
		m_GameFlags |= GAMEFLAG_BUILD;
	
	if (g_Config.m_SvSurvivalMode)
		m_GameFlags |= GAMEFLAG_SURVIVAL;
	
	m_GameFlags |= GAMEFLAG_ACID;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		new CRadar(&GameServer()->m_World, RADAR_HUMAN, i);
	
	m_pDoor = new CRadar(&GameServer()->m_World, RADAR_DOOR);
	m_pEnemySpawn = new CRadar(&GameServer()->m_World, RADAR_ENEMY);
}


bool CGameControllerInvasion::OnEntity(int Index, vec2 Pos)
{
	if (IGameController::OnEntity(Index, Pos))
		return true;

	if (Index == ENTITY_ENEMYSPAWN && m_NumEnemySpawnPos < MAX_ENEMIES)
	{
		m_aEnemySpawnPos[m_NumEnemySpawnPos++] = Pos;
		return true;
	}
	
	return false;
}


bool CGameControllerInvasion::GetSpawnPos(int Team, vec2 *pOutPos)
{
	if (!pOutPos || !m_NumEnemySpawnPos)
		return false;

	m_SpawnPosRotation++;
	m_SpawnPosRotation = m_SpawnPosRotation%m_NumEnemySpawnPos;
	
	*pOutPos = m_aEnemySpawnPos[m_SpawnPosRotation];
	return true;
}

vec2 CGameControllerInvasion::GetBotSpawnPos()
{
	if (m_GroupSpawnPos.x < 1.0f)
	{
		vec2 p;
		GetSpawnPos(0, &p);
		return p;
	}
	
	vec2 Pos = m_GroupSpawnPos;
	
	for (int i = 0; i < 99; i++)
	{
		Pos = m_GroupSpawnPos + vec2(frandom()-frandom(), frandom()-frandom()) * 400;
		if (!GameServer()->Collision()->TestBox(Pos, vec2(32.0f, 74.0f)))
			return Pos;
	}
	
	/*
	if (GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 48) && !GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y + 32))
		Pos.y += 32;
	else if (!GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 48) && GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y + 32))
		Pos.y -= 32;
	
	for (int i = 0; i < 9; i++)
	{
		vec2 p = Pos + vec2(0, -32);
		vec2 p2 = Pos;
		
		vec2 r = vec2(frandom()-frandom(), frandom()-frandom())*300;
		
		vec2 To = p + r + vec2(0, -32);
		vec2 To2 = p + r + vec2(0, 0);
		
		if (!GameServer()->Collision()->IntersectLine(p, To, 0x0, &To) && !GameServer()->Collision()->IntersectLine(p2, To2, 0x0, &To2))
			return mix(p2, To2, frandom());
	}
	*/

	return m_GroupSpawnPos;
}

void CGameControllerInvasion::RandomGroupSpawnPos()
{
	m_GroupSpawnPos =  m_aEnemySpawnPos[rand()%m_NumEnemySpawnPos];
	m_pEnemySpawn->Activate(m_GroupSpawnPos, Server()->Tick() + Server()->TickSpeed()*5);
}



bool CGameControllerInvasion::CanSpawn(int Team, vec2 *pOutPos, bool IsBot)
{
	CSpawnEval Eval;

	// spectators can't spawn
	if(Team == TEAM_SPECTATORS)
		return false;

	if (IsBot)
	{
		if (m_EnemiesLeft <= 0)
		{
			return false;
		}
		
		if (m_BotSpawnTick > Server()->Tick())
			return false;
		
		if (m_GroupSpawnPos.x < 1.0f && GetSpawnPos(1, pOutPos))
			return true;
	
		vec2 Pos = GetBotSpawnPos();
		*pOutPos = Pos;
		
		m_BotSpawnTick = Server()->Tick() + Server()->TickSpeed() * max(0.1f, 0.5f-g_Config.m_SvMapGenLevel*0.01f);
		
		return true;
	}
	else
		EvaluateSpawnType(&Eval, 0);

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}


void CGameControllerInvasion::OnCharacterSpawn(CCharacter *pChr, bool RequestAI)
{
	IGameController::OnCharacterSpawn(pChr);
	
	// init AI
	if (RequestAI)
	{
		bool Found = false;
		
		if (m_EnemiesLeft > 0)
		{
			m_EnemiesLeft--;
			Found = true;
			
			int Level = 0;
			
			for (int i = 0; i < 9; i++)
				if (m_EnemiesLeft < 1-i*3 + g_Config.m_SvMapGenLevel/2)
					Level++;
			
			if (frandom() < 0.7f && Level > 2)
				Level = rand()%(Level-1);
			
						
			GameServer()->GetAISkin(&pChr->GetPlayer()->m_AISkin, false, 1+rand()%(max(1, 1+g_Config.m_SvMapGenLevel/4-m_QuestWaveType*3)), m_QuestWaveType);
			pChr->GetPlayer()->SetAISkin();
			//pChr->GetPlayer()->m_pAI = new CAIbase(GameServer(), pChr->GetPlayer());
			pChr->m_IsBot = true;
		
			
			// todo: rewrite
			switch (m_QuestWaveType)
			{
				case WAVE_ALIENS: 
					pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case WAVE_FURRIES: 
					pChr->GetPlayer()->m_pAI = new CAIbunny1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case WAVE_CYBORGS: 
				case WAVE_ROBOTS: 
					pChr->GetPlayer()->m_pAI = new CAIrobot1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				case WAVE_SKELETONS: 
					pChr->GetPlayer()->m_pAI = new CAIpyro1(GameServer(), pChr->GetPlayer(), Level);
					break;
					
				default: 
					pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer(), Level);
					break;
			};
			
			m_EnemyCount++;
			pChr->m_SkipPickups = 999;
			Trigger(false);
		}
		
		if (!Found)
		{
			pChr->GetPlayer()->m_pAI = new CAIalien1(GameServer(), pChr->GetPlayer(), g_Config.m_SvMapGenLevel);
			pChr->GetPlayer()->m_ToBeKicked = true;
			Trigger(false);
		}
	}
}

void CGameControllerInvasion::Trigger(bool IncreaseLevel)
{
	if (IncreaseLevel)
		m_TriggerLevel++;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if (!pPlayer)
			continue;
		
		if (pPlayer->m_pAI)
			pPlayer->m_pAI->Trigger(m_TriggerLevel);
	}
}

void CGameControllerInvasion::SpawnNewWave(bool AddBots)
{
	int Level = g_Config.m_SvMapGenLevel;
	
	//m_QuestWaveType = WAVE_ALIENS;
	m_QuestWaveType = rand()%(min(NUM_WAVES-1, Level/5+1))+1;
	
	if (m_Quest == QUEST_SURVIVEWAVETIME)
	{
		m_QuestWaveEndTick = Server()->Tick() + Server()->TickSpeed() * 45;
		m_QuestWaveEnemiesLeft = 9999;
		m_QuestWaveSize = min(6 + Level + GameServer()->m_pController->CountPlayers(0), 32);
		m_EnemiesLeft = m_QuestWaveEnemiesLeft;
	}
	else if (m_Quest == QUEST_SURVIVEWAVE)
	{
		m_QuestWaveEndTick = 0;
		m_QuestWaveEnemiesLeft = min(int(8+Level*2), 60)*(1.0f + (GameServer()->m_pController->CountPlayers(0)-1)*0.2f);
		m_QuestWaveSize = min(6 + Level + GameServer()->m_pController->CountPlayers(0), 32);
		m_EnemiesLeft = m_QuestWaveEnemiesLeft;
	}
	// initial enemies on map load
	else
	{
		m_QuestWaveEndTick = 0;
		m_QuestWaveEnemiesLeft = 0;
		m_QuestWaveSize = 32;
		m_EnemiesLeft = min(20, 7 + Level);
	}
	
	m_EnemyCount = 0;
	
	m_GroupType = GROUP_ALIENS;
	
	if (AddBots)
	{
		RandomGroupSpawnPos();
		
		for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < m_QuestWaveSize; i++)
			GameServer()->AddBot();
	}
	
	m_Deaths = m_QuestWaveSize;
}


void CGameControllerInvasion::DisplayExit(vec2 Pos)
{
	m_pDoor->Activate(Pos);	
}


int CGameControllerInvasion::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	if (pVictim->m_IsBot && !pVictim->GetPlayer()->m_ToBeKicked)
	{
		if (m_EnemiesLeft <= 0)
			pVictim->GetPlayer()->m_ToBeKicked = true;
		
		if (pKiller)
		{
			Trigger(true);
			
			if (frandom() < 0.013f)
				GameServer()->m_pController->DropWeapon(pVictim->m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_UPGRADE)));
			else if (frandom() < 0.013f)
				GameServer()->m_pController->DropWeapon(pVictim->m_Pos, vec2(frandom()*6.0-frandom()*6.0, 0-frandom()*14.0), GameServer()->NewWeapon(GetStaticWeapon(SW_RESPAWNER)));
		}
	}
	
	if (g_Config.m_SvSurvivalMode && !pVictim->m_IsBot && CountPlayersAlive(-1, true) <= 1)
	{
		DeathMessage();
		m_RoundOverTick = Server()->Tick();
	}

	return 0;
}



void CGameControllerInvasion::NextLevel(int CID)
{
	//
	if (!m_RoundWin)
	{
		m_RoundWin = true;
		m_RoundWinTick = Server()->Tick() + Server()->TickSpeed()*CountHumans()*1;
		
		if (CountHumans() > 1)
			GameServer()->SendBroadcastFormat(-1, false, "%s reached the door", Server()->ClientName(CID));
	}
	
	
	CPlayer *pPlayer = GameServer()->m_apPlayers[CID];
	if(pPlayer && pPlayer->GetCharacter() && !pPlayer->GetCharacter()->IgnoreCollision())
		pPlayer->GetCharacter()->Warp();
}


	
void CGameControllerInvasion::ChangeQuest(int NextQuest, float QueueTimeInSeconds)
{
	if (m_NextQuest == NextQuest)
		return;
	
	m_NextQuest = NextQuest;
	m_QuestChangeTick = Server()->Tick() + Server()->TickSpeed() * QueueTimeInSeconds;
}


void CGameControllerInvasion::SendQuestStartMessage(int Quest)
{
	
	GameServer()->SendBroadcast(GetQuestStartMessage(Quest, m_QuestWaveType), -1);
	
}


void CGameControllerInvasion::SendQuestCompletedMessage(int Quest)
{
	GameServer()->SendBroadcast(GetQuestCompletedMessage(Quest, m_QuestWaveType), -1);
}


void CGameControllerInvasion::CompleteCurrentQuest()
{
	SendQuestCompletedMessage(m_Quest);
	m_Quest = QUEST_NONE;
	m_NextQuest = QUEST_NONE;
	m_QuestsCompleted++;
}


void CGameControllerInvasion::Tick()
{
	IGameController::Tick();
	
	if (m_GameState == STATE_FAIL)
		return;
	
	if (m_GameState == STATE_GAME)
	{
		// change quest on time
		if (m_QuestChangeTick && m_QuestChangeTick <= Server()->Tick())
		{
			m_Quest = m_NextQuest;
			m_NextQuest = QUEST_NONE;
			m_QuestChangeTick = 0;
			m_QuestProgressCounter = 0;
			
			if (m_Quest == QUEST_REACHDOOR)
				TriggerEscape();
			
			if (m_Quest == QUEST_SURVIVEWAVE || m_Quest == QUEST_SURVIVEWAVETIME)
				SpawnNewWave();
			
			SendQuestStartMessage(m_Quest);
		}
		
		
		if (m_Quest == QUEST_NONE && m_NextQuest == QUEST_NONE)
		{
			if (m_LevelQuestsLeft <= 0)
				ChangeQuest(QUEST_REACHDOOR, 6.0f);
			else if (m_QuestsCompleted == 0)
				ChangeQuest(QUEST_KILLREMAININGENEMIES, 6.0f);
			else if (g_Config.m_SvMapGenLevel > 5 && frandom() < 0.2f)
				ChangeQuest(QUEST_SURVIVEWAVETIME, 6.0f);
			else
				ChangeQuest(QUEST_SURVIVEWAVE, 6.0f);
			
			m_LevelQuestsLeft--;
		}
		
		if (m_Quest == QUEST_SURVIVEWAVE || m_Quest == QUEST_SURVIVEWAVETIME)
		{
			if (m_Quest == QUEST_SURVIVEWAVETIME)
				m_QuestProgressCounter = int((m_QuestWaveEndTick - Server()->Tick()) / Server()->TickSpeed());
			else
				m_QuestProgressCounter = m_EnemiesLeft + GameServer()->m_pController->CountBotsAlive();
			
			// wave quest completed
			if ((m_QuestWaveEndTick && m_QuestWaveEndTick <= Server()->Tick()) || (m_EnemiesLeft <= 0 && GameServer()->m_pController->CountBotsAlive() <= 0))
			{
				m_EnemiesLeft = 0;
				m_QuestWaveEndTick = 0;
				int CompletedQuest = m_Quest;
				CompleteCurrentQuest();
				
				if (CompletedQuest == QUEST_SURVIVEWAVETIME && GameServer()->m_pController->CountBotsAlive() > 8)
					ChangeQuest(QUEST_KILLREMAININGENEMIES, 5.0f);
			}
		}
		
		if (m_Quest == QUEST_KILLREMAININGENEMIES)
		{
			m_QuestProgressCounter = GameServer()->m_pController->CountBotsAlive();
			
			// quest completed
			if (m_QuestProgressCounter <= 0)
				CompleteCurrentQuest();
		}
	}
			
	// 
	if (m_GameState == STATE_STARTING)
	{
		if (GameServer()->m_pController->CountPlayers(0) > 0)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "start round, enemies: '%u'", m_Deaths);
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "inv", aBuf);
			
			m_TriggerTick = 0;
			m_AutoRestart = true;
			
			m_GameState = STATE_GAME;
			for (int i = 0; i < m_EnemiesLeft && GameServer()->m_pController->CountBots() < 32; i++)
				GameServer()->AddBot();
		}
		// reset to first map if there's no players for 60 seconds
		else if ((m_AutoRestart || g_Config.m_SvMapGenLevel > 1) && Server()->Tick() > Server()->TickSpeed()*60.0f)
		{
			m_AutoRestart = false;
			
			if (g_Config.m_SvMapGenRandSeed)
				g_Config.m_SvMapGenSeed = rand()%32767;
			
			FirstMap();
		}
	}
	else
	{
		if (g_Config.m_SvMapGenLevel > 1)
			m_AutoRestart = true;
			
		// lose => restart
		if (m_RoundOverTick && m_RoundOverTick < Server()->Tick() - Server()->TickSpeed()*2.0f)
		{
			if (++g_Config.m_SvInvFails >= 3)
			{
				g_Config.m_SvInvFails = 0;
				
				if (--g_Config.m_SvMapGenLevel < 1)
					g_Config.m_SvMapGenLevel = 1;
				
				g_Config.m_SvMapGenSeed = rand()%32767;
			
			
				/*
				g_Config.m_SvInvFails = 0;
				
				//if (--g_Config.m_SvMapGenLevel < 1)
				//	g_Config.m_SvMapGenLevel = 1;
				
				int l = g_Config.m_SvMapGenLevel;
				FirstMap();
				g_Config.m_SvMapGenLevel = l;
				*/
				g_Config.m_SvInvFails = 0;
				m_GameState = STATE_FAIL;
				EndRound();
			}
			else
				GameServer()->ReloadMap();
		}
	}
	
	GameServer()->UpdateAI();
	
	if (m_TriggerTick < Server()->Tick())
	{
		Trigger(true);
		m_TriggerTick = Server()->Tick() + Server()->TickSpeed()*4;
	}
	
	// kick unwanted bots
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
			
		if (pPlayer->m_IsBot && pPlayer->m_ToBeKicked)
			GameServer()->KickBot(pPlayer->GetCID());
	}
	
	if (m_RoundWin)
	{
		if (m_RoundWinTick < Server()->Tick())
		{
			m_RoundWin = false;
			m_RoundWinTick = 0;
			g_Config.m_SvMapGenLevel++;
			g_Config.m_SvInvFails = 0;
			
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				CPlayer *pPlayer = GameServer()->m_apPlayers[i];
				if(pPlayer)
					pPlayer->SaveData();
			}
			
			EndRound();
		}
	}
}


void CGameControllerInvasion::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	// send quest data as team score
	pGameDataObj->m_TeamscoreRed = m_Quest;
	pGameDataObj->m_TeamscoreBlue = m_QuestProgressCounter;
}