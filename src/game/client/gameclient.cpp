#include <string.h>

#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/friends.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/demo.h>
#include <engine/map.h>
#include <engine/storage.h>
#include <engine/sound.h>
#include <engine/gamepad.h>
#include <engine/serverbrowser.h>
#include <engine/shared/demo.h>
#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/generated/game_data.h>

#include <game/localization.h>
#include <game/version.h>
#include "render.h"

#include "gameclient.h"

#include "customstuff.h"
#include "skelebank.h"

#include "components/block.h"
#include "components/binds.h"
#include "components/broadcast.h"
#include "components/gamevote.h"
#include "components/camera.h"
#include "components/chat.h"
#include "components/console.h"
#include "components/controls.h"
#include "components/countryflags.h"
#include "components/damageind.h"
#include "components/debughud.h"
#include "components/effects.h"
#include "components/picker.h"
#include "components/inventory.h"
#include "components/flow.h"
#include "components/hud.h"
#include "components/items.h"
#include "components/weapons.h"
#include "components/fluid.h"
#include "components/cbelt.h"
#include "components/buildings.h"
#include "components/buildings2.h"
#include "components/droids.h"
#include "components/killmessages.h"
#include "components/mapimages.h"
#include "components/maplayers.h"
#include "components/menus.h"
#include "components/motd.h"
#include "components/particles.h"
#include "components/light.h"
#include "components/blood.h"
#include "components/guts.h"
#include "components/brains.h"
#include "components/tracer.h"
#include "components/splatter.h"
#include "components/spark.h"
#include "components/radar.h"
#include "components/players.h"
#include "components/ball.h"
#include "components/nameplates.h"
#include "components/scoreboard.h"
#include "components/skins.h"
#include "components/sounds.h"
#include "components/spectator.h"
#include "components/voting.h"

CGameClient g_GameClient;

inline vec2 RandomDir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }

// instanciate all systems
static CKillMessages gs_KillMessages;
static CCamera gs_Camera;
static CChat gs_Chat;
static CMotd gs_Motd;
static CBroadcast gs_Broadcast;
static CGameVoteDisplay gs_GameVoteDisplay;
static CGameConsole gs_GameConsole;
static CBinds gs_Binds;
static CBlocks gs_Blocks;
static CParticles gs_Particles;
static CBlood gs_Blood;
static CGuts gs_Guts;
static CBrains gs_Brains;
static CTracer gs_Tracers;
static CSplatter gs_Splatter;
static CSpark gs_Spark;
static CFluid gs_Fluid;
static CCBelt gs_CBelt;
static CLight gs_Light;
static CMenus gs_Menus;
static CSkins gs_Skins;
static CCountryFlags gs_CountryFlags;
static CFlow gs_Flow;
static CHud gs_Hud;
static CDebugHud gs_DebugHud;
static CControls gs_Controls;
static CEffects gs_Effects;
static CScoreboard gs_Scoreboard;
static CSounds gs_Sounds;
static CPicker gs_Picker;
static CInventory gs_Inventory;
static CDamageInd gsDamageInd;
static CVoting gs_Voting;
static CSpectator gs_Spectator;

static CDroids gs_Droids;
static CPlayers gs_Players;
static CBalls gs_Balls;
static CNamePlates gs_NamePlates;
static CItems gs_Items;
static CWeapons gs_Weapons;
static CBuildings gs_Buildings;
static CBuildings2 gs_Buildings2;
static CRadar gs_Radar;
static CMapImages gs_MapImages;

static CMapLayers gs_MapLayersBackGround(CMapLayers::TYPE_BACKGROUND);
static CMapLayers gs_MapLayersForeGround(CMapLayers::TYPE_FOREGROUND);

CGameClient::CStack::CStack() { m_Num = 0; }
void CGameClient::CStack::Add(class CComponent *pComponent) { m_paComponents[m_Num++] = pComponent; }

const char *CGameClient::Version() { return GAME_VERSION; }
const char *CGameClient::NetVersion() { return GAME_NETVERSION; }
const char *CGameClient::GetItemName(int Type) { return m_NetObjHandler.GetObjName(Type); }

void CGameClient::OnConsoleInit()
{
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pClient = Kernel()->RequestInterface<IClient>();
	m_pTextRender = Kernel()->RequestInterface<ITextRender>();
	m_pSound = Kernel()->RequestInterface<ISound>();
	m_pGamepad = Kernel()->RequestInterface<IGamepad>();
	m_pInput = Kernel()->RequestInterface<IInput>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	m_pDemoPlayer = Kernel()->RequestInterface<IDemoPlayer>();
	m_pDemoRecorder = Kernel()->RequestInterface<IDemoRecorder>();
	m_pServerBrowser = Kernel()->RequestInterface<IServerBrowser>();
	m_pEditor = Kernel()->RequestInterface<IEditor>();
	m_pFriends = Kernel()->RequestInterface<IFriends>();
	
	// setup pointers
	m_pBinds = &::gs_Binds;
	m_pGameConsole = &::gs_GameConsole;
	m_pParticles = &::gs_Particles;
	m_pBlood = &::gs_Blood;
	m_pGuts = &::gs_Guts;
	m_pBrains = &::gs_Brains;
	m_pTracers = &::gs_Tracers;
	m_pSplatter = &::gs_Splatter;
	m_pSpark = &::gs_Spark;
	m_pFluid = &::gs_Fluid;
	m_pCBelt = &::gs_CBelt;
	m_pLight = &::gs_Light;
	m_pMenus = &::gs_Menus;
	m_pInventory = &::gs_Inventory;
	m_pSkins = &::gs_Skins;
	m_pCountryFlags = &::gs_CountryFlags;
	m_pChat = &::gs_Chat;
	m_pFlow = &::gs_Flow;
	m_pCamera = &::gs_Camera;
	m_pControls = &::gs_Controls;
	m_pEffects = &::gs_Effects;
	m_pSounds = &::gs_Sounds;
	m_pMotd = &::gs_Motd;
	m_pGameVoteDisplay = &::gs_GameVoteDisplay;
	m_pDamageind = &::gsDamageInd;
	m_pMapimages = &::gs_MapImages;
	m_pVoting = &::gs_Voting;
	m_pScoreboard = &::gs_Scoreboard;
	m_pItems = &::gs_Items;
	m_pWeapons = &::gs_Weapons;
	m_pBuildings = &::gs_Buildings;
	m_pBuildings2 = &::gs_Buildings2;
	m_pBlocks = &::gs_Blocks;
	m_pDroids = &::gs_Droids;
	m_pRadar = &::gs_Radar;
	m_pMapLayersBackGround = &::gs_MapLayersBackGround;
	m_pMapLayersForeGround = &::gs_MapLayersForeGround;

	// make a list of all the systems, make sure to add them in the corrent render order
	m_All.Add(m_pSkins);
	m_All.Add(m_pCountryFlags);
	m_All.Add(m_pMapimages);
	m_All.Add(m_pEffects); // doesn't render anything, just updates effects
	m_All.Add(m_pTracers);
	m_All.Add(m_pParticles);
	m_All.Add(m_pBlood);
	m_All.Add(m_pGuts);
	m_All.Add(m_pBrains);
	m_All.Add(m_pSplatter);
	m_All.Add(m_pSpark);
	m_All.Add(m_pLight);
	m_All.Add(m_pBinds);
	m_All.Add(m_pControls);
	m_All.Add(m_pCamera);
	m_All.Add(m_pSounds);
	m_All.Add(m_pVoting);
	m_All.Add(m_pParticles); // doesn't render anything, just updates all the particles

	m_All.Add(&gs_MapLayersBackGround); // first to render
	m_All.Add(&m_pParticles->m_RenderSmoke1);
	m_All.Add(&m_pParticles->m_RenderMine1);
	m_All.Add(&m_pParticles->m_RenderTriangles);
	m_All.Add(&m_pParticles->m_RenderTrail);
	m_All.Add(&m_pParticles->m_RenderColorTrail);
	m_All.Add(&m_pParticles->m_RenderLazerload); // works
	m_All.Add(m_pBuildings);
	m_All.Add(&m_pTracers->m_RenderTracers);
	m_All.Add(&m_pTracers->m_RenderSpriteTracers);
	m_All.Add(&m_pGuts->m_RenderGuts);
	m_All.Add(&m_pBrains->m_RenderBrains);
	m_All.Add(&m_pParticles->m_RenderCrafting);
	m_All.Add(&gs_Droids);
	m_All.Add(m_pItems);
	m_All.Add(&m_pParticles->m_RenderPlayerSpawn);
	m_All.Add(&m_pParticles->m_RenderMonsterSpawn);
	m_All.Add(&m_pParticles->m_RenderTakeoff);
	m_All.Add(&gs_Players);
	m_All.Add(&gs_Balls);
	m_All.Add(m_pBuildings2);
	m_All.Add(m_pWeapons);
	m_All.Add(&m_pParticles->m_RenderMeat);
	m_All.Add(&m_pParticles->m_RenderLazer);
	m_All.Add(&m_pBlood->m_RenderBlood);
	m_All.Add(&m_pBlood->m_RenderAcid);
	m_All.Add(&m_pSplatter->m_RenderSplatter);
	m_All.Add(m_pBlocks);
	m_All.Add(&gs_MapLayersForeGround);
	m_All.Add(m_pCBelt);
	m_All.Add(&m_pParticles->m_RenderEffect1);
	m_All.Add(&m_pParticles->m_RenderSparks);
	m_All.Add(&m_pParticles->m_RenderDeath);
	m_All.Add(&m_pParticles->m_RenderHitEffects);
	m_All.Add(&m_pParticles->m_RenderExplosions);
	m_All.Add(&m_pParticles->m_RenderGreenExplosion);
	m_All.Add(&m_pParticles->m_RenderElectromine);
	m_All.Add(&m_pParticles->m_RenderElectric);
	m_All.Add(&m_pParticles->m_RenderMine2);
	m_All.Add(&m_pParticles->m_RenderFlames);
	m_All.Add(&m_pParticles->m_RenderFlame1);
	m_All.Add(&m_pParticles->m_RenderSwordHits);
	m_All.Add(&m_pParticles->m_RenderClawHits);
	m_All.Add(&m_pParticles->m_RenderBloodFX);
	m_All.Add(&m_pSpark->m_RenderSpark);
	m_All.Add(&m_pSpark->m_RenderArea1);
	m_All.Add(m_pFluid);
	m_All.Add(&m_pBlood->m_RenderAcidLayer);
	m_All.Add(&gs_NamePlates);
	m_All.Add(&m_pParticles->m_RenderGeneral);
	m_All.Add(&m_pParticles->m_RenderDamageInd);
	m_All.Add(m_pDamageind);
	m_All.Add(&m_pLight->m_RenderLight);
	m_All.Add(m_pRadar);
	m_All.Add(m_pInventory);
	m_All.Add(&gs_Hud);
	m_All.Add(&gs_Spectator);
	m_All.Add(&gs_Picker);
	m_All.Add(&gs_KillMessages);
	m_All.Add(m_pChat);
	m_All.Add(&gs_Broadcast);
	m_All.Add(&gs_DebugHud);
	m_All.Add(&gs_Scoreboard);
	m_All.Add(m_pMotd);
	m_All.Add(&gs_GameVoteDisplay);
	m_All.Add(m_pMenus);
	m_All.Add(m_pGameConsole);

	// build the input stack
	m_Input.Add(&m_pMenus->m_Binder); // this will take over all input when we want to bind a key
	m_Input.Add(&m_pBinds->m_SpecialBinds);
	m_Input.Add(m_pGameConsole);
	m_Input.Add(m_pChat); // chat has higher prio due to tha you can quit it by pressing esc
	m_Input.Add(m_pMotd); // for pressing esc to remove it
	m_Input.Add(m_pMenus);
	m_Input.Add(m_pGameVoteDisplay);
	m_Input.Add(&gs_Spectator);
	m_Input.Add(&gs_Picker);
	m_Input.Add(&gs_Inventory);
	m_Input.Add(m_pControls);
	m_Input.Add(m_pBinds);

	// add the some console commands
	Console()->Register("team", "i", CFGFLAG_CLIENT, ConTeam, this, "Switch team");
	Console()->Register("kill", "", CFGFLAG_CLIENT, ConKill, this, "Kill yourself");

	// register server dummy commands for tab completion
	Console()->Register("tune", "si", CFGFLAG_SERVER, 0, 0, "Tune variable to value");
	Console()->Register("tune_reset", "", CFGFLAG_SERVER, 0, 0, "Reset tuning");
	Console()->Register("tune_dump", "", CFGFLAG_SERVER, 0, 0, "Dump tuning");
	Console()->Register("change_map", "?r", CFGFLAG_SERVER, 0, 0, "Change map");
	Console()->Register("restart", "?i", CFGFLAG_SERVER, 0, 0, "Restart in x seconds");
	Console()->Register("broadcast", "r", CFGFLAG_SERVER, 0, 0, "Broadcast message");
	Console()->Register("say", "r", CFGFLAG_SERVER, 0, 0, "Say in chat");
	Console()->Register("set_team", "ii?i", CFGFLAG_SERVER, 0, 0, "Set team of player to team");
	Console()->Register("set_team_all", "i", CFGFLAG_SERVER, 0, 0, "Set team of all players to team");
	Console()->Register("add_vote", "sr", CFGFLAG_SERVER, 0, 0, "Add a voting option");
	Console()->Register("remove_vote", "s", CFGFLAG_SERVER, 0, 0, "remove a voting option");
	Console()->Register("force_vote", "ss?r", CFGFLAG_SERVER, 0, 0, "Force a voting option");
	Console()->Register("clear_votes", "", CFGFLAG_SERVER, 0, 0, "Clears the voting options");
	Console()->Register("vote", "r", CFGFLAG_SERVER, 0, 0, "Force a vote to yes/no");
	Console()->Register("swap_teams", "", CFGFLAG_SERVER, 0, 0, "Swap the current teams");
	Console()->Register("shuffle_teams", "", CFGFLAG_SERVER, 0, 0, "Shuffle the current teams");


	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->m_pClient = this;

	m_pCustomStuff = new CCustomStuff(this);
	
	// let all the other components register their console commands
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnConsoleInit();


	//
	Console()->Chain("player_name", ConchainSpecialInfoupdate, this);
	Console()->Chain("player_clan", ConchainSpecialInfoupdate, this);
	Console()->Chain("player_country", ConchainSpecialInfoupdate, this);
	Console()->Chain("player_color_body", ConchainSpecialInfoupdate, this);
	Console()->Chain("player_color_feet", ConchainSpecialInfoupdate, this);


	m_pSkelebank = new CSkelebank(RenderTools());
	
	//
	m_SuppressEvents = false;
}

