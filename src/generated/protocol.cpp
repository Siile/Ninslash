#include <engine/shared/protocol.h>
#include <engine/message.h>
#include "protocol.h"
CNetObjHandler::CNetObjHandler()
{
	m_pMsgFailedOn = "";
	m_pObjCorrectedOn = "";
	m_NumObjCorrections = 0;
}

int CNetObjHandler::NumObjCorrections() { return m_NumObjCorrections; }
const char *CNetObjHandler::CorrectedObjOn() { return m_pObjCorrectedOn; }
const char *CNetObjHandler::FailedMsgOn() { return m_pMsgFailedOn; }





static const int max_int = 0x7fffffff;
int CNetObjHandler::ClampInt(const char *pErrorMsg, int Value, int Min, int Max)
{
	if(Value < Min) { m_pObjCorrectedOn = pErrorMsg; m_NumObjCorrections++; return Min; }
	if(Value > Max) { m_pObjCorrectedOn = pErrorMsg; m_NumObjCorrections++; return Max; }
	return Value;
}
const char *CNetObjHandler::ms_apObjNames[] = {
	"invalid",
	"PlayerInput",
	"Projectile",
	"Laser",
	"LaserFail",
	"Pickup",
	"Weapon",
	"Droid",
	"Building",
	"Block",
	"Turret",
	"Powerupper",
	"Shop",
	"Flag",
	"Radar",
	"GameInfo",
	"GameData",
	"BallCore",
	"Ball",
	"CharacterCore",
	"Character",
	"PlayerInfo",
	"ClientInfo",
	"SpectatorInfo",
	"Common",
	"AmmoFill",
	"Repair",
	"BuildingHit",
	"FlameHit",
	"Explosion",
	"FlameExplosion",
	"Spawn",
	"HammerHit",
	"FX",
	"Lazer",
	"Swordtracer",
	"Death",
	"SoundGlobal",
	"SoundWorld",
	"Block",
	"DamageInd",
	"Effect",
	""
};

int CNetObjHandler::ms_aObjSizes[] = {
	0,
	sizeof(CNetObj_PlayerInput),
	sizeof(CNetObj_Projectile),
	sizeof(CNetObj_Laser),
	sizeof(CNetObj_LaserFail),
	sizeof(CNetObj_Pickup),
	sizeof(CNetObj_Weapon),
	sizeof(CNetObj_Droid),
	sizeof(CNetObj_Building),
	sizeof(CNetObj_Block),
	sizeof(CNetObj_Turret),
	sizeof(CNetObj_Powerupper),
	sizeof(CNetObj_Shop),
	sizeof(CNetObj_Flag),
	sizeof(CNetObj_Radar),
	sizeof(CNetObj_GameInfo),
	sizeof(CNetObj_GameData),
	sizeof(CNetObj_BallCore),
	sizeof(CNetObj_Ball),
	sizeof(CNetObj_CharacterCore),
	sizeof(CNetObj_Character),
	sizeof(CNetObj_PlayerInfo),
	sizeof(CNetObj_ClientInfo),
	sizeof(CNetObj_SpectatorInfo),
	sizeof(CNetEvent_Common),
	sizeof(CNetEvent_AmmoFill),
	sizeof(CNetEvent_Repair),
	sizeof(CNetEvent_BuildingHit),
	sizeof(CNetEvent_FlameHit),
	sizeof(CNetEvent_Explosion),
	sizeof(CNetEvent_FlameExplosion),
	sizeof(CNetEvent_Spawn),
	sizeof(CNetEvent_HammerHit),
	sizeof(CNetEvent_FX),
	sizeof(CNetEvent_Lazer),
	sizeof(CNetEvent_Swordtracer),
	sizeof(CNetEvent_Death),
	sizeof(CNetEvent_SoundGlobal),
	sizeof(CNetEvent_SoundWorld),
	sizeof(CNetEvent_Block),
	sizeof(CNetEvent_DamageInd),
	sizeof(CNetEvent_Effect),
	0
};

