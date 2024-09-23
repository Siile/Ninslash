#ifndef GAME_GENERATED_PROTOCOL_H
#define GAME_GENERATED_PROTOCOL_H


#include <engine/message.h>

enum
{
	INPUT_STATE_MASK=0x3f
};

enum
{
	TEAM_NEUTRAL=-1,
	TEAM_SPECTATORS=-1,
	TEAM_RED,
	TEAM_BLUE,

	FLAG_MISSING=-3,
	FLAG_ATSTAND,
	FLAG_TAKEN,

	BUILDING_SAWBLADE=1,
	BUILDING_MINE1,
	BUILDING_MINE2,
	BUILDING_BARREL,
	BUILDING_BARREL2,
	BUILDING_BARREL3,
	BUILDING_TURRET,
	BUILDING_LAZER,
	BUILDING_POWERUPPER,
	BUILDING_BASE,
	BUILDING_STAND,
	BUILDING_FLAMETRAP,
	BUILDING_JUMPPAD,
	BUILDING_SWITCH,
	BUILDING_DOOR1,
	BUILDING_GENERATOR,
	BUILDING_POWERBARREL,
	BUILDING_POWERBARREL2,
	BUILDING_LIGHTNINGWALL,
	BUILDING_LIGHTNINGWALL2,
	BUILDING_REACTOR,
	BUILDING_REACTOR_DESTROYED,
	BUILDING_TESLACOIL,
	BUILDING_SCREEN,
	BUILDING_SHOP,
	
	BSTATUS_REPAIR=1,
	BSTATUS_NOPE,
	BSTATUS_MIRROR,
	BSTATUS_FIRE,
	BSTATUS_ON,
	BSTATUS_EVENT,
	NUM_BSTATUS,
	
	FX_EXPLOSION1=1,
	FX_EXPLOSION2,
	FX_SMALLELECTRIC,
	FX_ELECTRIC,
	FX_SUPERELECTRIC,
	FX_GREEN_EXPLOSION,
	FX_ELECTROHIT,
	FX_ELECTROMINE,
	FX_SHIELDHIT,
	FX_MINE,
	FX_BARREL,
	FX_LAZERLOAD,
	FX_BLOOD1,
	FX_BLOOD2,
	FX_BLOOD3,
	FX_MONSTERDEATH,
	FX_MONSTERSPAWN,
	FX_TAKEOFF,
	FX_FLAME1,
	FX_ROLLDASH,
	NUMFX,
	
	EFFECT_ELECTRODAMAGE=1,
	EFFECT_DEATHRAY,
	EFFECT_SPAWNING,
	EFFECT_DAMAGE,
	EFFECT_INVISIBILITY,
	EFFECT_HP,
	EFFECT_DASH,
	EFFECT_FUEL,
	NUM_EFFECTS,
	
	DEATHTYPE_EMPTY=0,
	DEATHTYPE_SWORD,
	DEATHTYPE_CHAINSAW,
	DEATHTYPE_PISTOL,
	DEATHTYPE_SHOTGUN,
	DEATHTYPE_GRENADELAUNCHER,
	DEATHTYPE_RIFLE,
	DEATHTYPE_ELECTROLAUNCHER,
	DEATHTYPE_LASER,
	DEATHTYPE_FLAMER,
	DEATHTYPE_SAWBLADE,
	DEATHTYPE_SPIKE,
	DEATHTYPE_LANDMINE,
	DEATHTYPE_ELECTROMINE,
	DEATHTYPE_BARREL,
	DEATHTYPE_DEATHRAY,
	DEATHTYPE_DROID_WALKER,
	DEATHTYPE_FLAMETRAP,
	DEATHTYPE_TURRETADDITION,
	DEATHTYPE_POWERBARREL,
	DEATHTYPE_LIGHTNINGWALL,
	DEATHTYPE_TESLACOIL,
	DEATHTYPE_DROID_STAR,
	NUM_DEATHTYPES,
	
	NUM_SLOTS=12,
	NUM_BODIES=7,
	MAX_PLAYERITEMS=2,
	
	SPEC_FREEVIEW=-1,
};

enum
{
	EMOTE_NORMAL=0,
	EMOTE_PAIN,
	EMOTE_HAPPY,
	EMOTE_SURPRISE,
	EMOTE_ANGRY,
	EMOTE_BLINK,
	NUM_EMOTES
};

enum
{
	POWERUP_HEALTH=0,
	POWERUP_AMMO,
	POWERUP_WEAPON,
	POWERUP_ARMOR,
	POWERUP_COIN,
	POWERUP_KIT,
	NUM_POWERUPS
};