void CGameClient::AddFluidForce(vec2 Pos, vec2 Vel)
{
	m_pFluid->AddForce(Pos, Vel);
}



void CGameClient::OnInit()
{
	m_pGraphics = Kernel()->RequestInterface<IGraphics>();

	// propagate pointers
	m_UI.SetGraphics(Graphics(), TextRender());
	m_RenderTools.m_pGraphics = Graphics();
	m_RenderTools.m_pUI = UI();
	
	int64 Start = time_get();

	// set the language
	g_Localization.Load(g_Config.m_ClLanguagefile, Storage(), Console());

	// TODO: this should be different
	// setup item sizes
	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Client()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	// load default font
	static CFont *pDefaultFont = 0;
	char aFilename[512];
	IOHANDLE File = Storage()->OpenFile("fonts/DejaVuSans.ttf", IOFLAG_READ, IStorage::TYPE_ALL, aFilename, sizeof(aFilename));
	if(File)
	{
		io_close(File);
		pDefaultFont = TextRender()->LoadFont(aFilename);
		TextRender()->SetDefaultFont(pDefaultFont);
	}
	if(!pDefaultFont)
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load font. filename='fonts/DejaVuSans.ttf'");

	const char* pLanguageFont = "Source Han Sans";
	if(str_find(g_Config.m_ClLanguagefile, "chinese"))
		pLanguageFont = "Source Han Sans SC";

	File = Storage()->OpenFile("fonts/SourceHanSans.ttc", IOFLAG_READ, IStorage::TYPE_ALL, aFilename, sizeof(aFilename));
	if(File)
	{
		io_close(File);
		if(!TextRender()->LoadCallbackFont(aFilename))
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load callback font. filename='fonts/SourceHanSans.ttc'");
		TextRender()->SetCallbackFont(pLanguageFont);
	}

	// init all components
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "init all components");	
	for(int i = m_All.m_Num-1; i >= 0; --i)
		m_All.m_paComponents[i]->OnInit();

	// setup load amount// load textures
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "load textures");
	for(int i = 0; i < g_pData->m_NumImages; i++)
	{
		g_pData->m_aImages[i].m_Id = Graphics()->LoadTexture(g_pData->m_aImages[i].m_pFilename, IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
		g_GameClient.m_pMenus->RenderLoading();
	}

	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "reset components");
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnReset();

	int64 End = time_get();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "initialisation finished after %.2fms", ((End-Start)*1000)/(float)time_freq());
	Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "gameclient", aBuf);

	m_ServerMode = SERVERMODE_PURE;
	
	Skelebank()->Init(Storage());
	RenderTools()->m_pSkelebank = Skelebank();
	
	Graphics()->LoadShaders();
	
	if (g_Config.m_GfxMultiBuffering)
	{
		Graphics()->CreateTextureBuffer(Graphics()->ScreenWidth(),Graphics()->ScreenHeight());
		Graphics()->ClearBufferTexture();
	}
	
	Client()->LoadReady();
}

void CGameClient::DispatchInput()
{
	// handle mouse movement
	float x = 0.0f, y = 0.0f;
	if(Input()->MouseMoved() || Input()->GamepadMoved())
	{
		Input()->GetMousePosition(&x, &y);
		for(int h = 0; h < m_Input.m_Num; h++)
		{
			if(m_Input.m_paComponents[h]->OnMouseMove(x, y))
				break;
		}
	}

	// handle key presses
	for(int i = 0; i < Input()->NumEvents(); i++)
	{
		IInput::CEvent e = Input()->GetEvent(i);

		for(int h = 0; h < m_Input.m_Num; h++)
		{
			if(m_Input.m_paComponents[h]->OnInput(e))
			{
				//dbg_msg("", "%d char=%d key=%d flags=%d", h, e.ch, e.key, e.flags);
				break;
			}
		}
	}

	// clear all events for this frame
	Input()->ClearEvents();
}


int CGameClient::OnSnapInput(int *pData)
{
	return m_pControls->SnapInput(pData);
}

void CGameClient::OnConnected()
{
	m_Layers.Init(Kernel());
	m_Collision.Init(Layers());

	RenderTools()->RenderTilemapGenerateSkip(Layers());

	for(int i = 0; i < m_All.m_Num; i++)
	{
		m_All.m_paComponents[i]->OnMapLoad();
		m_All.m_paComponents[i]->OnReset();
	}

	CServerInfo CurrentServerInfo;
	Client()->GetServerInfo(&CurrentServerInfo);

	m_pFluid->Generate();
	m_pCBelt->Generate();
	
	m_ServerMode = SERVERMODE_PURE;
	m_LastSendInfo = 0;

	// send the inital info
	SendInfo(true);
}

