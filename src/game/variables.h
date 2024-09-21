

#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H
#undef GAME_VARIABLES_H // this file will be included several times


// client
MACRO_CONFIG_INT(ClPredict, cl_predict, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Predict client movements")
MACRO_CONFIG_INT(ClNameplates, cl_nameplates, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show name plates")
MACRO_CONFIG_INT(ClNameplatesAlways, cl_nameplates_always, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Always show name plates disregarding of distance")
MACRO_CONFIG_INT(ClNameplatesTeamcolors, cl_nameplates_teamcolors, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Use team colors for name plates")
MACRO_CONFIG_INT(ClNameplatesSize, cl_nameplates_size, 40, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Size of the name plates from 0 to 100%")
MACRO_CONFIG_INT(ClAutoswitchWeapons, cl_autoswitch_weapons, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto switch weapon on pickup")

MACRO_CONFIG_INT(ClShowhud, cl_showhud, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame HUD")
MACRO_CONFIG_INT(ClShowChatFriends, cl_show_chat_friends, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show only chat messages from friends")
MACRO_CONFIG_INT(ClShowfps, cl_showfps, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame FPS counter")

MACRO_CONFIG_INT(ClAirjumpindicator, cl_airjumpindicator, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClThreadsoundloading, cl_threadsoundloading, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Load sound files threaded")

MACRO_CONFIG_INT(ClWarningTeambalance, cl_warning_teambalance, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Warn about team balance")

MACRO_CONFIG_INT(ClLighting, cl_lighting, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Environmental lighting")

MACRO_CONFIG_INT(ClMouseDeadzone, cl_mouse_deadzone, 0, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMouseFollowfactor, cl_mouse_followfactor, 0, 0, 200, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMouseMaxDistance, cl_mouse_max_distance, 400, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(EdShowkeys, ed_showkeys, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

//MACRO_CONFIG_INT(ClFlow, cl_flow, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(ClShowWelcome, cl_show_welcome, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMotdTime, cl_motd_time, 10, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "How long to show the server message of the day")

MACRO_CONFIG_STR(ClVersionServer, cl_version_server, 100, "version.ninslash.com", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Server to use to check for new versions")

MACRO_CONFIG_STR(ClLanguagefile, cl_languagefile, 255, "", CFGFLAG_CLIENT|CFGFLAG_SAVE, "What language file to use")

MACRO_CONFIG_INT(PlayerColorBody, player_color_body, 0, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player body color")
MACRO_CONFIG_INT(PlayerColorFeet, player_color_feet, 0, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player feet color")
MACRO_CONFIG_INT(PlayerColorTopper, player_color_topper, 65535, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player topper color")
MACRO_CONFIG_INT(PlayerColorSkin, player_color_skin, 65535, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player skin color")
MACRO_CONFIG_STR(PlayerTopper, player_topper, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player hair or hat")
MACRO_CONFIG_STR(PlayerEye, player_eye, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player eyes")
MACRO_CONFIG_STR(PlayerHead, player_head, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player head")
MACRO_CONFIG_STR(PlayerBody, player_body, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player body")
MACRO_CONFIG_STR(PlayerHand, player_hand, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player hand")
MACRO_CONFIG_STR(PlayerFoot, player_foot, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player foot")
MACRO_CONFIG_INT(PlayerBloodColor, blood_color, 0, 0, 3, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Blood color")

//MACRO_CONFIG_INT(UiPage, ui_page, 6, 0, 10, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface page")
MACRO_CONFIG_INT(UiPage, ui_page, 1, 0, 25, CFGFLAG_CLIENT, "Interface page")
MACRO_CONFIG_INT(UiToolboxPage, ui_toolbox_page, 0, 0, 2, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toolbox page")
MACRO_CONFIG_STR(UiServerAddress, ui_server_address, 64, "localhost:8303", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface server address")
MACRO_CONFIG_INT(UiScale, ui_scale, 100, 50, 150, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface scale")
MACRO_CONFIG_INT(UiMousesens, ui_mousesens, 100, 5, 100000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Mouse sensitivity for menus/editor")

MACRO_CONFIG_INT(UiColorHue, ui_color_hue, 160, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color hue")
MACRO_CONFIG_INT(UiColorSat, ui_color_sat, 70, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color saturation")
MACRO_CONFIG_INT(UiColorLht, ui_color_lht, 175, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color lightness")
MACRO_CONFIG_INT(UiColorAlpha, ui_color_alpha, 228, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface alpha")

MACRO_CONFIG_INT(GfxNoclip, gfx_noclip, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable clipping")

// server
MACRO_CONFIG_INT(SvWarmup, sv_warmup, 0, 0, 0, CFGFLAG_SERVER, "Number of seconds to do warmup before round starts")
MACRO_CONFIG_STR(SvMotd, sv_motd, 900, "", CFGFLAG_SERVER, "Message of the day to display for the clients")
MACRO_CONFIG_INT(SvTeamdamage, sv_teamdamage, 0, 0, 1, CFGFLAG_SERVER, "Team damage")
MACRO_CONFIG_INT(SvRoundsPerMap, sv_rounds_per_map, 1, 1, 100, CFGFLAG_SERVER, "Number of rounds on each map before rotating")
MACRO_CONFIG_INT(SvRoundSwap, sv_round_swap, 1, 0, 1, CFGFLAG_SERVER, "Swap teams between rounds")
MACRO_CONFIG_INT(SvPowerups, sv_powerups, 1, 0, 1, CFGFLAG_SERVER, "Allow powerups like ninja")
MACRO_CONFIG_INT(SvScorelimit, sv_scorelimit, 0, 0, 1000, CFGFLAG_SERVER, "Score limit (0 disables)")
MACRO_CONFIG_INT(SvTimelimit, sv_timelimit, 0, 0, 1000, CFGFLAG_SERVER, "Time limit in minutes (0 disables)")
MACRO_CONFIG_STR(SvGametype, sv_gametype, 32, "dm", CFGFLAG_SERVER, "Game type (dm, tdm, ctf)")
MACRO_CONFIG_INT(SvTournamentMode, sv_tournament_mode, 0, 0, 1, CFGFLAG_SERVER, "Tournament mode. When enabled, players joins the server as spectator")
MACRO_CONFIG_INT(SvSpamprotection, sv_spamprotection, 1, 0, 1, CFGFLAG_SERVER, "Spam protection")

MACRO_CONFIG_INT(SvRespawnDelay, sv_respawn_delay, 1, 0, 10, CFGFLAG_SERVER, "Time needed to respawn after death")

MACRO_CONFIG_INT(SvSpectatorSlots, sv_spectator_slots, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for spectators")
MACRO_CONFIG_INT(SvTeambalanceTime, sv_teambalance_time, 1, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before autobalancing teams")
MACRO_CONFIG_INT(SvInactiveKickTime, sv_inactivekick_time, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before taking care of inactive players")
MACRO_CONFIG_INT(SvInactiveKick, sv_inactivekick, 1, 0, 2, CFGFLAG_SERVER, "How to deal with inactive players (0=move to spectator, 1=move to free spectator slot/kick, 2=kick)")

MACRO_CONFIG_INT(SvStrictSpectateMode, sv_strict_spectate_mode, 0, 0, 1, CFGFLAG_SERVER, "Restricts information in spectator mode")
MACRO_CONFIG_INT(SvVoteSpectate, sv_vote_spectate, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to move players to spectators")
MACRO_CONFIG_INT(SvVoteSpectateRejoindelay, sv_vote_spectate_rejoindelay, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before a player can rejoin after being moved to spectators by vote")
MACRO_CONFIG_INT(SvVoteKick, sv_vote_kick, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to kick players")
MACRO_CONFIG_INT(SvVoteKickMin, sv_vote_kick_min, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Minimum number of players required to start a kick vote")
MACRO_CONFIG_INT(SvVoteKickBantime, sv_vote_kick_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time to ban a player if kicked by vote. 0 makes it just use kick")


//

MACRO_CONFIG_INT(SvNull, sv_null, 0, 0, 100, CFGFLAG_SERVER, "does nothing")

MACRO_CONFIG_INT(SvDebugMessages, sv_debugmessages, 0, 0, 1, CFGFLAG_SERVER, "Enable debug messages for crash fixing")

MACRO_CONFIG_INT(SvEnableBuilding, sv_enablebuilding, 0, 0, 1, CFGFLAG_SERVER, "Enable building")
MACRO_CONFIG_INT(SvRandomWeapons, sv_randomweapons, 0, 0, 1, CFGFLAG_SERVER, "Enable random weapons")
MACRO_CONFIG_INT(SvWeaponSpawns, sv_weaponspawns, 1, 0, 1, CFGFLAG_SERVER, "Enable weapon spawning")
MACRO_CONFIG_INT(SvLaserWeapon, sv_laserweapon, 0, 0, 1, CFGFLAG_SERVER, "Give laser weapon on spawn")
MACRO_CONFIG_INT(SvSurvivalMode, sv_survivalmode, 0, 0, 1, CFGFLAG_SERVER, "Survival mode")
MACRO_CONFIG_INT(SvSurvivalAcid, sv_survivalacid, 1, 0, 1, CFGFLAG_SERVER, "Survival ends with rising acid")
MACRO_CONFIG_INT(SvSurvivalTime, sv_survivaltime, 0, 0, 600, CFGFLAG_SERVER, "Survival round time limit")
MACRO_CONFIG_INT(SvSurvivalReward, sv_survivalreward, 5, 0, 1000, CFGFLAG_SERVER, "Survival round winner's reward points")
MACRO_CONFIG_INT(SvAbilities, sv_abilities, 0, 0, 1, CFGFLAG_SERVER, "Enable classes & abilities")
MACRO_CONFIG_INT(SvPickupDrops, sv_pickupdrops, 1, 0, 1, CFGFLAG_SERVER, "Pickup drops")
MACRO_CONFIG_INT(SvHealthPickups, sv_healthpickups, 1, 0, 1, CFGFLAG_SERVER, "Enable hp and armor pickups")
MACRO_CONFIG_INT(SvWeaponDrops, sv_weapondrops, 1, 0, 1, CFGFLAG_SERVER, "Enable weapon drops")
MACRO_CONFIG_INT(SvNumBots, sv_bots, 4, 0, 30, CFGFLAG_SERVER, "Max number of bots")
MACRO_CONFIG_INT(SvNoBotTeam, sv_nobotteam, -1, -1, 9, CFGFLAG_SERVER, "")
MACRO_CONFIG_INT(SvBotLevel, sv_botlevel, 6, 1, 30, CFGFLAG_SERVER, "AI level of bots")
MACRO_CONFIG_INT(SvUnlimitedTurbo, sv_unlimited_turbo, 0, 0, 1, CFGFLAG_SERVER, "Unlimited turbo")
MACRO_CONFIG_INT(SvOneHitKill, sv_one_hit_kill, 0, 0, 1, CFGFLAG_SERVER, "One hit kills")
MACRO_CONFIG_INT(SvSelfKillPenalty, sv_selfkillpenalty, 1, 0, 1, CFGFLAG_SERVER, "Penalty for self kills")

MACRO_CONFIG_INT(SvSpectatorUpdateTime, sv_spectatorupdatetime, 5, 1, 20, CFGFLAG_SERVER, "Time between spectator view changes to spectators")
MACRO_CONFIG_INT(SvSpectateOnlyHumans, sv_spectateonlyhumans, 0, 0, 1, CFGFLAG_SERVER, "Spectate only humans")

MACRO_CONFIG_INT(SvDisablePVP, sv_disablepvp, 0, 0, 1, CFGFLAG_SERVER, "Disable PvP damage")
MACRO_CONFIG_INT(SvEnableVotingMenu, sv_enablevotingmenu, 1, 0, 1, CFGFLAG_SERVER, "Enable shopping in voting menu")

MACRO_CONFIG_INT(SvBotsSkipPickups, sv_botsskippickups, 0, 1, 1, CFGFLAG_SERVER, "Bots skips pickups")

MACRO_CONFIG_INT(SvAutoBalance, sv_autobalance, 0, 1, 1, CFGFLAG_SERVER, "Auto balance")
MACRO_CONFIG_INT(SvNoBotNames, sv_nobotnames, 0, 0, 1, CFGFLAG_SERVER, "Hide bot names")
MACRO_CONFIG_INT(SvNearHumanRespawn, sv_nearhumanrespawn, 0, 0, 1, CFGFLAG_SERVER, "Bots respawn near human players")

MACRO_CONFIG_INT(SvNumRounds, sv_numrounds, 7, 1, 100, CFGFLAG_SERVER, "Number of rounds")


// AI
MACRO_CONFIG_INT(SvBotReactTime, sv_bot_react_time, 6, 0, 20, CFGFLAG_SERVER, "Time bot takes to start shooting")
MACRO_CONFIG_INT(SvGodBots, sv_godbots, 0, 0, 1, CFGFLAG_SERVER, "Hard bots")
MACRO_CONFIG_INT(SvRobots, sv_robots, 0, 0, 1, CFGFLAG_SERVER, "Robot bot skins")


MACRO_CONFIG_INT(SvStartGold, sv_startgold, 0, 0, 999, CFGFLAG_SERVER, "Starting gold")

// 
MACRO_CONFIG_INT(SvInfiniteGrenades, sv_infinitegrenades, 0, 0, 1, CFGFLAG_SERVER, "Infinite grenades")



MACRO_CONFIG_INT(SvBroadcastLock, sv_broadcastlock, 3, 0, 5, CFGFLAG_SERVER, "Broadcast lock time (seconds)")

MACRO_CONFIG_INT(SvRandomMaps, sv_random_maps, 1, 0, 1, CFGFLAG_SERVER, "Random select map in maps list (1 = on, 0 = off)")

MACRO_CONFIG_INT(SvSavePlayerdata, sv_save_playerdata, 0, 0, 1, CFGFLAG_SERVER, "Save player data to a file")


// debug
#ifdef CONF_DEBUG // this one can crash the server if not used correctly
	MACRO_CONFIG_INT(DbgDummies, dbg_dummies, 0, 0, 15, CFGFLAG_SERVER, "")
#endif

MACRO_CONFIG_INT(DbgFocus, dbg_focus, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(DbgTuning, dbg_tuning, 0, 0, 1, CFGFLAG_CLIENT, "")
#endif