enum
{
	EMOTICON_OOP=0,
	EMOTICON_EXCLAMATION,
	EMOTICON_HEARTS,
	EMOTICON_DROP,
	EMOTICON_DOTDOT,
	EMOTICON_MUSIC,
	EMOTICON_SORRY,
	EMOTICON_GHOST,
	EMOTICON_SUSHI,
	EMOTICON_SPLATTEE,
	EMOTICON_DEVILTEE,
	EMOTICON_ZOMG,
	EMOTICON_ZZZ,
	EMOTICON_WTF,
	EMOTICON_EYES,
	EMOTICON_QUESTION,
	NUM_EMOTICONS
};

enum
{
	STATUS_SPAWNING=0,
	STATUS_AFLAME,
	STATUS_SLOWED,
	STATUS_ELECTRIC,
	STATUS_DEATHRAY,
	STATUS_SHIELD,
	STATUS_DASH,
	STATUS_INVISIBILITY,
	STATUS_SLOWMOVING,
	STATUS_BOMBCARRIER,
	STATUS_MASK1,
	STATUS_MASK2,
	STATUS_MASK3,
	NUM_STATUSS
};

enum
{
	BALLSTATUS_STATIONARY=0,
	BALLSTATUS_SUPER,
	NUM_BALLSTATUSS
};

enum
{
	DAMAGETYPE_NORMAL=0,
	DAMAGETYPE_FLAME,
	DAMAGETYPE_ELECTRIC,
	DAMAGETYPE_FLUID,
	NUM_DAMAGETYPES
};

enum
{
	DROIDSTATUS_IDLE=0,
	DROIDSTATUS_HURT,
	DROIDSTATUS_ELECTRIC,
	DROIDSTATUS_TERMINATED,
	NUM_DROIDSTATUSS
};

enum
{
	DROIDTYPE_WALKER=0,
	DROIDTYPE_STAR,
	DROIDTYPE_CRAWLER,
	DROIDTYPE_BOSSCRAWLER,
	DROIDTYPE_FLY,
	NUM_DROIDTYPES
};

enum
{
	DROIDANIM_IDLE=0,
	DROIDANIM_MOVE,
	DROIDANIM_ATTACK,
	DROIDANIM_JUMPATTACK,
	NUM_DROIDANIMS
};

enum
{
	COREACTION_IDLE=0,
	COREACTION_JUMP,
	COREACTION_WALLJUMP,
	COREACTION_ROLL,
	COREACTION_SLIDE,
	COREACTION_SLIDEKICK,
	COREACTION_FALL,
	COREACTION_JUMPPAD,
	COREACTION_HANG,
	NUM_COREACTIONS
};

enum
{
	INVENTORYACTION_SWAP=0,
	INVENTORYACTION_COMBINE,
	INVENTORYACTION_TAKEPART,
	INVENTORYACTION_DROP,
	INVENTORYACTION_SHOP,
	INVENTORYACTION_ROLL,
	NUM_INVENTORYACTIONS
};

enum
{
	RADAR_CHARACTER=0,
	RADAR_HUMAN,
	RADAR_ENEMY,
	RADAR_DOOR,
	RADAR_REACTOR,
	RADAR_BOMB,
	NUM_RADARS
};

enum
{
	PLAYERFLAG_PLAYING = 1<<0,
	PLAYERFLAG_IN_MENU = 1<<1,
	PLAYERFLAG_CHATTING = 1<<2,
	PLAYERFLAG_SCOREBOARD = 1<<3,
};

enum
{
	GAMEFLAG_TEAMS = 1<<0,
	GAMEFLAG_INFECTION = 1<<1,
	GAMEFLAG_COOP = 1<<2,
	GAMEFLAG_SURVIVAL = 1<<3,
	GAMEFLAG_BUILD = 1<<4,
	GAMEFLAG_FLAGS = 1<<5,
	GAMEFLAG_ACID = 1<<6,
};

enum
{
	GAMESTATEFLAG_GAMEOVER = 1<<0,
	GAMESTATEFLAG_SUDDENDEATH = 1<<1,
	GAMESTATEFLAG_PAUSED = 1<<2,
};