void CGameClient::OnReset()
{
	// clear out the invalid pointers
	m_LastNewPredictedTick = -1;
	mem_zero(&g_GameClient.m_Snap, sizeof(g_GameClient.m_Snap));

	for(int i = 0; i < MAX_CLIENTS; i++)
		m_aClients[i].Reset();

	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnReset();

	m_DemoSpecID = SPEC_FREEVIEW;
	m_FlagDropTick[TEAM_RED] = 0;
	m_FlagDropTick[TEAM_BLUE] = 0;
	m_Tuning = CTuningParams();
	
	CustomStuff()->Reset();
}


void CGameClient::UpdatePositions()
{
	// local character position
	if(g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(!m_Snap.m_pLocalCharacter || (m_Snap.m_pGameInfoObj && m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
		{
			// don't use predicted
		}
		else
			m_LocalCharacterPos = mix(m_PredictedPrevChar.m_Pos, m_PredictedChar.m_Pos, Client()->PredIntraGameTick());
	}
	else if(m_Snap.m_pLocalCharacter && m_Snap.m_pLocalPrevCharacter)
	{
		m_LocalCharacterPos = mix(
			vec2(m_Snap.m_pLocalPrevCharacter->m_X, m_Snap.m_pLocalPrevCharacter->m_Y),
			vec2(m_Snap.m_pLocalCharacter->m_X, m_Snap.m_pLocalCharacter->m_Y), Client()->IntraGameTick());
	}

	// spectator position
	if(m_Snap.m_SpecInfo.m_Active)
	{
		if(Client()->State() == IClient::STATE_DEMOPLAYBACK && DemoPlayer()->GetDemoType() == IDemoPlayer::DEMOTYPE_SERVER &&
			m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW)
		{
			m_Snap.m_SpecInfo.m_Position = mix(
				vec2(m_Snap.m_aCharacters[m_Snap.m_SpecInfo.m_SpectatorID].m_Prev.m_X, m_Snap.m_aCharacters[m_Snap.m_SpecInfo.m_SpectatorID].m_Prev.m_Y),
				vec2(m_Snap.m_aCharacters[m_Snap.m_SpecInfo.m_SpectatorID].m_Cur.m_X, m_Snap.m_aCharacters[m_Snap.m_SpecInfo.m_SpectatorID].m_Cur.m_Y),
				Client()->IntraGameTick());
			m_Snap.m_SpecInfo.m_UsePosition = true;
		}
		else if(m_Snap.m_pSpectatorInfo && (Client()->State() == IClient::STATE_DEMOPLAYBACK || m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW))
		{
			if(m_Snap.m_pPrevSpectatorInfo)
				m_Snap.m_SpecInfo.m_Position = mix(vec2(m_Snap.m_pPrevSpectatorInfo->m_X, m_Snap.m_pPrevSpectatorInfo->m_Y),
													vec2(m_Snap.m_pSpectatorInfo->m_X, m_Snap.m_pSpectatorInfo->m_Y), Client()->IntraGameTick());
			else
				m_Snap.m_SpecInfo.m_Position = vec2(m_Snap.m_pSpectatorInfo->m_X, m_Snap.m_pSpectatorInfo->m_Y);
			m_Snap.m_SpecInfo.m_UsePosition = true;
		}
	}
	
	bool Paused = false;
	if(m_Snap.m_pGameInfoObj && m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
		Paused = true;
	
	CustomStuff()->Update(Paused);

	if (Collision())
		Collision()->m_GlobalAcid = SurvivalAcid();
	
	// global acid level timer
	if (m_Snap.m_pGameInfoObj)
	{
		if(m_Snap.m_pGameInfoObj->m_TimeLimit && !m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			if (m_Snap.m_pGameInfoObj->m_TimeLimit != 0)
				Collision()->m_Time = m_Snap.m_pGameInfoObj->m_TimeLimit*60*Client()->GameTickSpeed() - ((Client()->GameTick()-m_Snap.m_pGameInfoObj->m_RoundStartTick));
			else
				Collision()->m_GlobalAcid = false;
		}
		else
			Collision()->m_GlobalAcid = false;
	}
	else
		Collision()->m_GlobalAcid = false;

}


static void Evolve(CNetObj_Character *pCharacter, int Tick)
{
	CWorldCore TempWorld;
	CCharacterCore TempCore;
	mem_zero(&TempCore, sizeof(TempCore));
	TempCore.Init(&TempWorld, g_GameClient.Collision());
	TempCore.Read(pCharacter);

	while(pCharacter->m_Tick < Tick)
	{
		pCharacter->m_Tick++;
		TempCore.Tick(false);
		TempCore.Move();
		TempCore.Quantize();
	}

	TempCore.Write(pCharacter);
}

static void EvolveBall(CNetObj_Ball *pBall, int Tick)
{
	CWorldCore TempWorld;
	CBallCore TempCore;
	mem_zero(&TempCore, sizeof(TempCore));
	TempCore.Init(&TempWorld, g_GameClient.Collision());
	TempCore.Read(pBall);

	while(pBall->m_Tick < Tick)
	{
		pBall->m_Tick++;
		TempCore.Tick();
		TempCore.Move();
		TempCore.Quantize();
	}

	TempCore.Write(pBall);
}


void CGameClient::OnRender()
{
	/*Graphics()->Clear(1,0,0);

	menus->render_background();
	return;*/
	/*
	Graphics()->Clear(1,0,0);
	Graphics()->MapScreen(0,0,100,100);

	Graphics()->QuadsBegin();
		Graphics()->SetColor(1,1,1,1);
		Graphics()->QuadsDraw(50, 50, 30, 30);
	Graphics()->QuadsEnd();

	return;*/

	// update the local character and spectate position
	UpdatePositions();

	// dispatch all input to systems
	DispatchInput();

	Graphics()->ClearBufferTexture();
	//Graphics()->ShaderBegin(SHADER_TEST);
	
	// render all systems
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnRender();

	//Graphics()->ShaderEnd();
	
	// render fullscreen layer
	//RenderTools()->RenderFullScreenLayer();
	
	// clear new tick flags
	m_NewTick = false;
	m_NewPredictedTick = false;

	// check if client info has to be resent
	if(m_LastSendInfo && Client()->State() == IClient::STATE_ONLINE && m_Snap.m_LocalClientID >= 0 && !m_pMenus->IsActive() && m_LastSendInfo+time_freq()*3 < time_get())
	{
		// resend if client info differs
		if(str_comp(g_Config.m_PlayerName, m_aClients[m_Snap.m_LocalClientID].m_aName) ||
			str_comp(g_Config.m_PlayerClan, m_aClients[m_Snap.m_LocalClientID].m_aClan) ||
			g_Config.m_PlayerCountry != m_aClients[m_Snap.m_LocalClientID].m_Country ||
			str_comp(g_Config.m_PlayerTopper, m_aClients[m_Snap.m_LocalClientID].m_aTopperName) ||
			str_comp(g_Config.m_PlayerEye, m_aClients[m_Snap.m_LocalClientID].m_aEyeName) ||
			str_comp(g_Config.m_PlayerHead, m_aClients[m_Snap.m_LocalClientID].m_aHeadName) ||
			str_comp(g_Config.m_PlayerBody, m_aClients[m_Snap.m_LocalClientID].m_aBodyName) ||
			str_comp(g_Config.m_PlayerHand, m_aClients[m_Snap.m_LocalClientID].m_aHandName) ||
			str_comp(g_Config.m_PlayerFoot, m_aClients[m_Snap.m_LocalClientID].m_aFootName) ||
			(m_Snap.m_pGameInfoObj && !(m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS) &&	// no teamgame?
			(//g_Config.m_PlayerBody != m_aClients[m_Snap.m_LocalClientID].m_Body ||
			g_Config.m_PlayerBloodColor != m_aClients[m_Snap.m_LocalClientID].m_BloodColor ||
			g_Config.m_PlayerColorBody != m_aClients[m_Snap.m_LocalClientID].m_ColorBody ||
			g_Config.m_PlayerColorFeet != m_aClients[m_Snap.m_LocalClientID].m_ColorFeet ||
			g_Config.m_PlayerColorSkin != m_aClients[m_Snap.m_LocalClientID].m_ColorSkin ||
			g_Config.m_PlayerColorTopper != m_aClients[m_Snap.m_LocalClientID].m_ColorTopper)))
		{
			SendInfo(false);
		}
		m_LastSendInfo = 0;
	}
}

void CGameClient::OnRelease()
{
	// release all systems
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnRelease();
}

void CGameClient::OnMessage(int MsgId, CUnpacker *pUnpacker)
{
	// special messages
	if(MsgId == NETMSGTYPE_SV_EXTRAPROJECTILE)
	{
		int Num = pUnpacker->GetInt();

		for(int k = 0; k < Num; k++)
		{
			CNetObj_Projectile Proj;
			for(unsigned i = 0; i < sizeof(CNetObj_Projectile)/sizeof(int); i++)
				((int *)&Proj)[i] = pUnpacker->GetInt();

			if(pUnpacker->Error())
				return;

			g_GameClient.m_pItems->AddExtraProjectile(&Proj);
		}

		return;
	}
	else if(MsgId == NETMSGTYPE_SV_TUNEPARAMS)
	{
		// unpack the new tuning
		CTuningParams NewTuning;
		int *pParams = (int *)&NewTuning;
		for(unsigned i = 0; i < sizeof(CTuningParams)/sizeof(int); i++)
			pParams[i] = pUnpacker->GetInt();

		// check for unpacking errors
		if(pUnpacker->Error())
			return;

		m_ServerMode = SERVERMODE_PURE;

		// apply new tuning
		m_Tuning = NewTuning;
		return;
	}

	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgId, pUnpacker);
	if(!pRawMsg)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgId), MsgId, m_NetObjHandler.FailedMsgOn());
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "client", aBuf);
		return;
	}

	// TODO: this should be done smarter
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnMessage(MsgId, pRawMsg);

	if(MsgId == NETMSGTYPE_SV_READYTOENTER)
	{
		Client()->EnterGame();
	}
	else if (MsgId == NETMSGTYPE_SV_EMOTICON)
	{
		CNetMsg_Sv_Emoticon *pMsg = (CNetMsg_Sv_Emoticon *)pRawMsg;

		// apply
		m_aClients[pMsg->m_ClientID].m_Emoticon = pMsg->m_Emoticon;
		m_aClients[pMsg->m_ClientID].m_EmoticonStart = Client()->GameTick();
	}
	else if (MsgId == NETMSGTYPE_SV_INVENTORY)
	{
		CNetMsg_Sv_Inventory *pMsg = (CNetMsg_Sv_Inventory *)pRawMsg;

		// apply
		CustomStuff()->m_aItem[0] = pMsg->m_Item1;
		CustomStuff()->m_aItem[1] = pMsg->m_Item2;
		CustomStuff()->m_aItem[2] = pMsg->m_Item3;
		CustomStuff()->m_aItem[3] = pMsg->m_Item4;
		CustomStuff()->m_aItem[4] = pMsg->m_Item5;
		CustomStuff()->m_aItem[5] = pMsg->m_Item6;
		CustomStuff()->m_aItem[6] = pMsg->m_Item7;
		CustomStuff()->m_aItem[7] = pMsg->m_Item8;
		CustomStuff()->m_aItem[8] = pMsg->m_Item9;
		CustomStuff()->m_aItem[9] = pMsg->m_Item10;
		CustomStuff()->m_aItem[10] = pMsg->m_Item11;
		CustomStuff()->m_aItem[11] = pMsg->m_Item12;
		CustomStuff()->m_Gold = pMsg->m_Gold;
	}
	else if(MsgId == NETMSGTYPE_SV_SOUNDGLOBAL)
	{
		if(m_SuppressEvents)
			return;

		// don't enqueue pseudo-global sounds from demos (created by PlayAndRecord)
		CNetMsg_Sv_SoundGlobal *pMsg = (CNetMsg_Sv_SoundGlobal *)pRawMsg;
		if(pMsg->m_SoundID == SOUND_CTF_DROP || pMsg->m_SoundID == SOUND_CTF_RETURN ||
			pMsg->m_SoundID == SOUND_CTF_CAPTURE || pMsg->m_SoundID == SOUND_CTF_GRAB_EN ||
			pMsg->m_SoundID == SOUND_CTF_GRAB_PL)
			g_GameClient.m_pSounds->Enqueue(CSounds::CHN_GLOBAL, pMsg->m_SoundID);
		else
			g_GameClient.m_pSounds->Play(CSounds::CHN_GLOBAL, pMsg->m_SoundID, 1.0f);
	}
}