const char *CNetObjHandler::ms_apMsgNames[] = {
	"invalid",
	"Sv_Broadcast",
	"Sv_GameVote",
	"Sv_Chat",
	"Sv_KillMsg",
	"Sv_SoundGlobal",
	"Sv_TuneParams",
	"Sv_ExtraProjectile",
	"Sv_ReadyToEnter",
	"Sv_WeaponPickup",
	"Sv_Emoticon",
	"Sv_VoteClearOptions",
	"Sv_VoteOptionListAdd",
	"Sv_VoteOptionAdd",
	"Sv_VoteOptionRemove",
	"Sv_VoteSet",
	"Sv_VoteStatus",
	"Sv_Inventory",
	"Cl_Say",
	"Cl_SetTeam",
	"Cl_SetSpectatorMode",
	"Cl_StartInfo",
	"Cl_ChangeInfo",
	"Cl_Kill",
	"Cl_Emoticon",
	"Cl_DropWeapon",
	"Cl_SelectItem",
	"Cl_UseKit",
	"Cl_Vote",
	"Cl_VoteGameMode",
	"Cl_CallVote",
	"Cl_InventoryAction",
	""
};

const char *CNetObjHandler::GetObjName(int Type)
{
	if(Type < 0 || Type >= NUM_NETOBJTYPES) return "(out of range)";
	return ms_apObjNames[Type];
};

int CNetObjHandler::GetObjSize(int Type)
{
	if(Type < 0 || Type >= NUM_NETOBJTYPES) return 0;
	return ms_aObjSizes[Type];
};

const char *CNetObjHandler::GetMsgName(int Type)
{
	if(Type < 0 || Type >= NUM_NETMSGTYPES) return "(out of range)";
	return ms_apMsgNames[Type];
};