enum
{
	NETOBJ_INVALID=0,
	NETOBJTYPE_PLAYERINPUT,
	NETOBJTYPE_PROJECTILE,
	NETOBJTYPE_LASER,
	NETOBJTYPE_LASERFAIL,
	NETOBJTYPE_PICKUP,
	NETOBJTYPE_WEAPON,
	NETOBJTYPE_DROID,
	NETOBJTYPE_BUILDING,
	NETOBJTYPE_BLOCK,
	NETOBJTYPE_TURRET,
	NETOBJTYPE_POWERUPPER,
	NETOBJTYPE_SHOP,
	NETOBJTYPE_FLAG,
	NETOBJTYPE_RADAR,
	NETOBJTYPE_GAMEINFO,
	NETOBJTYPE_GAMEDATA,
	NETOBJTYPE_BALLCORE,
	NETOBJTYPE_BALL,
	NETOBJTYPE_CHARACTERCORE,
	NETOBJTYPE_CHARACTER,
	NETOBJTYPE_PLAYERINFO,
	NETOBJTYPE_CLIENTINFO,
	NETOBJTYPE_SPECTATORINFO,
	NETEVENTTYPE_COMMON,
	NETEVENTTYPE_AMMOFILL,
	NETEVENTTYPE_REPAIR,
	NETEVENTTYPE_BUILDINGHIT,
	NETEVENTTYPE_FLAMEHIT,
	NETEVENTTYPE_EXPLOSION,
	NETEVENTTYPE_FLAMEEXPLOSION,
	NETEVENTTYPE_SPAWN,
	NETEVENTTYPE_HAMMERHIT,
	NETEVENTTYPE_FX,
	NETEVENTTYPE_LAZER,
	NETEVENTTYPE_SWORDTRACER,
	NETEVENTTYPE_DEATH,
	NETEVENTTYPE_SOUNDGLOBAL,
	NETEVENTTYPE_SOUNDWORLD,
	NETEVENTTYPE_BLOCK,
	NETEVENTTYPE_DAMAGEIND,
	NETEVENTTYPE_EFFECT,
	NUM_NETOBJTYPES
};

enum
{
	NETMSG_INVALID=0,
	NETMSGTYPE_SV_BROADCAST,
	NETMSGTYPE_SV_GAMEVOTE,
	NETMSGTYPE_SV_CHAT,
	NETMSGTYPE_SV_KILLMSG,
	NETMSGTYPE_SV_SOUNDGLOBAL,
	NETMSGTYPE_SV_TUNEPARAMS,
	NETMSGTYPE_SV_EXTRAPROJECTILE,
	NETMSGTYPE_SV_READYTOENTER,
	NETMSGTYPE_SV_WEAPONPICKUP,
	NETMSGTYPE_SV_EMOTICON,
	NETMSGTYPE_SV_VOTECLEAROPTIONS,
	NETMSGTYPE_SV_VOTEOPTIONLISTADD,
	NETMSGTYPE_SV_VOTEOPTIONADD,
	NETMSGTYPE_SV_VOTEOPTIONREMOVE,
	NETMSGTYPE_SV_VOTESET,
	NETMSGTYPE_SV_VOTESTATUS,
	NETMSGTYPE_SV_INVENTORY,
	NETMSGTYPE_CL_SAY,
	NETMSGTYPE_CL_SETTEAM,
	NETMSGTYPE_CL_SETSPECTATORMODE,
	NETMSGTYPE_CL_STARTINFO,
	NETMSGTYPE_CL_CHANGEINFO,
	NETMSGTYPE_CL_KILL,
	NETMSGTYPE_CL_EMOTICON,
	NETMSGTYPE_CL_DROPWEAPON,
	NETMSGTYPE_CL_SELECTITEM,
	NETMSGTYPE_CL_USEKIT,
	NETMSGTYPE_CL_VOTE,
	NETMSGTYPE_CL_VOTEGAMEMODE,
	NETMSGTYPE_CL_CALLVOTE,
	NETMSGTYPE_CL_INVENTORYACTION,
	NUM_NETMSGTYPES
};

struct CNetObj_PlayerInput
{
	int m_Direction;
	int m_TargetX;
	int m_TargetY;
	int m_Jump;
	int m_Fire;
	int m_Hook;
	int m_Charge;
	int m_Down;
	int m_PlayerFlags;
	int m_WantedWeapon;
	int m_NextWeapon;
	int m_PrevWeapon;
};

struct CNetObj_Projectile
{
	int m_X;
	int m_Y;
	int m_VelX;
	int m_VelY;
	int m_Vel2X;
	int m_Vel2Y;
	int m_Type;
	int m_StartTick;
};

struct CNetObj_Laser
{
	int m_X;
	int m_Y;
	int m_FromX;
	int m_FromY;
	int m_Charge;
	int m_StartTick;
};

struct CNetObj_LaserFail
{
	int m_X;
	int m_Y;
	int m_FromX;
	int m_FromY;
	int m_PowerLevel;
	int m_StartTick;
};

struct CNetObj_Pickup
{
	int m_X;
	int m_Y;
	int m_Mirror;
	int m_Angle;
	int m_Type;
	int m_Subtype;
};

struct CNetObj_Weapon
{
	int m_X;
	int m_Y;
	int m_WeaponType;
	int m_AttackTick;
	int m_Angle;
};

struct CNetObj_Droid
{
	int m_X;
	int m_Y;
	int m_Angle;
	int m_AttackTick;
	int m_Type;
	int m_Status;
	int m_Anim;
	int m_Dir;
};

struct CNetObj_Building
{
	int m_X;
	int m_Y;
	int m_Status;
	int m_Type;
	int m_Team;
};