void CGameClient::OnStateChange(int NewState, int OldState)
{
	// reset everything when not already connected (to keep gathered stuff)
	if(NewState < IClient::STATE_ONLINE)
		OnReset();

	// then change the state
	for(int i = 0; i < m_All.m_Num; i++)
		m_All.m_paComponents[i]->OnStateChange(NewState, OldState);
}

void CGameClient::OnShutdown() {}
void CGameClient::OnEnterGame()
{
	CustomStuff()->Reset();
}

void CGameClient::OnGameOver()
{
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK && g_Config.m_ClEditor == 0)
		Client()->AutoScreenshot_Start();
}

void CGameClient::OnStartGame()
{
	CustomStuff()->Reset();
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		Client()->DemoRecorder_HandleAutoStart();
}

void CGameClient::OnRconLine(const char *pLine)
{
	m_pGameConsole->PrintLine(CGameConsole::CONSOLETYPE_REMOTE, pLine);
}



void CGameClient::AddPlayerSplatter(vec2 Pos, vec4 Color)
{
	for (int c = 0; c < MAX_CLIENTS; c++)
	{
		if (CustomStuff()->m_aPlayerInfo[c].m_InUse)
		{
			if (abs(CustomStuff()->m_aPlayerInfo[c].Pos().x - Pos.x) < 250 && abs(CustomStuff()->m_aPlayerInfo[c].Pos().y - Pos.y) < 250)
			{
				vec2 At;
				if (!Collision()->IntersectLine(CustomStuff()->m_aPlayerInfo[c].Pos(), Pos, 0x0, 0x0))
					CustomStuff()->m_aPlayerInfo[c].AddSplatter(Color);		
			}							
		}
	}
}