int CNetObjHandler::ValidateObj(int Type, void *pData, int Size)
{
	switch(Type)
	{
	case NETOBJTYPE_PLAYERINPUT:
	{
		CNetObj_PlayerInput *pObj = (CNetObj_PlayerInput *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_PlayerFlags", pObj->m_PlayerFlags, 0, 256);
		return 0;
	}
	
	case NETOBJTYPE_PROJECTILE:
	{
		CNetObj_Projectile *pObj = (CNetObj_Projectile *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_StartTick", pObj->m_StartTick, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_LASER:
	{
		CNetObj_Laser *pObj = (CNetObj_Laser *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_StartTick", pObj->m_StartTick, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_LASERFAIL:
	{
		CNetObj_LaserFail *pObj = (CNetObj_LaserFail *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_PowerLevel", pObj->m_PowerLevel, 0, 100);
		ClampInt("m_StartTick", pObj->m_StartTick, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_PICKUP:
	{
		CNetObj_Pickup *pObj = (CNetObj_Pickup *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Mirror", pObj->m_Mirror, 0, 1);
		ClampInt("m_Type", pObj->m_Type, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_WEAPON:
	{
		CNetObj_Weapon *pObj = (CNetObj_Weapon *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETOBJTYPE_DROID:
	{
		CNetObj_Droid *pObj = (CNetObj_Droid *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_AttackTick", pObj->m_AttackTick, 0, max_int);
		ClampInt("m_Type", pObj->m_Type, 0, 16);
		ClampInt("m_Status", pObj->m_Status, 0, 16);
		ClampInt("m_Anim", pObj->m_Anim, 0, 8);
		ClampInt("m_Dir", pObj->m_Dir, -1, 1);
		return 0;
	}
	
	case NETOBJTYPE_BUILDING:
	{
		CNetObj_Building *pObj = (CNetObj_Building *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Status", pObj->m_Status, 0, max_int);
		ClampInt("m_Type", pObj->m_Type, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_BLOCK:
	{
		CNetObj_Block *pObj = (CNetObj_Block *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETOBJTYPE_TURRET:
	{
		CNetObj_Turret *pObj = (CNetObj_Turret *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_AttackTick", pObj->m_AttackTick, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_POWERUPPER:
	{
		CNetObj_Powerupper *pObj = (CNetObj_Powerupper *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Item", pObj->m_Item, -1, 9);
		return 0;
	}
	
	case NETOBJTYPE_SHOP:
	{
		CNetObj_Shop *pObj = (CNetObj_Shop *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETOBJTYPE_FLAG:
	{
		CNetObj_Flag *pObj = (CNetObj_Flag *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Team", pObj->m_Team, TEAM_RED, TEAM_BLUE);
		return 0;
	}
	
	case NETOBJTYPE_RADAR:
	{
		CNetObj_Radar *pObj = (CNetObj_Radar *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETOBJTYPE_GAMEINFO:
	{
		CNetObj_GameInfo *pObj = (CNetObj_GameInfo *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_GameFlags", pObj->m_GameFlags, 0, 256);
		ClampInt("m_GameStateFlags", pObj->m_GameStateFlags, 0, 256);
		ClampInt("m_RoundStartTick", pObj->m_RoundStartTick, 0, max_int);
		ClampInt("m_WarmupTimer", pObj->m_WarmupTimer, 0, max_int);
		ClampInt("m_ScoreLimit", pObj->m_ScoreLimit, 0, max_int);
		ClampInt("m_TimeLimit", pObj->m_TimeLimit, 0, max_int);
		ClampInt("m_RoundNum", pObj->m_RoundNum, 0, max_int);
		ClampInt("m_RoundCurrent", pObj->m_RoundCurrent, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_GAMEDATA:
	{
		CNetObj_GameData *pObj = (CNetObj_GameData *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_FlagCarrierRed", pObj->m_FlagCarrierRed, FLAG_MISSING, MAX_CLIENTS-1);
		ClampInt("m_FlagCarrierBlue", pObj->m_FlagCarrierBlue, FLAG_MISSING, MAX_CLIENTS-1);
		return 0;
	}
	
	case NETOBJTYPE_BALLCORE:
	{
		CNetObj_BallCore *pObj = (CNetObj_BallCore *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETOBJTYPE_BALL:
	{
		CNetObj_Ball *pObj = (CNetObj_Ball *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETOBJTYPE_CHARACTERCORE:
	{
		CNetObj_CharacterCore *pObj = (CNetObj_CharacterCore *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Health", pObj->m_Health, 0, 100);
		ClampInt("m_HookedPlayer", pObj->m_HookedPlayer, 0, MAX_CLIENTS-1);
		ClampInt("m_HookState", pObj->m_HookState, -1, 6);
		ClampInt("m_HookTick", pObj->m_HookTick, 0, max_int);
		ClampInt("m_Direction", pObj->m_Direction, -1, 1);
		ClampInt("m_Down", pObj->m_Down, 0, 1);
		ClampInt("m_HandJetpack", pObj->m_HandJetpack, 0, 1);
		ClampInt("m_Jetpack", pObj->m_Jetpack, 0, 1);
		ClampInt("m_JetpackPower", pObj->m_JetpackPower, 0, 200);
		ClampInt("m_Wallrun", pObj->m_Wallrun, -100, 100);
		ClampInt("m_Roll", pObj->m_Roll, 0, 32);
		ClampInt("m_Slide", pObj->m_Slide, -10, 32);
		ClampInt("m_JumpTimer", pObj->m_JumpTimer, -10, 10);
		ClampInt("m_Charge", pObj->m_Charge, 0, 1);
		ClampInt("m_ChargeLevel", pObj->m_ChargeLevel, -50, 100);
		ClampInt("m_Action", pObj->m_Action, 0, 64);
		ClampInt("m_Jumped", pObj->m_Jumped, 0, 3);
		ClampInt("m_Sliding", pObj->m_Sliding, 0, 1);
		ClampInt("m_Grounded", pObj->m_Grounded, 0, 1);
		ClampInt("m_Slope", pObj->m_Slope, -1, 1);
		return 0;
	}
	
	case NETOBJTYPE_CHARACTER:
	{
		CNetObj_Character *pObj = (CNetObj_Character *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_PlayerFlags", pObj->m_PlayerFlags, 0, 256);
		ClampInt("m_Armor", pObj->m_Armor, 0, 100);
		ClampInt("m_AmmoCount", pObj->m_AmmoCount, 0, 30);
		ClampInt("m_Emote", pObj->m_Emote, 0, 6);
		ClampInt("m_AttackTick", pObj->m_AttackTick, 0, max_int);
		return 0;
	}
	
	case NETOBJTYPE_PLAYERINFO:
	{
		CNetObj_PlayerInfo *pObj = (CNetObj_PlayerInfo *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Local", pObj->m_Local, 0, 1);
		ClampInt("m_ClientID", pObj->m_ClientID, 0, MAX_CLIENTS-1);
		ClampInt("m_Team", pObj->m_Team, TEAM_SPECTATORS, TEAM_BLUE);
		ClampInt("m_Spectating", pObj->m_Spectating, 0, 1);
		ClampInt("m_WeaponSlot", pObj->m_WeaponSlot, 0, 3);
		ClampInt("m_Kits", pObj->m_Kits, 0, 99);
		return 0;
	}
	
	case NETOBJTYPE_CLIENTINFO:
	{
		CNetObj_ClientInfo *pObj = (CNetObj_ClientInfo *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_IsBot", pObj->m_IsBot, 0, 1);
		ClampInt("m_BloodColor", pObj->m_BloodColor, 0, 3);
		return 0;
	}
	
	case NETOBJTYPE_SPECTATORINFO:
	{
		CNetObj_SpectatorInfo *pObj = (CNetObj_SpectatorInfo *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_SpectatorID", pObj->m_SpectatorID, SPEC_FREEVIEW, MAX_CLIENTS-1);
		return 0;
	}
	
	case NETEVENTTYPE_COMMON:
	{
		CNetEvent_Common *pObj = (CNetEvent_Common *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_AMMOFILL:
	{
		CNetEvent_AmmoFill *pObj = (CNetEvent_AmmoFill *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_Weapon", pObj->m_Weapon, 0, NUM_WEAPONS-1);
		return 0;
	}
	
	case NETEVENTTYPE_REPAIR:
	{
		CNetEvent_Repair *pObj = (CNetEvent_Repair *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_BUILDINGHIT:
	{
		CNetEvent_BuildingHit *pObj = (CNetEvent_BuildingHit *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_FLAMEHIT:
	{
		CNetEvent_FlameHit *pObj = (CNetEvent_FlameHit *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_EXPLOSION:
	{
		CNetEvent_Explosion *pObj = (CNetEvent_Explosion *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_FLAMEEXPLOSION:
	{
		CNetEvent_FlameExplosion *pObj = (CNetEvent_FlameExplosion *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_SPAWN:
	{
		CNetEvent_Spawn *pObj = (CNetEvent_Spawn *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_HAMMERHIT:
	{
		CNetEvent_HammerHit *pObj = (CNetEvent_HammerHit *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_FX:
	{
		CNetEvent_FX *pObj = (CNetEvent_FX *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_FX", pObj->m_FX, 1, NUMFX-1);
		return 0;
	}
	
	case NETEVENTTYPE_LAZER:
	{
		CNetEvent_Lazer *pObj = (CNetEvent_Lazer *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_SWORDTRACER:
	{
		CNetEvent_Swordtracer *pObj = (CNetEvent_Swordtracer *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_DEATH:
	{
		CNetEvent_Death *pObj = (CNetEvent_Death *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_ClientID", pObj->m_ClientID, 0, MAX_CLIENTS-1);
		return 0;
	}
	
	case NETEVENTTYPE_SOUNDGLOBAL:
	{
		CNetEvent_SoundGlobal *pObj = (CNetEvent_SoundGlobal *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_SoundID", pObj->m_SoundID, 0, NUM_SOUNDS-1);
		return 0;
	}
	
	case NETEVENTTYPE_SOUNDWORLD:
	{
		CNetEvent_SoundWorld *pObj = (CNetEvent_SoundWorld *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_SoundID", pObj->m_SoundID, 0, NUM_SOUNDS-1);
		return 0;
	}
	
	case NETEVENTTYPE_BLOCK:
	{
		CNetEvent_Block *pObj = (CNetEvent_Block *)pData;
		if(sizeof(*pObj) != Size) return -1;
		return 0;
	}
	
	case NETEVENTTYPE_DAMAGEIND:
	{
		CNetEvent_DamageInd *pObj = (CNetEvent_DamageInd *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_ClientID", pObj->m_ClientID, -1, MAX_CLIENTS-1);
		return 0;
	}
	
	case NETEVENTTYPE_EFFECT:
	{
		CNetEvent_Effect *pObj = (CNetEvent_Effect *)pData;
		if(sizeof(*pObj) != Size) return -1;
		ClampInt("m_EffectID", pObj->m_EffectID, 0, NUM_EFFECTS-1);
		ClampInt("m_ClientID", pObj->m_ClientID, 0, MAX_CLIENTS-1);
		return 0;
	}
	
	}
	return -1;
};

void *CNetObjHandler::SecureUnpackMsg(int Type, CUnpacker *pUnpacker)
{
	m_pMsgFailedOn = 0;
	switch(Type)
	{
	case NETMSGTYPE_SV_BROADCAST:
	{
		CNetMsg_Sv_Broadcast *pMsg = (CNetMsg_Sv_Broadcast *)m_aMsgData;
		(void)pMsg;
		pMsg->m_pMessage = pUnpacker->GetString();
	} break;
	
	case NETMSGTYPE_SV_GAMEVOTE:
	{
		CNetMsg_Sv_GameVote *pMsg = (CNetMsg_Sv_GameVote *)m_aMsgData;
		(void)pMsg;
		pMsg->m_pName = pUnpacker->GetString();
		pMsg->m_pDescription = pUnpacker->GetString();
		pMsg->m_pImage = pUnpacker->GetString();
		pMsg->m_pPlayers = pUnpacker->GetString();
		pMsg->m_Index = pUnpacker->GetInt();
		pMsg->m_TimeLeft = pUnpacker->GetInt();
	} break;
	
	case NETMSGTYPE_SV_CHAT:
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Team = pUnpacker->GetInt();
		pMsg->m_ClientID = pUnpacker->GetInt();
		pMsg->m_pMessage = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		if(pMsg->m_Team < TEAM_SPECTATORS || pMsg->m_Team > TEAM_BLUE) { m_pMsgFailedOn = "m_Team"; break; }
		if(pMsg->m_ClientID < -1 || pMsg->m_ClientID > MAX_CLIENTS-1) { m_pMsgFailedOn = "m_ClientID"; break; }
	} break;
	
	case NETMSGTYPE_SV_KILLMSG:
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Killer = pUnpacker->GetInt();
		pMsg->m_Victim = pUnpacker->GetInt();
		pMsg->m_Weapon = pUnpacker->GetInt();
		pMsg->m_ModeSpecial = pUnpacker->GetInt();
		if(pMsg->m_Killer < 0 || pMsg->m_Killer > MAX_CLIENTS-1) { m_pMsgFailedOn = "m_Killer"; break; }
		if(pMsg->m_Victim < 0 || pMsg->m_Victim > MAX_CLIENTS-1) { m_pMsgFailedOn = "m_Victim"; break; }
	} break;
	
	case NETMSGTYPE_SV_SOUNDGLOBAL:
	{
		CNetMsg_Sv_SoundGlobal *pMsg = (CNetMsg_Sv_SoundGlobal *)m_aMsgData;
		(void)pMsg;
		pMsg->m_SoundID = pUnpacker->GetInt();
		if(pMsg->m_SoundID < 0 || pMsg->m_SoundID > NUM_SOUNDS-1) { m_pMsgFailedOn = "m_SoundID"; break; }
	} break;
	
	case NETMSGTYPE_SV_TUNEPARAMS:
	{
		CNetMsg_Sv_TuneParams *pMsg = (CNetMsg_Sv_TuneParams *)m_aMsgData;
		(void)pMsg;
	} break;
	
	case NETMSGTYPE_SV_EXTRAPROJECTILE:
	{
		CNetMsg_Sv_ExtraProjectile *pMsg = (CNetMsg_Sv_ExtraProjectile *)m_aMsgData;
		(void)pMsg;
	} break;
	
	case NETMSGTYPE_SV_READYTOENTER:
	{
		CNetMsg_Sv_ReadyToEnter *pMsg = (CNetMsg_Sv_ReadyToEnter *)m_aMsgData;
		(void)pMsg;
	} break;
	
	case NETMSGTYPE_SV_WEAPONPICKUP:
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Weapon = pUnpacker->GetInt();
	} break;
	
	case NETMSGTYPE_SV_EMOTICON:
	{
		CNetMsg_Sv_Emoticon *pMsg = (CNetMsg_Sv_Emoticon *)m_aMsgData;
		(void)pMsg;
		pMsg->m_ClientID = pUnpacker->GetInt();
		pMsg->m_Emoticon = pUnpacker->GetInt();
		if(pMsg->m_ClientID < 0 || pMsg->m_ClientID > MAX_CLIENTS-1) { m_pMsgFailedOn = "m_ClientID"; break; }
		if(pMsg->m_Emoticon < 0 || pMsg->m_Emoticon > NUM_EMOTICONS-1) { m_pMsgFailedOn = "m_Emoticon"; break; }
	} break;
	
	case NETMSGTYPE_SV_VOTECLEAROPTIONS:
	{
		CNetMsg_Sv_VoteClearOptions *pMsg = (CNetMsg_Sv_VoteClearOptions *)m_aMsgData;
		(void)pMsg;
	} break;
	
	case NETMSGTYPE_SV_VOTEOPTIONLISTADD:
	{
		CNetMsg_Sv_VoteOptionListAdd *pMsg = (CNetMsg_Sv_VoteOptionListAdd *)m_aMsgData;
		(void)pMsg;
		pMsg->m_NumOptions = pUnpacker->GetInt();
		pMsg->m_pDescription0 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription1 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription2 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription3 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription4 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription5 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription6 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription7 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription8 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription9 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription10 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription11 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription12 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription13 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pDescription14 = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		if(pMsg->m_NumOptions < 1 || pMsg->m_NumOptions > 15) { m_pMsgFailedOn = "m_NumOptions"; break; }
	} break;
	
	case NETMSGTYPE_SV_VOTEOPTIONADD:
	{
		CNetMsg_Sv_VoteOptionAdd *pMsg = (CNetMsg_Sv_VoteOptionAdd *)m_aMsgData;
		(void)pMsg;
		pMsg->m_pDescription = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
	} break;
	
	case NETMSGTYPE_SV_VOTEOPTIONREMOVE:
	{
		CNetMsg_Sv_VoteOptionRemove *pMsg = (CNetMsg_Sv_VoteOptionRemove *)m_aMsgData;
		(void)pMsg;
		pMsg->m_pDescription = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
	} break;
	
	case NETMSGTYPE_SV_VOTESET:
	{
		CNetMsg_Sv_VoteSet *pMsg = (CNetMsg_Sv_VoteSet *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Timeout = pUnpacker->GetInt();
		pMsg->m_pDescription = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pReason = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		if(pMsg->m_Timeout < 0 || pMsg->m_Timeout > 60) { m_pMsgFailedOn = "m_Timeout"; break; }
	} break;
	
	case NETMSGTYPE_SV_VOTESTATUS:
	{
		CNetMsg_Sv_VoteStatus *pMsg = (CNetMsg_Sv_VoteStatus *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Type = pUnpacker->GetInt();
		pMsg->m_Yes = pUnpacker->GetInt();
		pMsg->m_No = pUnpacker->GetInt();
		pMsg->m_Pass = pUnpacker->GetInt();
		pMsg->m_Total = pUnpacker->GetInt();
		pMsg->m_Option5 = pUnpacker->GetInt();
		pMsg->m_Option6 = pUnpacker->GetInt();
		if(pMsg->m_Type < 0 || pMsg->m_Type > 1) { m_pMsgFailedOn = "m_Type"; break; }
		if(pMsg->m_Yes < 0 || pMsg->m_Yes > MAX_CLIENTS) { m_pMsgFailedOn = "m_Yes"; break; }
		if(pMsg->m_No < 0 || pMsg->m_No > MAX_CLIENTS) { m_pMsgFailedOn = "m_No"; break; }
		if(pMsg->m_Pass < 0 || pMsg->m_Pass > MAX_CLIENTS) { m_pMsgFailedOn = "m_Pass"; break; }
		if(pMsg->m_Total < 0 || pMsg->m_Total > MAX_CLIENTS) { m_pMsgFailedOn = "m_Total"; break; }
		if(pMsg->m_Option5 < 0 || pMsg->m_Option5 > MAX_CLIENTS) { m_pMsgFailedOn = "m_Option5"; break; }
		if(pMsg->m_Option6 < 0 || pMsg->m_Option6 > MAX_CLIENTS) { m_pMsgFailedOn = "m_Option6"; break; }
	} break;
	
	case NETMSGTYPE_SV_INVENTORY:
	{
		CNetMsg_Sv_Inventory *pMsg = (CNetMsg_Sv_Inventory *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Item1 = pUnpacker->GetInt();
		pMsg->m_Item2 = pUnpacker->GetInt();
		pMsg->m_Item3 = pUnpacker->GetInt();
		pMsg->m_Item4 = pUnpacker->GetInt();
		pMsg->m_Item5 = pUnpacker->GetInt();
		pMsg->m_Item6 = pUnpacker->GetInt();
		pMsg->m_Item7 = pUnpacker->GetInt();
		pMsg->m_Item8 = pUnpacker->GetInt();
		pMsg->m_Item9 = pUnpacker->GetInt();
		pMsg->m_Item10 = pUnpacker->GetInt();
		pMsg->m_Item11 = pUnpacker->GetInt();
		pMsg->m_Item12 = pUnpacker->GetInt();
		pMsg->m_Gold = pUnpacker->GetInt();
		if(pMsg->m_Gold < 0 || pMsg->m_Gold > 999) { m_pMsgFailedOn = "m_Gold"; break; }
	} break;
	
	case NETMSGTYPE_CL_SAY:
	{
		CNetMsg_Cl_Say *pMsg = (CNetMsg_Cl_Say *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Team = pUnpacker->GetInt();
		pMsg->m_pMessage = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		if(pMsg->m_Team < 0 || pMsg->m_Team > 1) { m_pMsgFailedOn = "m_Team"; break; }
	} break;
	
	case NETMSGTYPE_CL_SETTEAM:
	{
		CNetMsg_Cl_SetTeam *pMsg = (CNetMsg_Cl_SetTeam *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Team = pUnpacker->GetInt();
		if(pMsg->m_Team < TEAM_SPECTATORS || pMsg->m_Team > TEAM_BLUE) { m_pMsgFailedOn = "m_Team"; break; }
	} break;
	
	case NETMSGTYPE_CL_SETSPECTATORMODE:
	{
		CNetMsg_Cl_SetSpectatorMode *pMsg = (CNetMsg_Cl_SetSpectatorMode *)m_aMsgData;
		(void)pMsg;
		pMsg->m_SpectatorID = pUnpacker->GetInt();
		if(pMsg->m_SpectatorID < SPEC_FREEVIEW || pMsg->m_SpectatorID > MAX_CLIENTS-1) { m_pMsgFailedOn = "m_SpectatorID"; break; }
	} break;
	
	case NETMSGTYPE_CL_STARTINFO:
	{
		CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)m_aMsgData;
		(void)pMsg;
		pMsg->m_pName = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pClan = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_Country = pUnpacker->GetInt();
		pMsg->m_pTopper = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pEye = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pHead = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pBody = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pHand = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pFoot = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_ColorBody = pUnpacker->GetInt();
		pMsg->m_ColorFeet = pUnpacker->GetInt();
		pMsg->m_ColorTopper = pUnpacker->GetInt();
		pMsg->m_ColorSkin = pUnpacker->GetInt();
		pMsg->m_BloodColor = pUnpacker->GetInt();
		pMsg->m_IsBot = pUnpacker->GetInt();
		if(pMsg->m_IsBot < 0 || pMsg->m_IsBot > 1) { m_pMsgFailedOn = "m_IsBot"; break; }
	} break;
	
	case NETMSGTYPE_CL_CHANGEINFO:
	{
		CNetMsg_Cl_ChangeInfo *pMsg = (CNetMsg_Cl_ChangeInfo *)m_aMsgData;
		(void)pMsg;
		pMsg->m_pName = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pClan = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_Country = pUnpacker->GetInt();
		pMsg->m_pTopper = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pEye = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pHead = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pBody = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pHand = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_pFoot = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_ColorBody = pUnpacker->GetInt();
		pMsg->m_ColorFeet = pUnpacker->GetInt();
		pMsg->m_ColorTopper = pUnpacker->GetInt();
		pMsg->m_ColorSkin = pUnpacker->GetInt();
		pMsg->m_BloodColor = pUnpacker->GetInt();
		pMsg->m_IsBot = pUnpacker->GetInt();
		if(pMsg->m_IsBot < 0 || pMsg->m_IsBot > 1) { m_pMsgFailedOn = "m_IsBot"; break; }
	} break;
	
	case NETMSGTYPE_CL_KILL:
	{
		CNetMsg_Cl_Kill *pMsg = (CNetMsg_Cl_Kill *)m_aMsgData;
		(void)pMsg;
	} break;
	
	case NETMSGTYPE_CL_EMOTICON:
	{
		CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Emoticon = pUnpacker->GetInt();
		if(pMsg->m_Emoticon < 0 || pMsg->m_Emoticon > NUM_EMOTICONS-1) { m_pMsgFailedOn = "m_Emoticon"; break; }
	} break;
	
	case NETMSGTYPE_CL_DROPWEAPON:
	{
		CNetMsg_Cl_DropWeapon *pMsg = (CNetMsg_Cl_DropWeapon *)m_aMsgData;
		(void)pMsg;
	} break;
	
	case NETMSGTYPE_CL_SELECTITEM:
	{
		CNetMsg_Cl_SelectItem *pMsg = (CNetMsg_Cl_SelectItem *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Item = pUnpacker->GetInt();
		if(pMsg->m_Item < 0 || pMsg->m_Item > 99) { m_pMsgFailedOn = "m_Item"; break; }
	} break;
	
	case NETMSGTYPE_CL_USEKIT:
	{
		CNetMsg_Cl_UseKit *pMsg = (CNetMsg_Cl_UseKit *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Kit = pUnpacker->GetInt();
		pMsg->m_X = pUnpacker->GetInt();
		pMsg->m_Y = pUnpacker->GetInt();
		if(pMsg->m_Kit < 0 || pMsg->m_Kit > 99) { m_pMsgFailedOn = "m_Kit"; break; }
	} break;
	
	case NETMSGTYPE_CL_VOTE:
	{
		CNetMsg_Cl_Vote *pMsg = (CNetMsg_Cl_Vote *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Vote = pUnpacker->GetInt();
		if(pMsg->m_Vote < -1 || pMsg->m_Vote > 1) { m_pMsgFailedOn = "m_Vote"; break; }
	} break;
	
	case NETMSGTYPE_CL_VOTEGAMEMODE:
	{
		CNetMsg_Cl_VoteGameMode *pMsg = (CNetMsg_Cl_VoteGameMode *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Vote = pUnpacker->GetInt();
		if(pMsg->m_Vote < 0 || pMsg->m_Vote > 6) { m_pMsgFailedOn = "m_Vote"; break; }
	} break;
	
	case NETMSGTYPE_CL_CALLVOTE:
	{
		CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Type = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_Value = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
		pMsg->m_Reason = pUnpacker->GetString(CUnpacker::SANITIZE_CC|CUnpacker::SKIP_START_WHITESPACES);
	} break;
	
	case NETMSGTYPE_CL_INVENTORYACTION:
	{
		CNetMsg_Cl_InventoryAction *pMsg = (CNetMsg_Cl_InventoryAction *)m_aMsgData;
		(void)pMsg;
		pMsg->m_Type = pUnpacker->GetInt();
		pMsg->m_Item1 = pUnpacker->GetInt();
		pMsg->m_Item2 = pUnpacker->GetInt();
		pMsg->m_Slot = pUnpacker->GetInt();
		if(pMsg->m_Type < 0 || pMsg->m_Type > 6) { m_pMsgFailedOn = "m_Type"; break; }
	} break;
	
	default:
		m_pMsgFailedOn = "(type out of range)";
		break;
	}
	
	if(pUnpacker->Error())
		m_pMsgFailedOn = "(unpack error)";
	
	if(m_pMsgFailedOn)
		return 0;
	m_pMsgFailedOn = "";
	return m_aMsgData;
};