struct CNetObj_Block
{
	int m_X;
	int m_Y;
	int m_Type;
};

struct CNetObj_Turret : public CNetObj_Building
{
	int m_Angle;
	int m_Weapon;
	int m_AttackTick;
};

struct CNetObj_Powerupper : public CNetObj_Building
{
	int m_Item;
};

struct CNetObj_Shop : public CNetObj_Building
{
	int m_Item1;
	int m_Item2;
	int m_Item3;
	int m_Item4;
};

struct CNetObj_Flag
{
	int m_X;
	int m_Y;
	int m_Team;
};

struct CNetObj_Radar
{
	int m_TargetX;
	int m_TargetY;
	int m_Type;
};

struct CNetObj_GameInfo
{
	int m_GameFlags;
	int m_GameStateFlags;
	int m_RoundStartTick;
	int m_WarmupTimer;
	int m_ScoreLimit;
	int m_TimeLimit;
	int m_RoundNum;
	int m_RoundCurrent;
};

struct CNetObj_GameData
{
	int m_TeamscoreRed;
	int m_TeamscoreBlue;
	int m_FlagCarrierRed;
	int m_FlagCarrierBlue;
};

struct CNetObj_BallCore
{
	int m_Tick;
	int m_X;
	int m_Y;
	int m_VelX;
	int m_VelY;
	int m_Angle;
	int m_AngleForce;
	int m_Status;
};

struct CNetObj_Ball : public CNetObj_BallCore
{
};

struct CNetObj_CharacterCore
{
	int m_Tick;
	int m_X;
	int m_Y;
	int m_VelX;
	int m_VelY;
	int m_Movement1;
	int m_Health;
	int m_HookedPlayer;
	int m_HookState;
	int m_HookTick;
	int m_HookX;
	int m_HookY;
	int m_HookDx;
	int m_HookDy;
	int m_Angle;
	int m_Direction;
	int m_Down;
	int m_Anim;
	int m_LockDirection;
	int m_HandJetpack;
	int m_Jetpack;
	int m_JetpackPower;
	int m_Wallrun;
	int m_Roll;
	int m_Slide;
	int m_JumpTimer;
	int m_Charge;
	int m_ChargeLevel;
	int m_Status;
	int m_DamageTick;
	int m_Action;
	int m_ActionState;
	int m_Jumped;
	int m_Sliding;
	int m_Grounded;
	int m_Slope;
};

struct CNetObj_Character : public CNetObj_CharacterCore
{
	int m_PlayerFlags;
	int m_Armor;
	int m_AmmoCount;
	int m_Weapon;
	int m_Emote;
	int m_AttackTick;
	int m_Movement;
};

struct CNetObj_PlayerInfo
{
	int m_Local;
	int m_ClientID;
	int m_Team;
	int m_Spectating;
	int m_Score;
	int m_Latency;
	int m_WeaponSlot;
	int m_Weapon1;
	int m_Weapon2;
	int m_Weapon3;
	int m_Weapon4;
	int m_Kits;
};

struct CNetObj_ClientInfo
{
	int m_Name0;
	int m_Name1;
	int m_Name2;
	int m_Name3;
	int m_Clan0;
	int m_Clan1;
	int m_Clan2;
	int m_Country;
	int m_Topper0;
	int m_Topper1;
	int m_Topper2;
	int m_Topper3;
	int m_Topper4;
	int m_Topper5;
	int m_Eye0;
	int m_Eye1;
	int m_Eye2;
	int m_Eye3;
	int m_Eye4;
	int m_Eye5;
	int m_Head0;
	int m_Head1;
	int m_Head2;
	int m_Head3;
	int m_Head4;
	int m_Head5;
	int m_Body0;
	int m_Body1;
	int m_Body2;
	int m_Body3;
	int m_Body4;
	int m_Body5;
	int m_Hand0;
	int m_Hand1;
	int m_Hand2;
	int m_Hand3;
	int m_Hand4;
	int m_Hand5;
	int m_Foot0;
	int m_Foot1;
	int m_Foot2;
	int m_Foot3;
	int m_Foot4;
	int m_Foot5;
	int m_ColorBody;
	int m_ColorFeet;
	int m_ColorTopper;
	int m_ColorSkin;
	int m_IsBot;
	int m_BloodColor;
};

struct CNetObj_SpectatorInfo
{
	int m_SpectatorID;
	int m_X;
	int m_Y;
};

struct CNetEvent_Common
{
	int m_X;
	int m_Y;
};

struct CNetEvent_AmmoFill : public CNetEvent_Common
{
	int m_Weapon;
};

struct CNetEvent_Repair : public CNetEvent_Common
{
};

struct CNetEvent_BuildingHit : public CNetEvent_Common
{
};