void CGameClient::ProcessEvents()
{
	if(m_SuppressEvents)
		return;

	int SnapType = IClient::SNAP_CURRENT;
	int Num = Client()->SnapNumItems(SnapType);
	for(int Index = 0; Index < Num; Index++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(SnapType, Index, &Item);

		if(Item.m_Type == NETEVENTTYPE_BUILDINGHIT)
		{
			CNetEvent_BuildingHit *ev = (CNetEvent_BuildingHit *)pData;
			for (int i = 0; i < 19; i++)
				g_GameClient.m_pEffects->Spark(vec2(ev->m_X, ev->m_Y));
			
			m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_METAL_HIT, 1.0f, vec2(ev->m_X, ev->m_Y));
		}
		
		if(Item.m_Type == NETEVENTTYPE_FLAMEHIT)
		{
			CNetEvent_BuildingHit *ev = (CNetEvent_BuildingHit *)pData;
			for (int i = 0; i < 2; i++)
				g_GameClient.m_pEffects->Flame(vec2(ev->m_X, ev->m_Y), vec2(frandom()-frandom(), frandom()-frandom()), 0.7f);
			
			//m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_METAL_HIT, 1.0f, vec2(ev->m_X, ev->m_Y));
		}
			
		if(Item.m_Type == NETEVENTTYPE_DAMAGEIND)
		{
			CNetEvent_DamageInd *ev = (CNetEvent_DamageInd *)pData;
			
			// 0 - 200
			int BloodAmount = ev->m_Damage * g_Config.m_GoreBlood / 30.0f;
			
			if (ev->m_Damage > 0 && BloodAmount <= 0)
				BloodAmount = 1;
			
			if (BloodAmount > 0)
			{
				for (int i = 0; i < BloodAmount; i++)
					g_GameClient.m_pEffects->Blood(vec2(ev->m_X, ev->m_Y), RandomDir()*0.3f - GetDirection(ev->m_Angle)*2.0f, CustomStuff()->BloodColor(ev->m_ClientID));
			}
			
			g_GameClient.m_pEffects->DamageInd(vec2(ev->m_X, ev->m_Y), RandomDir()*0.3f - GetDirection(ev->m_Angle)*2.0f, abs(ev->m_Damage), GetPlayerColor(ev->m_ClientID));

			if (BloodAmount > 0)
				AddPlayerSplatter(vec2(ev->m_X, ev->m_Y), CustomStuff()->BloodColor(ev->m_ClientID));
		}
		else if(Item.m_Type == NETEVENTTYPE_EXPLOSION)
		{
			CNetEvent_Explosion *ev = (CNetEvent_Explosion *)pData;
			g_GameClient.m_pEffects->Explosion(vec2(ev->m_X, ev->m_Y), ev->m_Weapon);
			
			// todo: readd camera shake
			float d = distance(CustomStuff()->m_LocalPos, vec2(ev->m_X, ev->m_Y));
			float s = GetExplosionSize(ev->m_Weapon);
			
			if (d < s)
			{
				float a = ScreenshakeAmount(ev->m_Weapon);
				
				if (a > 0)
					CustomStuff()->SetScreenshake(a * (0.5f + (s-d)*0.5f));
			}
		}
		else if(Item.m_Type == NETEVENTTYPE_REPAIR)
		{
			CNetEvent_Repair *ev = (CNetEvent_Repair *)pData;
			g_GameClient.m_pEffects->Repair(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_AMMOFILL)
		{
			CNetEvent_AmmoFill *ev = (CNetEvent_AmmoFill *)pData;
			g_GameClient.m_pEffects->AmmoFill(vec2(ev->m_X, ev->m_Y), ev->m_Weapon);
		}
		else if(Item.m_Type == NETEVENTTYPE_FLAMEEXPLOSION)
		{
			CNetEvent_FlameExplosion *ev = (CNetEvent_FlameExplosion *)pData;
			g_GameClient.m_pEffects->FlameExplosion(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_HAMMERHIT)
		{
			CNetEvent_HammerHit *ev = (CNetEvent_HammerHit *)pData;
			g_GameClient.m_pEffects->HammerHit(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_FX)
		{
			CNetEvent_FX *ev = (CNetEvent_FX *)pData;
			g_GameClient.m_pEffects->SpriteSheet(ev->m_FX, vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_EFFECT)
		{
			//CNetEvent_Effect *ev = (CNetEvent_Effect *)pData;
			//CustomStuff()->m_aPlayerInfo[ev->m_ClientID].m_EffectIntensity[ev->m_EffectID] = 1.0f;
		}
		else if(Item.m_Type == NETEVENTTYPE_LAZER)
		{
			CNetEvent_Lazer *ev = (CNetEvent_Lazer *)pData;
			g_GameClient.m_pEffects->Lazer(vec2(ev->m_X, ev->m_Y), ev->m_Height);
		}
		else if(Item.m_Type == NETEVENTTYPE_SWORDTRACER)
		{
			//CNetEvent_Swordtracer *ev = (CNetEvent_Swordtracer *)pData;
			//g_GameClient.m_pEffects->SwordHit(vec2(ev->m_X, ev->m_Y), ev->m_Angle);
		}
		else if(Item.m_Type == NETEVENTTYPE_SPAWN)
		{
			CNetEvent_Spawn *ev = (CNetEvent_Spawn *)pData;
			g_GameClient.m_pEffects->PlayerSpawn(vec2(ev->m_X, ev->m_Y));
		}
		else if(Item.m_Type == NETEVENTTYPE_DEATH)
		{
			CNetEvent_Death *ev = (CNetEvent_Death *)pData;
			g_GameClient.m_pEffects->PlayerDeath(vec2(ev->m_X, ev->m_Y), ev->m_ClientID);
			CustomStuff()->m_aPlayerInfo[ev->m_ClientID].Reset();
		}
		else if(Item.m_Type == NETEVENTTYPE_SOUNDWORLD)
		{
			CNetEvent_SoundWorld *ev = (CNetEvent_SoundWorld *)pData;
			g_GameClient.m_pSounds->PlayAt(CSounds::CHN_WORLD, ev->m_SoundID, 1.0f, vec2(ev->m_X, ev->m_Y));
		}
	}
}

void CGameClient::OnNewSnapshot()
{
	m_NewTick = true;

	// clear out the invalid pointers
	mem_zero(&g_GameClient.m_Snap, sizeof(g_GameClient.m_Snap));
	m_Snap.m_LocalClientID = -1;

	// secure snapshot
	{
		int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
		for(int Index = 0; Index < Num; Index++)
		{
			IClient::CSnapItem Item;
			void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, Index, &Item);
			if(m_NetObjHandler.ValidateObj(Item.m_Type, pData, Item.m_DataSize) != 0)
			{
				if(g_Config.m_Debug)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "invalidated index=%d type=%d (%s) size=%d id=%d", Index, Item.m_Type, m_NetObjHandler.GetObjName(Item.m_Type), Item.m_DataSize, Item.m_ID);
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
				}
				Client()->SnapInvalidateItem(IClient::SNAP_CURRENT, Index);
			}
		}
	}

	ProcessEvents();

	if(g_Config.m_DbgStress)
	{
		if((Client()->GameTick()%100) == 0)
		{
			char aMessage[64];
			int MsgLen = rand()%(sizeof(aMessage)-1);
			for(int i = 0; i < MsgLen; i++)
				aMessage[i] = 'a'+(rand()%('z'-'a'));
			aMessage[MsgLen] = 0;

			CNetMsg_Cl_Say Msg;
			Msg.m_Team = rand()&1;
			Msg.m_pMessage = aMessage;
			Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
		}
	}

	// go trough all the items in the snapshot and gather the info we want
	{
		m_Snap.m_aTeamSize[TEAM_RED] = m_Snap.m_aTeamSize[TEAM_BLUE] = 0;

		int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
		for(int i = 0; i < Num; i++)
		{
			IClient::CSnapItem Item;
			const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

			if(Item.m_Type == NETOBJTYPE_CLIENTINFO)
			{
				const CNetObj_ClientInfo *pInfo = (const CNetObj_ClientInfo *)pData;
				int ClientID = Item.m_ID;
				IntsToStr(&pInfo->m_Name0, 4, m_aClients[ClientID].m_aName);
				IntsToStr(&pInfo->m_Clan0, 3, m_aClients[ClientID].m_aClan);
				m_aClients[ClientID].m_Country = pInfo->m_Country;
				IntsToStr(&pInfo->m_Topper0, 6, m_aClients[ClientID].m_aTopperName);
				IntsToStr(&pInfo->m_Eye0, 6, m_aClients[ClientID].m_aEyeName);
				IntsToStr(&pInfo->m_Head0, 6, m_aClients[ClientID].m_aHeadName);
				IntsToStr(&pInfo->m_Body0, 6, m_aClients[ClientID].m_aBodyName);
				IntsToStr(&pInfo->m_Hand0, 6, m_aClients[ClientID].m_aHandName);
				IntsToStr(&pInfo->m_Foot0, 6, m_aClients[ClientID].m_aFootName);

				m_aClients[ClientID].m_ColorBody = pInfo->m_ColorBody;
				m_aClients[ClientID].m_ColorFeet = pInfo->m_ColorFeet;
				m_aClients[ClientID].m_ColorTopper = pInfo->m_ColorTopper;
				m_aClients[ClientID].m_ColorSkin = pInfo->m_ColorSkin;
				m_aClients[ClientID].m_BloodColor = pInfo->m_BloodColor;
				
				m_aClients[ClientID].m_IsBot = pInfo->m_IsBot;
				m_aClients[ClientID].m_BloodColor = pInfo->m_BloodColor;

				// prepare the info
				
				if(m_aClients[ClientID].m_aTopperName[0] == 'x' || m_aClients[ClientID].m_aTopperName[1] == '_')
					str_copy(m_aClients[ClientID].m_aTopperName, "default", 64);
					
				if(m_aClients[ClientID].m_aEyeName[0] == 'x' || m_aClients[ClientID].m_aEyeName[1] == '_')
					str_copy(m_aClients[ClientID].m_aEyeName, "default", 64);

				if(m_aClients[ClientID].m_aHeadName[0] == 'x' || m_aClients[ClientID].m_aHeadName[1] == '_')
					str_copy(m_aClients[ClientID].m_aHeadName, "default", 64);

				if(m_aClients[ClientID].m_aBodyName[0] == 'x' || m_aClients[ClientID].m_aBodyName[1] == '_')
					str_copy(m_aClients[ClientID].m_aBodyName, "default", 64);

				if(m_aClients[ClientID].m_aFootName[0] == 'x' || m_aClients[ClientID].m_aFootName[1] == '_')
					str_copy(m_aClients[ClientID].m_aFootName, "default", 64);

				if(m_aClients[ClientID].m_aHandName[0] == 'x' || m_aClients[ClientID].m_aHandName[1] == '_')
					str_copy(m_aClients[ClientID].m_aHandName, "default", 64);

				m_aClients[ClientID].m_SkinInfo.m_ColorBody = m_pSkins->GetColorV4(m_aClients[ClientID].m_ColorBody);
				m_aClients[ClientID].m_SkinInfo.m_ColorFeet = m_pSkins->GetColorV4(m_aClients[ClientID].m_ColorFeet);
				m_aClients[ClientID].m_SkinInfo.m_ColorTopper = m_pSkins->GetColorV4(m_aClients[ClientID].m_ColorTopper);
				m_aClients[ClientID].m_SkinInfo.m_ColorSkin = m_pSkins->GetColorV4(m_aClients[ClientID].m_ColorSkin);
				m_aClients[ClientID].m_SkinInfo.m_IsBot = pInfo->m_IsBot;
				m_aClients[ClientID].m_SkinInfo.m_BloodColor = pInfo->m_BloodColor;
				m_aClients[ClientID].m_SkinInfo.m_Size = 64;


				
				// find new skin
				m_aClients[ClientID].m_TopperID = g_GameClient.m_pSkins->FindTopper(m_aClients[ClientID].m_aTopperName);
				
				if(m_aClients[ClientID].m_TopperID < 0)
				{
					m_aClients[ClientID].m_TopperID = g_GameClient.m_pSkins->FindTopper("default");
					if(m_aClients[ClientID].m_TopperID < 0)
						m_aClients[ClientID].m_TopperID = 0;
				}	
				
				m_aClients[ClientID].m_EyeID = g_GameClient.m_pSkins->FindEye(m_aClients[ClientID].m_aEyeName);
				
				if(m_aClients[ClientID].m_EyeID < 0)
				{
					m_aClients[ClientID].m_EyeID = g_GameClient.m_pSkins->FindEye("default");
					if(m_aClients[ClientID].m_EyeID < 0)
						m_aClients[ClientID].m_EyeID = 0;
				}

				m_aClients[ClientID].m_HeadID = g_GameClient.m_pSkins->FindHead(m_aClients[ClientID].m_aHeadName);
				
				if(m_aClients[ClientID].m_HeadID < 0)
				{
					m_aClients[ClientID].m_HeadID = g_GameClient.m_pSkins->FindHead("default");
					if(m_aClients[ClientID].m_HeadID < 0)
						m_aClients[ClientID].m_HeadID = 0;
				}

				m_aClients[ClientID].m_BodyID = g_GameClient.m_pSkins->FindBody(m_aClients[ClientID].m_aBodyName);
				
				if(m_aClients[ClientID].m_BodyID < 0)
				{
					m_aClients[ClientID].m_BodyID = g_GameClient.m_pSkins->FindBody("default");
					if(m_aClients[ClientID].m_BodyID < 0)
						m_aClients[ClientID].m_BodyID = 0;
				}

				m_aClients[ClientID].m_FootID = g_GameClient.m_pSkins->FindFoot(m_aClients[ClientID].m_aFootName);
				
				if(m_aClients[ClientID].m_FootID < 0)
				{
					m_aClients[ClientID].m_FootID = g_GameClient.m_pSkins->FindFoot("default");
					if(m_aClients[ClientID].m_FootID < 0)
						m_aClients[ClientID].m_FootID = 0;
				}

				m_aClients[ClientID].m_HandID = g_GameClient.m_pSkins->FindHand(m_aClients[ClientID].m_aHandName);
				
				if(m_aClients[ClientID].m_HandID < 0)
				{
					m_aClients[ClientID].m_HandID = g_GameClient.m_pSkins->FindHand("default");
					if(m_aClients[ClientID].m_HandID < 0)
						m_aClients[ClientID].m_HandID = 0;
				}


				m_aClients[ClientID].m_SkinInfo.m_TopperTexture = g_GameClient.m_pSkins->GetTopper(m_aClients[ClientID].m_TopperID)->m_Texture;
				m_aClients[ClientID].m_SkinInfo.m_EyeTexture = g_GameClient.m_pSkins->GetEye(m_aClients[ClientID].m_EyeID)->m_Texture;
				m_aClients[ClientID].m_SkinInfo.m_HeadTexture = g_GameClient.m_pSkins->GetHead(m_aClients[ClientID].m_HeadID)->m_Texture;
				m_aClients[ClientID].m_SkinInfo.m_BodyTexture = g_GameClient.m_pSkins->GetBody(m_aClients[ClientID].m_BodyID)->m_Texture;
				m_aClients[ClientID].m_SkinInfo.m_HandTexture = g_GameClient.m_pSkins->GetHand(m_aClients[ClientID].m_HandID)->m_Texture;
				m_aClients[ClientID].m_SkinInfo.m_FootTexture = g_GameClient.m_pSkins->GetFoot(m_aClients[ClientID].m_FootID)->m_Texture;
				
				/*
					m_aClients[ClientID].m_SkinInfo.m_ColorBody = vec4(1,1,1,1);
					m_aClients[ClientID].m_SkinInfo.m_ColorFeet = vec4(1,1,1,1);
					m_aClients[ClientID].m_SkinInfo.m_ColorTopper = vec4(1,1,1,1);
					m_aClients[ClientID].m_SkinInfo.m_ColorSkin = vec4(1,1,1,1);
				*/

				m_aClients[ClientID].UpdateRenderInfo();

			}
			else if(Item.m_Type == NETOBJTYPE_PLAYERINFO)
			{
				const CNetObj_PlayerInfo *pInfo = (const CNetObj_PlayerInfo *)pData;

				m_aClients[pInfo->m_ClientID].m_Team = pInfo->m_Team;
				m_aClients[pInfo->m_ClientID].m_Active = true;
				m_Snap.m_paPlayerInfos[pInfo->m_ClientID] = pInfo;
				m_Snap.m_NumPlayers++;

				if(pInfo->m_Local)
				{
					m_Snap.m_LocalClientID = Item.m_ID;
					m_Snap.m_pLocalInfo = pInfo;

					if(pInfo->m_Team == TEAM_SPECTATORS || pInfo->m_Spectating)
					{
						m_Snap.m_SpecInfo.m_Active = true;
						m_Snap.m_SpecInfo.m_SpectatorID = SPEC_FREEVIEW;
					}
				}

				// calculate team-balance
				if(pInfo->m_Team != TEAM_SPECTATORS)
					m_Snap.m_aTeamSize[pInfo->m_Team]++;

			}
			else if(Item.m_Type == NETOBJTYPE_CHARACTER)
			{
				const void *pOld = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_CHARACTER, Item.m_ID);
				m_Snap.m_aCharacters[Item.m_ID].m_Cur = *((const CNetObj_Character *)pData);
				if(pOld)
				{
					m_Snap.m_aCharacters[Item.m_ID].m_Active = true;
					m_Snap.m_aCharacters[Item.m_ID].m_Prev = *((const CNetObj_Character *)pOld);

					if(m_Snap.m_aCharacters[Item.m_ID].m_Prev.m_Tick)
						Evolve(&m_Snap.m_aCharacters[Item.m_ID].m_Prev, Client()->PrevGameTick());
					if(m_Snap.m_aCharacters[Item.m_ID].m_Cur.m_Tick)
						Evolve(&m_Snap.m_aCharacters[Item.m_ID].m_Cur, Client()->GameTick());
				}
			}
			else if(Item.m_Type == NETOBJTYPE_BALL)
			{
				const void *pOld = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_BALL, 0);
				m_Snap.m_Ball.m_Cur = *((const CNetObj_Ball *)pData);
				if(pOld)
				{
					m_Snap.m_Ball.m_Active = true;
					m_Snap.m_Ball.m_Prev = *((const CNetObj_Ball *)pOld);

					if(m_Snap.m_Ball.m_Prev.m_Tick)
						EvolveBall(&m_Snap.m_Ball.m_Prev, Client()->PrevGameTick());
					if(m_Snap.m_Ball.m_Cur.m_Tick)
						EvolveBall(&m_Snap.m_Ball.m_Cur, Client()->GameTick());
				}
			}
			else if(Item.m_Type == NETOBJTYPE_SPECTATORINFO)
			{
				m_Snap.m_pSpectatorInfo = (const CNetObj_SpectatorInfo *)pData;
				m_Snap.m_pPrevSpectatorInfo = (const CNetObj_SpectatorInfo *)Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_SPECTATORINFO, Item.m_ID);

				m_Snap.m_SpecInfo.m_SpectatorID = m_Snap.m_pSpectatorInfo->m_SpectatorID;
			}
			else if(Item.m_Type == NETOBJTYPE_GAMEINFO)
			{
				static bool s_GameOver = 0;
				m_Snap.m_pGameInfoObj = (const CNetObj_GameInfo *)pData;
				if(!s_GameOver && m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
					OnGameOver();
				else if(s_GameOver && !(m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
					OnStartGame();
				s_GameOver = m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER;
			}
			else if(Item.m_Type == NETOBJTYPE_GAMEDATA)
			{
				m_Snap.m_pGameDataObj = (const CNetObj_GameData *)pData;
				m_Snap.m_GameDataSnapID = Item.m_ID;
				if(m_Snap.m_pGameDataObj->m_FlagCarrierRed == FLAG_TAKEN)
				{
					if(m_FlagDropTick[TEAM_RED] == 0)
						m_FlagDropTick[TEAM_RED] = Client()->GameTick();
				}
				else if(m_FlagDropTick[TEAM_RED] != 0)
						m_FlagDropTick[TEAM_RED] = 0;
				if(m_Snap.m_pGameDataObj->m_FlagCarrierBlue == FLAG_TAKEN)
				{
					if(m_FlagDropTick[TEAM_BLUE] == 0)
						m_FlagDropTick[TEAM_BLUE] = Client()->GameTick();
				}
				else if(m_FlagDropTick[TEAM_BLUE] != 0)
						m_FlagDropTick[TEAM_BLUE] = 0;
			}
			else if(Item.m_Type == NETOBJTYPE_FLAG)
			{
				m_Snap.m_paFlags[Item.m_ID%2] = (const CNetObj_Flag *)pData;
			}
			else if(Item.m_Type == NETOBJTYPE_DROID)
			{
			}
			else if(Item.m_Type == NETOBJTYPE_BUILDING)
			{
				const CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
				
				if (pBuilding->m_Type == BUILDING_JUMPPAD)
				{
					if (m_Snap.m_ImpactCount < MAX_IMPACTS)
					{
						vec2 p = vec2(pBuilding->m_X, pBuilding->m_Y);
						m_Snap.m_aImpactPos[m_Snap.m_ImpactCount] = vec4(p.x-64, p.y-8-16, p.x+64, p.y-8+16);
					}
					
					m_Snap.m_ImpactCount++;
				}
			}
		}
	}

	// setup local pointers
	if(m_Snap.m_LocalClientID >= 0)
	{
		CSnapState::CCharacterInfo *c = &m_Snap.m_aCharacters[m_Snap.m_LocalClientID];
		if(c->m_Active)
		{
			m_Snap.m_pLocalCharacter = &c->m_Cur;
			m_Snap.m_pLocalPrevCharacter = &c->m_Prev;
			m_LocalCharacterPos = vec2(m_Snap.m_pLocalCharacter->m_X, m_Snap.m_pLocalCharacter->m_Y);
		}
		else if(Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_CHARACTER, m_Snap.m_LocalClientID))
		{
			// player died
			m_pControls->OnPlayerDeath();
		}
	}
	else
	{
		m_Snap.m_SpecInfo.m_Active = true;
		if(Client()->State() == IClient::STATE_DEMOPLAYBACK && DemoPlayer()->GetDemoType() == IDemoPlayer::DEMOTYPE_SERVER &&
			m_DemoSpecID != SPEC_FREEVIEW && m_Snap.m_aCharacters[m_DemoSpecID].m_Active)
			m_Snap.m_SpecInfo.m_SpectatorID = m_DemoSpecID;
		else
			m_Snap.m_SpecInfo.m_SpectatorID = SPEC_FREEVIEW;
	}

	// clear out unneeded client data
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!m_Snap.m_paPlayerInfos[i] && m_aClients[i].m_Active)
			m_aClients[i].Reset();
	}

	// update friend state
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(i == m_Snap.m_LocalClientID || !m_Snap.m_paPlayerInfos[i] || !Friends()->IsFriend(m_aClients[i].m_aName, m_aClients[i].m_aClan, true))
			m_aClients[i].m_Friend = false;
		else
			m_aClients[i].m_Friend = true;
	}

	// sort player infos by score
	mem_copy(m_Snap.m_paInfoByScore, m_Snap.m_paPlayerInfos, sizeof(m_Snap.m_paInfoByScore));
	for(int k = 0; k < MAX_CLIENTS-1; k++) // ffs, bubblesort
	{
		for(int i = 0; i < MAX_CLIENTS-k-1; i++)
		{
			if(m_Snap.m_paInfoByScore[i+1] && (!m_Snap.m_paInfoByScore[i] || m_Snap.m_paInfoByScore[i]->m_Score < m_Snap.m_paInfoByScore[i+1]->m_Score))
			{
				const CNetObj_PlayerInfo *pTmp = m_Snap.m_paInfoByScore[i];
				m_Snap.m_paInfoByScore[i] = m_Snap.m_paInfoByScore[i+1];
				m_Snap.m_paInfoByScore[i+1] = pTmp;
			}
		}
	}
	// sort player infos by team
	int Teams[3] = { TEAM_RED, TEAM_BLUE, TEAM_SPECTATORS };
	int Index = 0;
	for(int Team = 0; Team < 3; ++Team)
	{
		for(int i = 0; i < MAX_CLIENTS && Index < MAX_CLIENTS; ++i)
		{
			if(m_Snap.m_paPlayerInfos[i] && m_Snap.m_paPlayerInfos[i]->m_Team == Teams[Team])
				m_Snap.m_paInfoByTeam[Index++] = m_Snap.m_paPlayerInfos[i];
		}
	}

	CTuningParams StandardTuning;
	CServerInfo CurrentServerInfo;
	Client()->GetServerInfo(&CurrentServerInfo);
	if(CurrentServerInfo.m_aGameType[0] != '0')
	{
		m_ServerMode = SERVERMODE_MOD;
		/*
		if(str_comp(CurrentServerInfo.m_aGameType, "INF") != 0 && str_comp(CurrentServerInfo.m_aGameType, "DEF") != 0 && str_comp(CurrentServerInfo.m_aGameType, "GUN") != 0 && str_comp(CurrentServerInfo.m_aGameType, "DM") != 0 && str_comp(CurrentServerInfo.m_aGameType, "TDM") != 0 && str_comp(CurrentServerInfo.m_aGameType, "CTF") != 0)
			m_ServerMode = SERVERMODE_MOD;
		else if(mem_comp(&StandardTuning, &m_Tuning, sizeof(CTuningParams)) == 0)
			m_ServerMode = SERVERMODE_PURE;
		else
			m_ServerMode = SERVERMODE_PUREMOD;
		*/
	}

	// add tuning to demo
	if(DemoRecorder()->IsRecording() && mem_comp(&StandardTuning, &m_Tuning, sizeof(CTuningParams)) != 0)
	{
		CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
		int *pParams = (int *)&m_Tuning;
		for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
			Msg.AddInt(pParams[i]);
		Client()->SendMsg(&Msg, MSGFLAG_RECORD|MSGFLAG_NOSEND);
	}
}

