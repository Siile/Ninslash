

#include <base/math.h>

#include <engine/demo.h>
#include <engine/keys.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/storage.h>

#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/localization.h>

#include <game/client/ui.h>

#include <game/generated/client_data.h>
#include <engine/shared/config.h>

#include "menus.h"

#if defined(WIN32)
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

static HANDLE serverProcess = -1;
#else
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

static sorted_array<string> s_maplist;
static bool atexitRegistered = false;

static int MapScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	sorted_array<string> *maplist = (sorted_array<string> *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".map") != 0)
		return 0;
	maplist->add(string(pName, l - 4));
	return 0;
}

void CMenus::ServerCreatorInit()
{
	if( s_maplist.size() == 0 )
	{
		Storage()->ListDirectory(IStorage::TYPE_ALL, "maps", MapScan, &s_maplist);
	}
}

static void StopServer()
{
#if defined(WIN32)
	if( serverProcess != -1 )
		TerminateProcess(serverProcess, 0);
	serverProcess = -1;
#elif defined(__ANDROID__)
	system("$SECURE_STORAGE_DIR/busybox killall ninslash_srv");
#else
	system("killall ninslash_srv ninslash_srv_d");
#endif
}

static void StartServer(const char *type, const char *map, int bots)
{
	char aBuf[4096];
	str_format(aBuf, sizeof(aBuf),
		"sv_port 8303\n"
		"sv_name \"%s\"\n"
		"sv_gametype %s\n"
		"sv_map %s\n"
		"sv_maprotation %s\n"
		"sv_preferredteamsize %d\n"
		"sv_scorelimit 0\n"
		"sv_randomweapons 1\n"
		"sv_vanillapickups 1\n"
		"sv_weapondrops 1\n",
		type, type, map, map, bots);

	FILE *ff = fopen("server.cfg", "wb");
	if( !ff )
		return;
	fwrite(aBuf, str_length(aBuf), 1, ff);
	fclose(ff);

#if defined(WIN32)
	serverProcess = (HANDLE) _spawnl(_P_NOWAIT, "ninslash_srv.exe", "ninslash_srv.exe", "-f", "server.cfg", NULL);
#elif defined(__ANDROID__)
	system("$SECURE_STORAGE_DIR/ninslash_srv -f server.cfg >/dev/null 2>&1 &");
#else
	system("./ninslash_srv_d -f server.cfg || ./ninslash_srv -f server.cfg &");
#endif

	if( !atexitRegistered )
		atexit(&StopServer);
	atexitRegistered = true;
}

static bool ServerStatus()
{
#if defined(WIN32)
	return serverProcess != -1;
#elif defined(__ANDROID__)
	int status = system("$SECURE_STORAGE_DIR/busybox sh -c 'ps | grep ninslash_srv'");
	return WEXITSTATUS(status) == 0;
#else
	int status = system("ps | grep ninslash_srv");
	return WEXITSTATUS(status) == 0;
#endif
}

void CMenus::ServerCreatorProcess(CUIRect MainView)
{
	static int s_map = 0;
	static int s_bots = 5;

	static int64 LastUpdateTime = 0;
	static bool ServerRunning = false;
	static bool ServerStarting = false;

	bool ServerStarted = false;

	ServerCreatorInit();

	// background
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 20.0f);
	MainView.Margin(20.0f, &MainView);

	MainView.HSplitTop(10, 0, &MainView);
	CUIRect MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, Localize("Local server"), 20.0f, 0);

	if (time_get() / time_freq() > LastUpdateTime + 2)
	{
		LastUpdateTime = time_get() / time_freq();
		ServerRunning = ServerStatus();
		if (ServerRunning && ServerStarting)
			ServerStarted = true;
		ServerStarting = false;
	}

	MainView.HSplitTop(30, 0, &MainView);
	MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, ServerStarting ? Localize("Server is starting") :
								 ServerRunning ? Localize("Server is running") :
								 Localize("Server stopped"), 20.0f, 0);

	MainView.HSplitTop(50, 0, &MainView);

	CUIRect Button;

	MainView.VSplitLeft(50, 0, &Button);
	Button.h = 30;
	Button.w = 200;
	static int s_StopServerButton = 0;
	if( ServerRunning && DoButton_Menu(&s_StopServerButton, Localize("Stop server"), 0, &Button))
	{
		StopServer();
		LastUpdateTime = time_get() / time_freq() - 2;
	}

	static int s_StartDmServerButton = 0;
	if( !ServerRunning && !ServerStarting && DoButton_Menu(&s_StartDmServerButton, Localize("Start DM server"), 0, &Button))
	{
		StartServer("dm", s_maplist[s_map].cstr(), s_bots);
		LastUpdateTime = time_get() / time_freq(); // We do not actually ping the server, just wait 3 seconds
		ServerStarting = true;
	}

	MainView.VSplitLeft(300, 0, &Button);
	Button.h = 30;
	Button.w = 200;
	static int s_StartInfServerButton = 0;
	if( !ServerRunning && !ServerStarting && DoButton_Menu(&s_StartInfServerButton, Localize("Start INF server"), 0, &Button) )
	{
		StartServer("inf", s_maplist[s_map].cstr(), s_bots);
		LastUpdateTime = time_get() / time_freq(); // We do not actually ping the server, just wait 3 seconds
		ServerStarting = true;
	}

	MainView.VSplitLeft(550, 0, &Button);
	Button.h = 30;
	Button.w = 200;
	static int s_StartCtfServerButton = 0;
	if( !ServerRunning && !ServerStarting && DoButton_Menu(&s_StartCtfServerButton, Localize("Start CTF server"), 0, &Button) )
	{
		StartServer("ctf", s_maplist[s_map].cstr(), s_bots);
		LastUpdateTime = time_get() / time_freq(); // We do not actually ping the server, just wait 3 seconds
		ServerStarting = true;
	}

	static int s_JoinServerButton = 0;
	if(ServerStarted || (ServerRunning && DoButton_Menu(&s_JoinServerButton, Localize("Join server"), 0, &Button)))
	{
		strcpy(g_Config.m_UiServerAddress, "127.0.0.1");
		Client()->Connect(g_Config.m_UiServerAddress);
	}

	MainView.HSplitTop(60, 0, &MainView);

	MainView.VSplitLeft(50, 0, &MsgBox);
	MsgBox.w = 100;

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "%s: %i", Localize("Bots"), s_bots);
	UI()->DoLabelScaled(&MsgBox, aBuf, 20.0f, -1);

	MainView.VSplitLeft(150, 0, &Button);
	Button.h = 30;
	Button.w = 500;

	s_bots = (int)(DoScrollbarH(&s_bots, &Button, s_bots/15.0f)*15.0f+0.1f);

	MainView.HSplitTop(60, 0, &MainView);

	static float s_ScrollValue = 0.0f;
	UiDoListboxStart(&s_ScrollValue, &MainView, 50.0f, Localize("Map"), "", s_maplist.size(), 1, s_map, s_ScrollValue);

	for(int i = 0; i < s_maplist.size(); ++i)
	{
		CListboxItem Item = UiDoListboxNextItem(&s_maplist[i], s_map == i);
		if(Item.m_Visible)
		{
			CUIRect Label;
			//Item.m_Rect.Margin(5.0f, &Item.m_Rect);
			Item.m_Rect.HSplitTop(10.0f, &Item.m_Rect, &Label);
			UI()->DoLabelScaled(&Label, s_maplist[i].cstr(), 20.0f, 0);
		}
	}

	s_map = UiDoListboxEnd(&s_ScrollValue, 0);
}