struct CNetEvent_FlameHit : public CNetEvent_Common
{
};

struct CNetEvent_Explosion : public CNetEvent_Common
{
	int m_Weapon;
};

struct CNetEvent_FlameExplosion : public CNetEvent_Common
{
};

struct CNetEvent_Spawn : public CNetEvent_Common
{
};

struct CNetEvent_HammerHit : public CNetEvent_Common
{
};

struct CNetEvent_FX : public CNetEvent_Common
{
	int m_FX;
};

struct CNetEvent_Lazer : public CNetEvent_Common
{
	int m_Height;
};

struct CNetEvent_Swordtracer : public CNetEvent_Common
{
	int m_Angle;
};

struct CNetEvent_Death : public CNetEvent_Common
{
	int m_ClientID;
};

struct CNetEvent_SoundGlobal : public CNetEvent_Common
{
	int m_SoundID;
};

struct CNetEvent_SoundWorld : public CNetEvent_Common
{
	int m_SoundID;
};

struct CNetEvent_Block : public CNetEvent_Common
{
	int m_Type;
};

struct CNetEvent_DamageInd : public CNetEvent_Common
{
	int m_Angle;
	int m_Damage;
	int m_ClientID;
};

struct CNetEvent_Effect : public CNetEvent_Common
{
	int m_EffectID;
	int m_ClientID;
};