void CGameClient::OnPredict()
{
	// store the previous values so we can detect prediction errors
	CCharacterCore BeforePrevChar = m_PredictedPrevChar;
	CCharacterCore BeforeChar = m_PredictedChar;
	

	// we can't predict without our own id or own character
	/*
	if(m_Snap.m_LocalClientID == -1 || !m_Snap.m_aCharacters[m_Snap.m_LocalClientID].m_Active)
	{
		CWorldCore World;
		World.m_Tuning = m_Tuning;
		
		// predict only ball
		if (m_Snap.m_Ball.m_Active)
		{
			g_GameClient.m_PredictedBall.Init(&World, Collision());
			World.m_pBall = &g_GameClient.m_PredictedBall;
			g_GameClient.m_PredictedBall.Read(&m_Snap.m_Ball.m_Cur);
		}
		
		for(int Tick = Client()->GameTick()+1; Tick <= Client()->PredGameTick(); Tick++)
		{
			m_LastNewPredictedTick = Tick;
			m_NewPredictedTick = true;
				
			if (World.m_pBall)
			{
				if(Tick == Client()->PredGameTick())
					m_PredictedPrevBall = *World.m_pBall;
			
				World.m_pBall->Tick();
			}
		
			if (World.m_pBall)
			{
				World.m_pBall->Move();
				World.m_pBall->Quantize();
			}
		
			// check if we want to trigger effects
			if(Tick > m_LastNewPredictedTick)
			{
				// ball events
				if (World.m_pBall)
				{
					vec2 Pos = World.m_pBall->m_Pos;
					int Events = World.m_pBall->m_TriggeredEvents;
					
					if(Events&COREEVENT_BALL_BOUNCE)
						g_GameClient.m_pSounds->PlayAndRecord(CSounds::CHN_WORLD, SOUND_PLAYER_JUMP, 1.0f, Pos);
				}
			}
		}
		m_PredictedTick = Client()->PredGameTick();
		
		return;
	}
	*/

	// don't predict anything if we are paused
	if(m_Snap.m_pGameInfoObj && m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
	{
		if (m_Snap.m_LocalClientID == -1 || !m_Snap.m_aCharacters[m_Snap.m_LocalClientID].m_Active)
			return;
		
		if(m_Snap.m_pLocalCharacter)
			m_PredictedChar.Read(m_Snap.m_pLocalCharacter);
		if(m_Snap.m_pLocalPrevCharacter)
			m_PredictedPrevChar.Read(m_Snap.m_pLocalPrevCharacter);  
		return;
	}

	// repredict character
	CWorldCore World;
	World.m_Tuning = m_Tuning;

	// search for players
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_Snap.m_aCharacters[i].m_Active)
			continue;

		g_GameClient.m_aClients[i].m_Predicted.Init(&World, Collision());
		World.m_apCharacters[i] = &g_GameClient.m_aClients[i].m_Predicted;
		g_GameClient.m_aClients[i].m_Predicted.Read(&m_Snap.m_aCharacters[i].m_Cur);
	}
	
	// search for jumppads
	for(int i = 0; i < MAX_IMPACTS; i++)
	{
		if(m_Snap.m_aImpactPos[i].x == 0)
			continue;

		World.m_aImpactPos[i] = m_Snap.m_aImpactPos[i];
	}
	
	// search for the ball
	if (m_Snap.m_Ball.m_Active)
	{
		g_GameClient.m_PredictedBall.Init(&World, Collision());
		World.m_pBall = &g_GameClient.m_PredictedBall;
		g_GameClient.m_PredictedBall.Read(&m_Snap.m_Ball.m_Cur);
	}
	

	// predict
	for(int Tick = Client()->GameTick()+1; Tick <= Client()->PredGameTick(); Tick++)
	{
		// fetch the local
		if(Tick == Client()->PredGameTick() && World.m_apCharacters[m_Snap.m_LocalClientID])
			m_PredictedPrevChar = *World.m_apCharacters[m_Snap.m_LocalClientID];

		// first calculate where everyone should move
		for(int c = 0; c < MAX_CLIENTS; c++)
		{
			if(!World.m_apCharacters[c])
				continue;

			mem_zero(&World.m_apCharacters[c]->m_Input, sizeof(World.m_apCharacters[c]->m_Input));
			if(m_Snap.m_LocalClientID == c)
			{
				// apply player input
				int *pInput = Client()->GetInput(Tick);
				if(pInput)
					World.m_apCharacters[c]->m_Input = *((CNetObj_PlayerInput*)pInput);
				World.m_apCharacters[c]->Tick(true);
			}
			else
				World.m_apCharacters[c]->Tick(false);
		}
		
		if (World.m_pBall)
		{
			if(Tick == Client()->PredGameTick())
				m_PredictedPrevBall = *World.m_pBall;
		
			World.m_pBall->Tick();
		}

		// move all players and quantize their data
		for(int c = 0; c < MAX_CLIENTS; c++)
		{
			if(!World.m_apCharacters[c])
				continue;

			World.m_apCharacters[c]->Move();
			World.m_apCharacters[c]->Quantize();
		}
		
		// move ball
		if (World.m_pBall)
		{
			World.m_pBall->Move();
			World.m_pBall->Quantize();
		}

		// check if we want to trigger effects
		if(Tick > m_LastNewPredictedTick)
		{
			m_LastNewPredictedTick = Tick;
			m_NewPredictedTick = true;

			// ball events
			if (World.m_pBall)
			{
				vec2 Pos = World.m_pBall->m_Pos;
				int Events = World.m_pBall->m_TriggeredEvents;
				
				if(Events&COREEVENT_BALL_BOUNCE)
					g_GameClient.m_pSounds->PlayAndRecord(CSounds::CHN_WORLD, SOUND_BALL_BOUNCE, 1.0f, Pos);
			}
			
			// player events
			if(m_Snap.m_LocalClientID != -1 && World.m_apCharacters[m_Snap.m_LocalClientID])
			{
				vec2 Pos = World.m_apCharacters[m_Snap.m_LocalClientID]->m_Pos;
				int Events = World.m_apCharacters[m_Snap.m_LocalClientID]->m_TriggeredEvents;
				if(Events&COREEVENT_GROUND_JUMP) g_GameClient.m_pSounds->PlayAndRecord(CSounds::CHN_WORLD, SOUND_PLAYER_JUMP, 1.0f, Pos);

				if(Events&COREEVENT_HOOK_ATTACH_GROUND) g_GameClient.m_pSounds->PlayAndRecord(CSounds::CHN_WORLD, SOUND_HOOK_ATTACH_GROUND, 1.0f, Pos);
				if(Events&COREEVENT_HOOK_HIT_NOHOOK) g_GameClient.m_pSounds->PlayAndRecord(CSounds::CHN_WORLD, SOUND_HOOK_NOATTACH, 1.0f, Pos);

				/*if(events&COREEVENT_AIR_JUMP)
				{
					GameClient.effects->air_jump(pos);
					GameClient.sounds->play_and_record(SOUNDS::CHN_WORLD, SOUND_PLAYER_AIRJUMP, 1.0f, pos);
				}*/
			}
		}

		if(Tick == Client()->PredGameTick() && World.m_apCharacters[m_Snap.m_LocalClientID])
			m_PredictedChar = *World.m_apCharacters[m_Snap.m_LocalClientID];
	}

	if(g_Config.m_Debug && g_Config.m_ClPredict && m_PredictedTick == Client()->PredGameTick())
	{
		CNetObj_CharacterCore Before = {0}, Now = {0}, BeforePrev = {0}, NowPrev = {0};
		BeforeChar.Write(&Before);
		BeforePrevChar.Write(&BeforePrev);
		m_PredictedChar.Write(&Now);
		m_PredictedPrevChar.Write(&NowPrev);

		if(mem_comp(&Before, &Now, sizeof(CNetObj_CharacterCore)) != 0)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client", "prediction error");
			for(unsigned i = 0; i < sizeof(CNetObj_CharacterCore)/sizeof(int); i++)
				if(((int *)&Before)[i] != ((int *)&Now)[i])
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "	%d %d %d (%d %d)", i, ((int *)&Before)[i], ((int *)&Now)[i], ((int *)&BeforePrev)[i], ((int *)&NowPrev)[i]);
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "client", aBuf);
				}
		}
	}

	m_PredictedTick = Client()->PredGameTick();
}