struct CNetMsg_Sv_Broadcast
{
	const char *m_pMessage;
	int MsgID() const { return NETMSGTYPE_SV_BROADCAST; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_pMessage, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_GameVote
{
	const char *m_pName;
	const char *m_pDescription;
	const char *m_pImage;
	const char *m_pPlayers;
	int m_Index;
	int m_TimeLeft;
	int MsgID() const { return NETMSGTYPE_SV_GAMEVOTE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_pName, -1);
		pPacker->AddString(m_pDescription, -1);
		pPacker->AddString(m_pImage, -1);
		pPacker->AddString(m_pPlayers, -1);
		pPacker->AddInt(m_Index);
		pPacker->AddInt(m_TimeLeft);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_Chat
{
	int m_Team;
	int m_ClientID;
	const char *m_pMessage;
	int MsgID() const { return NETMSGTYPE_SV_CHAT; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Team);
		pPacker->AddInt(m_ClientID);
		pPacker->AddString(m_pMessage, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_KillMsg
{
	int m_Killer;
	int m_Victim;
	int m_Weapon;
	int m_ModeSpecial;
	int MsgID() const { return NETMSGTYPE_SV_KILLMSG; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Killer);
		pPacker->AddInt(m_Victim);
		pPacker->AddInt(m_Weapon);
		pPacker->AddInt(m_ModeSpecial);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_SoundGlobal
{
	int m_SoundID;
	int MsgID() const { return NETMSGTYPE_SV_SOUNDGLOBAL; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_SoundID);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_TuneParams
{
	int MsgID() const { return NETMSGTYPE_SV_TUNEPARAMS; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_ExtraProjectile
{
	int MsgID() const { return NETMSGTYPE_SV_EXTRAPROJECTILE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_ReadyToEnter
{
	int MsgID() const { return NETMSGTYPE_SV_READYTOENTER; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_WeaponPickup
{
	int m_Weapon;
	int MsgID() const { return NETMSGTYPE_SV_WEAPONPICKUP; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Weapon);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_Emoticon
{
	int m_ClientID;
	int m_Emoticon;
	int MsgID() const { return NETMSGTYPE_SV_EMOTICON; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_ClientID);
		pPacker->AddInt(m_Emoticon);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_VoteClearOptions
{
	int MsgID() const { return NETMSGTYPE_SV_VOTECLEAROPTIONS; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_VoteOptionListAdd
{
	int m_NumOptions;
	const char *m_pDescription0;
	const char *m_pDescription1;
	const char *m_pDescription2;
	const char *m_pDescription3;
	const char *m_pDescription4;
	const char *m_pDescription5;
	const char *m_pDescription6;
	const char *m_pDescription7;
	const char *m_pDescription8;
	const char *m_pDescription9;
	const char *m_pDescription10;
	const char *m_pDescription11;
	const char *m_pDescription12;
	const char *m_pDescription13;
	const char *m_pDescription14;
	int MsgID() const { return NETMSGTYPE_SV_VOTEOPTIONLISTADD; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_NumOptions);
		pPacker->AddString(m_pDescription0, -1);
		pPacker->AddString(m_pDescription1, -1);
		pPacker->AddString(m_pDescription2, -1);
		pPacker->AddString(m_pDescription3, -1);
		pPacker->AddString(m_pDescription4, -1);
		pPacker->AddString(m_pDescription5, -1);
		pPacker->AddString(m_pDescription6, -1);
		pPacker->AddString(m_pDescription7, -1);
		pPacker->AddString(m_pDescription8, -1);
		pPacker->AddString(m_pDescription9, -1);
		pPacker->AddString(m_pDescription10, -1);
		pPacker->AddString(m_pDescription11, -1);
		pPacker->AddString(m_pDescription12, -1);
		pPacker->AddString(m_pDescription13, -1);
		pPacker->AddString(m_pDescription14, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_VoteOptionAdd
{
	const char *m_pDescription;
	int MsgID() const { return NETMSGTYPE_SV_VOTEOPTIONADD; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_pDescription, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_VoteOptionRemove
{
	const char *m_pDescription;
	int MsgID() const { return NETMSGTYPE_SV_VOTEOPTIONREMOVE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_pDescription, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_VoteSet
{
	int m_Timeout;
	const char *m_pDescription;
	const char *m_pReason;
	int MsgID() const { return NETMSGTYPE_SV_VOTESET; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Timeout);
		pPacker->AddString(m_pDescription, -1);
		pPacker->AddString(m_pReason, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_VoteStatus
{
	int m_Type;
	int m_Yes;
	int m_No;
	int m_Pass;
	int m_Total;
	int m_Option5;
	int m_Option6;
	int MsgID() const { return NETMSGTYPE_SV_VOTESTATUS; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Type);
		pPacker->AddInt(m_Yes);
		pPacker->AddInt(m_No);
		pPacker->AddInt(m_Pass);
		pPacker->AddInt(m_Total);
		pPacker->AddInt(m_Option5);
		pPacker->AddInt(m_Option6);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Sv_Inventory
{
	int m_Item1;
	int m_Item2;
	int m_Item3;
	int m_Item4;
	int m_Item5;
	int m_Item6;
	int m_Item7;
	int m_Item8;
	int m_Item9;
	int m_Item10;
	int m_Item11;
	int m_Item12;
	int m_Gold;
	int MsgID() const { return NETMSGTYPE_SV_INVENTORY; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Item1);
		pPacker->AddInt(m_Item2);
		pPacker->AddInt(m_Item3);
		pPacker->AddInt(m_Item4);
		pPacker->AddInt(m_Item5);
		pPacker->AddInt(m_Item6);
		pPacker->AddInt(m_Item7);
		pPacker->AddInt(m_Item8);
		pPacker->AddInt(m_Item9);
		pPacker->AddInt(m_Item10);
		pPacker->AddInt(m_Item11);
		pPacker->AddInt(m_Item12);
		pPacker->AddInt(m_Gold);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_Say
{
	int m_Team;
	const char *m_pMessage;
	int MsgID() const { return NETMSGTYPE_CL_SAY; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Team);
		pPacker->AddString(m_pMessage, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_SetTeam
{
	int m_Team;
	int MsgID() const { return NETMSGTYPE_CL_SETTEAM; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Team);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_SetSpectatorMode
{
	int m_SpectatorID;
	int MsgID() const { return NETMSGTYPE_CL_SETSPECTATORMODE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_SpectatorID);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_StartInfo
{
	const char *m_pName;
	const char *m_pClan;
	int m_Country;
	const char *m_pTopper;
	const char *m_pEye;
	const char *m_pHead;
	const char *m_pBody;
	const char *m_pHand;
	const char *m_pFoot;
	int m_ColorBody;
	int m_ColorFeet;
	int m_ColorTopper;
	int m_ColorSkin;
	int m_BloodColor;
	int m_IsBot;
	int MsgID() const { return NETMSGTYPE_CL_STARTINFO; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_pName, -1);
		pPacker->AddString(m_pClan, -1);
		pPacker->AddInt(m_Country);
		pPacker->AddString(m_pTopper, -1);
		pPacker->AddString(m_pEye, -1);
		pPacker->AddString(m_pHead, -1);
		pPacker->AddString(m_pBody, -1);
		pPacker->AddString(m_pHand, -1);
		pPacker->AddString(m_pFoot, -1);
		pPacker->AddInt(m_ColorBody);
		pPacker->AddInt(m_ColorFeet);
		pPacker->AddInt(m_ColorTopper);
		pPacker->AddInt(m_ColorSkin);
		pPacker->AddInt(m_BloodColor);
		pPacker->AddInt(m_IsBot);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_ChangeInfo
{
	const char *m_pName;
	const char *m_pClan;
	int m_Country;
	const char *m_pTopper;
	const char *m_pEye;
	const char *m_pHead;
	const char *m_pBody;
	const char *m_pHand;
	const char *m_pFoot;
	int m_ColorBody;
	int m_ColorFeet;
	int m_ColorTopper;
	int m_ColorSkin;
	int m_BloodColor;
	int m_IsBot;
	int MsgID() const { return NETMSGTYPE_CL_CHANGEINFO; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_pName, -1);
		pPacker->AddString(m_pClan, -1);
		pPacker->AddInt(m_Country);
		pPacker->AddString(m_pTopper, -1);
		pPacker->AddString(m_pEye, -1);
		pPacker->AddString(m_pHead, -1);
		pPacker->AddString(m_pBody, -1);
		pPacker->AddString(m_pHand, -1);
		pPacker->AddString(m_pFoot, -1);
		pPacker->AddInt(m_ColorBody);
		pPacker->AddInt(m_ColorFeet);
		pPacker->AddInt(m_ColorTopper);
		pPacker->AddInt(m_ColorSkin);
		pPacker->AddInt(m_BloodColor);
		pPacker->AddInt(m_IsBot);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_Kill
{
	int MsgID() const { return NETMSGTYPE_CL_KILL; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_Emoticon
{
	int m_Emoticon;
	int MsgID() const { return NETMSGTYPE_CL_EMOTICON; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Emoticon);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_DropWeapon
{
	int MsgID() const { return NETMSGTYPE_CL_DROPWEAPON; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_SelectItem
{
	int m_Item;
	int MsgID() const { return NETMSGTYPE_CL_SELECTITEM; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Item);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_UseKit
{
	int m_Kit;
	int m_X;
	int m_Y;
	int MsgID() const { return NETMSGTYPE_CL_USEKIT; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Kit);
		pPacker->AddInt(m_X);
		pPacker->AddInt(m_Y);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_Vote
{
	int m_Vote;
	int MsgID() const { return NETMSGTYPE_CL_VOTE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Vote);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_VoteGameMode
{
	int m_Vote;
	int MsgID() const { return NETMSGTYPE_CL_VOTEGAMEMODE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Vote);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_CallVote
{
	const char *m_Type;
	const char *m_Value;
	const char *m_Reason;
	int MsgID() const { return NETMSGTYPE_CL_CALLVOTE; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddString(m_Type, -1);
		pPacker->AddString(m_Value, -1);
		pPacker->AddString(m_Reason, -1);
		return pPacker->Error() != 0;
	}
};

struct CNetMsg_Cl_InventoryAction
{
	int m_Type;
	int m_Item1;
	int m_Item2;
	int m_Slot;
	int MsgID() const { return NETMSGTYPE_CL_INVENTORYACTION; }
	
	bool Pack(CMsgPacker *pPacker)
	{
		pPacker->AddInt(m_Type);
		pPacker->AddInt(m_Item1);
		pPacker->AddInt(m_Item2);
		pPacker->AddInt(m_Slot);
		return pPacker->Error() != 0;
	}
};

enum
{
	SOUND_GUN_FIRE=0,
	SOUND_SHOTGUN_FIRE,
	SOUND_ELECTRO_FIRE,
	SOUND_SCYTHE_FIRE,
	SOUND_GRENADE_FIRE,
	SOUND_TOOL_FIRE,
	SOUND_TOOL_HIT,
	SOUND_HAMMER_FIRE,
	SOUND_HAMMER_HIT,
	SOUND_CHAINSAW_IDLE,
	SOUND_CHAINSAW_FIRE,
	SOUND_GRENADE_EXPLODE,
	SOUND_RIFLE_FIRE,
	SOUND_LASER_FIRE,
	SOUND_LASER_BOUNCE,
	SOUND_WEAPON_SWITCH,
	SOUND_WALKER_FIRE,
	SOUND_DEATH,
	SOUND_KILL,
	SOUND_PLAYER_PAIN_SHORT,
	SOUND_PLAYER_PAIN_LONG,
	SOUND_BODY_LAND,
	SOUND_PLAYER_AIRJUMP,
	SOUND_PLAYER_JUMP,
	SOUND_PLAYER_DIE,
	SOUND_PLAYER_SPAWN,
	SOUND_PLAYER_SKID,
	SOUND_TEE_CRY,
	SOUND_CHARGE_UP,
	SOUND_CHARGE_FULL,
	SOUND_CHARGE_DOWN,
	SOUND_HOOK_LOOP,
	SOUND_HOOK_ATTACH_GROUND,
	SOUND_HOOK_ATTACH_PLAYER,
	SOUND_HOOK_NOATTACH,
	SOUND_PICKUP_HEALTH,
	SOUND_PICKUP_ARMOR,
	SOUND_PICKUP_GRENADE,
	SOUND_PICKUP_SHOTGUN,
	SOUND_WEAPON_SPAWN,
	SOUND_WEAPON_NOAMMO,
	SOUND_HIT,
	SOUND_KICK,
	SOUND_KICKHIT,
	SOUND_UI_PICK,
	SOUND_UI_NEGATIVE,
	SOUND_UI_POSITIVE,
	SOUND_CHAT_SERVER,
	SOUND_CHAT_CLIENT,
	SOUND_CHAT_HIGHLIGHT,
	SOUND_CTF_DROP,
	SOUND_CTF_RETURN,
	SOUND_CTF_GRAB_PL,
	SOUND_CTF_GRAB_EN,
	SOUND_CTF_CAPTURE,
	SOUND_METAL_HIT,
	SOUND_BALL_BOUNCE,
	SOUND_DASH,
	SOUND_JETPACK,
	SOUND_JETPACK1,
	SOUND_JETPACK2,
	SOUND_JETPACK3,
	SOUND_JETPACK4,
	SOUND_TURBO1,
	SOUND_TURBO2,
	SOUND_TURBO3,
	SOUND_TURBO4,
	SOUND_FLAMER1,
	SOUND_FLAMER2,
	SOUND_DEATHRAY,
	SOUND_ELECTRODEATH,
	SOUND_SPAWN,
	SOUND_SHIELD_HIT,
	SOUND_ITEM_INVISIBILITY,
	SOUND_ITEM_SHIELD,
	SOUND_ITEM_HEAL,
	SOUND_ITEM_RAGE,
	SOUND_ITEM_FUEL,
	SOUND_ELECTROMINE,
	SOUND_NEGATIVE,
	SOUND_BUILD_TURRET,
	SOUND_FIRETRAP,
	SOUND_JUMPPAD,
	SOUND_TESLACOIL_FIRE,
	SOUND_DOOR1,
	SOUND_WALKER_TAKEOFF,
	SOUND_GRENADE2_EXPLODE,
	SOUND_BUILD,
	SOUND_STAR_FIRE,
	SOUND_GREEN_EXPLOSION,
	SOUND_GAMEOVER,
	SOUND_MENU,
	SOUND_BG1,
	SOUND_BG2,
	SOUND_BG3,
	SOUND_BG4,
	SOUND_BG5,
	SOUND_BG6,
	SOUND_BG7,
	SOUND_BG8,
	SOUND_BG9,
	SOUND_BG10,
	SOUND_BG11,
	SOUND_BG12,
	SOUND_BG13,
	SOUND_RIFLE1_FIRE,
	SOUND_BASE1_FIRE,
	SOUND_BASE2_FIRE,
	SOUND_BASE3_FIRE,
	SOUND_BASE4_FIRE,
	SOUND_BARREL1_FIRE,
	SOUND_BARREL2_FIRE,
	SOUND_BARREL3_FIRE,
	SOUND_BARREL4_FIRE,
	SOUND_BAZOOKA_FIRE,
	SOUND_BOUNCER_FIRE,
	SOUND_BOUNCER_BOUNCE,
	SOUND_BOUNCER_EXPLODE,
	SOUND_SFX_BOUNCE1,
	SOUND_UPGRADE,
	SOUND_BOMB_BEEP,
	SOUND_BOMB_DENIED,
	SOUND_INV1,
	SOUND_INV2,
	SOUND_INV3,
	SOUND_INV4,
	SOUND_GUI_DENIED1,
	SOUND_WEAPON_CHARGE1_1,
	SOUND_WEAPON_CHARGE1_2,
	SOUND_WEAPON_CHARGE1_3,
	SOUND_WEAPON_CHARGE1_4,
	SOUND_WEAPON_CHARGE1_5,
	SOUND_WEAPON_CHARGE1_6,
	SOUND_WEAPON_CHARGE1_7,
	SOUND_WEAPON_CHARGE1_8,
	SOUND_WEAPON_CHARGE1_9,
	NUM_SOUNDS
};
enum
{
	WEAPON_TOOL=0,
	WEAPON_HAMMER,
	WEAPON_SHOTGUN,
	WEAPON_RIFLE,
	WEAPON_LASER,
	WEAPON_ELECTRIC,
	WEAPON_GRENADE,
	WEAPON_FLAMER,
	WEAPON_CHAINSAW,
	WEAPON_SCYTHE,
	NUM_WEAPONS
};


class CNetObjHandler
{
	const char *m_pMsgFailedOn;
	const char *m_pObjCorrectedOn;
	char m_aMsgData[1024];
	int m_NumObjCorrections;
	int ClampInt(const char *pErrorMsg, int Value, int Min, int Max);

	static const char *ms_apObjNames[];
	static int ms_aObjSizes[];
	static const char *ms_apMsgNames[];

public:
	CNetObjHandler();

	int ValidateObj(int Type, void *pData, int Size);
	const char *GetObjName(int Type);
	int GetObjSize(int Type);
	int NumObjCorrections();
	const char *CorrectedObjOn();

	const char *GetMsgName(int Type);
	void *SecureUnpackMsg(int Type, CUnpacker *pUnpacker);
	const char *FailedMsgOn();
};


#endif // GAME_GENERATED_PROTOCOL_H