void CGameClient::OnActivateEditor()
{
	OnRelease();
}

void CGameClient::CClientData::UpdateRenderInfo()
{
	m_RenderInfo = m_SkinInfo;

	m_RenderInfo.m_Mask = 0;
	
	// force team colors
	if(g_GameClient.m_Snap.m_pGameInfoObj && g_GameClient.m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		const int TeamColors[2] = {2555648, 8912640};
		const int TeamFeetColors[2] = {65280, 10354432};
		if (g_GameClient.m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_INFECTION)
		{
			if (m_Team == TEAM_BLUE)
			{
				//m_RenderInfo.m_ColorBody = g_GameClient.m_pSkins->GetColorV4(0);
				//m_RenderInfo.m_ColorFeet = g_GameClient.m_pSkins->GetColorV4(0);
				m_RenderInfo.m_Mask = 1;
			}
		}
		else if(m_Team >= TEAM_RED && m_Team <= TEAM_BLUE)
		{
			m_RenderInfo.m_ColorBody = g_GameClient.m_pSkins->GetColorV4(TeamColors[m_Team]);
			m_RenderInfo.m_ColorFeet = g_GameClient.m_pSkins->GetColorV4(TeamFeetColors[m_Team]);
		}
		else
		{
			m_RenderInfo.m_ColorBody = g_GameClient.m_pSkins->GetColorV4(12895054);
			m_RenderInfo.m_ColorFeet = g_GameClient.m_pSkins->GetColorV4(12895054);
		}
	}
}

void CGameClient::CClientData::Reset()
{
	m_aName[0] = 0;
	m_aClan[0] = 0;
	m_Country = -1;
	m_TopperID = 0;
	m_EyeID = 0;
	m_HeadID = 0;
	m_BodyID = 0;
	m_FootID = 0;
	m_HandID = 0;
	m_Team = 0;
	m_Angle = 0;
	m_Emoticon = 0;
	m_EmoticonStart = -1;
	m_Active = false;
	m_ChatIgnore = false;
	m_SkinInfo.m_TopperTexture = g_GameClient.m_pSkins->GetTopper(0)->m_Texture;
	m_SkinInfo.m_EyeTexture = g_GameClient.m_pSkins->GetEye(0)->m_Texture;
	m_SkinInfo.m_HeadTexture = g_GameClient.m_pSkins->GetHead(0)->m_Texture;
	m_SkinInfo.m_BodyTexture = g_GameClient.m_pSkins->GetBody(0)->m_Texture;
	m_SkinInfo.m_HandTexture = g_GameClient.m_pSkins->GetHand(0)->m_Texture;
	m_SkinInfo.m_FootTexture = g_GameClient.m_pSkins->GetFoot(0)->m_Texture;
	m_SkinInfo.m_ColorBody = vec4(1,1,1,1);
	m_SkinInfo.m_ColorFeet = vec4(1,1,1,1);
	m_SkinInfo.m_ColorTopper = vec4(1,1,1,1);
	m_SkinInfo.m_ColorSkin = vec4(1,1,1,1);
	m_SkinInfo.m_IsBot = false;
	m_SkinInfo.m_BloodColor = 0;
	UpdateRenderInfo();
}

void CGameClient::SendSwitchTeam(int Team)
{
	CNetMsg_Cl_SetTeam Msg;
	Msg.m_Team = Team;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CGameClient::SendInfo(bool Start)
{
	if(Start)
	{
		CNetMsg_Cl_StartInfo Msg;
		Msg.m_IsBot  = 0;
		Msg.m_pName = g_Config.m_PlayerName;
		Msg.m_pClan = g_Config.m_PlayerClan;
		Msg.m_Country = g_Config.m_PlayerCountry;
		Msg.m_pTopper = g_Config.m_PlayerTopper;
		Msg.m_pEye = g_Config.m_PlayerEye;
		Msg.m_pHead = g_Config.m_PlayerHead;
		Msg.m_pBody = g_Config.m_PlayerBody;
		Msg.m_pHand = g_Config.m_PlayerHand;
		Msg.m_pFoot = g_Config.m_PlayerFoot;
		Msg.m_ColorBody = g_Config.m_PlayerColorBody;
		Msg.m_ColorFeet = g_Config.m_PlayerColorFeet;
		Msg.m_ColorTopper = g_Config.m_PlayerColorTopper;
		Msg.m_ColorSkin = g_Config.m_PlayerColorSkin;
		Msg.m_BloodColor = g_Config.m_PlayerBloodColor;
		Msg.m_Language = g_Config.m_ClLanguagecode;
		Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	}
	else
	{
		CNetMsg_Cl_ChangeInfo Msg;
		Msg.m_pName = g_Config.m_PlayerName;
		Msg.m_pClan = g_Config.m_PlayerClan;
		Msg.m_Country = g_Config.m_PlayerCountry;
		Msg.m_pTopper = g_Config.m_PlayerTopper;
		Msg.m_pEye = g_Config.m_PlayerEye;
		Msg.m_pHead = g_Config.m_PlayerHead;
		Msg.m_pBody = g_Config.m_PlayerBody;
		Msg.m_pHand = g_Config.m_PlayerHand;
		Msg.m_pFoot = g_Config.m_PlayerFoot;
		Msg.m_ColorBody = g_Config.m_PlayerColorBody;
		Msg.m_ColorFeet = g_Config.m_PlayerColorFeet;
		Msg.m_ColorTopper = g_Config.m_PlayerColorTopper;
		Msg.m_ColorSkin = g_Config.m_PlayerColorSkin;
		Msg.m_BloodColor = g_Config.m_PlayerBloodColor;
		Msg.m_Language = g_Config.m_ClLanguagecode;
		Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);

		// activate timer to resend the info if it gets filtered
		if(!m_LastSendInfo || m_LastSendInfo+time_freq()*5 < time_get())
			m_LastSendInfo = time_get();
	}
}

void CGameClient::SendKill(int ClientID)
{
	CNetMsg_Cl_Kill Msg;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

void CGameClient::ConTeam(IConsole::IResult *pResult, void *pUserData)
{
	((CGameClient*)pUserData)->SendSwitchTeam(pResult->GetInteger(0));
}

void CGameClient::ConKill(IConsole::IResult *pResult, void *pUserData)
{
	((CGameClient*)pUserData)->SendKill(-1);
}

void CGameClient::ConchainSpecialInfoupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
		((CGameClient*)pUserData)->SendInfo(false);
}

IGameClient *CreateGameClient()
{
	return &g_GameClient;
}


bool CGameClient::BuildingEnabled()
{
	if (m_Snap.m_pGameInfoObj)
	{
		int Flags = m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_BUILD)
			return true;
	}
	
	return false;
}

bool CGameClient::Survival()
{
	if (m_Snap.m_pGameInfoObj)
	{
		int Flags = m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_SURVIVAL)
			return true;
	}
	
	return false;
}

bool CGameClient::SurvivalAcid()
{
	if (m_Snap.m_pGameInfoObj)
	{
		int Flags = m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (m_Snap.m_pGameInfoObj->m_TimeLimit == 0)
			return false;
				
		if (Flags & GAMEFLAG_ACID)
			return true;
	}
	
	return false;
}

bool CGameClient::IsCoop()
{
	if (m_Snap.m_pGameInfoObj)
	{
		int Flags = m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_COOP)
			return true;
	}
	
	return false;
}


bool CGameClient::IsLocalUndead()
{
	if (m_Snap.m_pGameInfoObj)
	{
		int Flags = m_Snap.m_pGameInfoObj->m_GameFlags;
	
		if (Flags & GAMEFLAG_INFECTION && CustomStuff()->m_LocalTeam == TEAM_BLUE)
			return true;
	}
	
	return false;
}

vec4 CGameClient::GetBloodColor(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return vec4(1, 0, 0, 1);
	
	//return CustomStuff()->m_aPlayerInfo[ClientID].m_Color;
	return vec4(1, 0, 0, 1);
}


vec4 CGameClient::GetPlayerColor(int ClientID)
{
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return vec4(1, 1, 1, 1);
	
	return CustomStuff()->m_aPlayerInfo[ClientID].m_Color;
}

bool CGameClient::BuildingNear(vec2 Pos, float Range)
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return true;

	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		
		if (Item.m_Type == NETOBJTYPE_BUILDING)
		{
			const struct CNetObj_Building *pBuilding = (const CNetObj_Building *)pData;
			vec2 Pos2 = vec2(pBuilding->m_X, pBuilding->m_Y);
			
			if (distance(Pos2, Pos) < Range)
				return true;
		}
		else if (Item.m_Type == NETOBJTYPE_TURRET)
		{
			const struct CNetObj_Turret *pBuilding = (const CNetObj_Turret *)pData;
			vec2 Pos2 = vec2(pBuilding->m_X, pBuilding->m_Y);
			
			if (distance(Pos2, Pos) < Range)
				return true;
		}
		else if (Item.m_Type == NETOBJTYPE_POWERUPPER)
		{
			const struct CNetObj_Powerupper *pBuilding = (const CNetObj_Powerupper *)pData;
			vec2 Pos2 = vec2(pBuilding->m_X, pBuilding->m_Y);
			
			if (distance(Pos2, Pos) < Range)
				return true;
		}
		else if (Item.m_Type == NETOBJTYPE_SHOP)
		{
			const struct CNetObj_Shop *pBuilding = (const CNetObj_Shop *)pData;
			vec2 Pos2 = vec2(pBuilding->m_X, pBuilding->m_Y);
			
			if (distance(Pos2, Pos) < Range)
				return true;
		}
	}
	
	return false;
}