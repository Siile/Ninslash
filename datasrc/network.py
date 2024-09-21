from datatypes import *

Emotes = ["NORMAL", "PAIN", "HAPPY", "SURPRISE", "ANGRY", "BLINK"]
PlayerFlags = ["PLAYING", "IN_MENU", "CHATTING", "SCOREBOARD"]
GameFlags = ["TEAMS", "INFECTION", "COOP", "SURVIVAL", "BUILD", "FLAGS", "ACID"]
GameStateFlags = ["GAMEOVER", "SUDDENDEATH", "PAUSED"]

Emoticons = ["OOP", "EXCLAMATION", "HEARTS", "DROP", "DOTDOT", "MUSIC", "SORRY", "GHOST", "SUSHI", "SPLATTEE", "DEVILTEE", "ZOMG", "ZZZ", "WTF", "EYES", "QUESTION"]

Powerups = ["HEALTH", "AMMO", "WEAPON", "ARMOR", "COIN", "KIT"]

# keep masks at the end
Statuses = ["SPAWNING", "AFLAME", "SLOWED", "ELECTRIC", "DEATHRAY", "SHIELD", "DASH", "INVISIBILITY", "SLOWMOVING", "BOMBCARRIER", "MASK1", "MASK2", "MASK3"]

BallStatuses = ["STATIONARY", "SUPER"]

Damagetypes = ["NORMAL", "FLAME", "ELECTRIC", "FLUID"]

Droidstatus = ["IDLE", "HURT", "ELECTRIC", "TERMINATED"]
Droidtype = ["WALKER", "STAR", "CRAWLER", "BOSSCRAWLER", "FLY"]
Droidanim = ["IDLE", "MOVE", "ATTACK", "JUMPATTACK"]

CoreAction = ["IDLE", "JUMP", "WALLJUMP", "ROLL", "SLIDE", "SLIDEKICK", "FALL", "JUMPPAD", "HANG"]

InventoryAction = ["SWAP", "COMBINE", "TAKEPART", "DROP", "SHOP", "ROLL"]

Radar = ["CHARACTER", "HUMAN", "ENEMY", "DOOR", "REACTOR", "BOMB"]

RawHeader = '''

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
'''

RawSource = '''
#include <engine/message.h>
#include "protocol.h"
'''

Enums = [
	Enum("EMOTE", Emotes),
	Enum("POWERUP", Powerups),
	Enum("EMOTICON", Emoticons),
	Enum("STATUS", Statuses),
	Enum("BALLSTATUS", BallStatuses),
	Enum("DAMAGETYPE", Damagetypes),
	Enum("DROIDSTATUS", Droidstatus),
	Enum("DROIDTYPE", Droidtype),
	Enum("DROIDANIM", Droidanim),
	Enum("COREACTION", CoreAction),
	Enum("INVENTORYACTION", InventoryAction),
	Enum("RADAR", Radar)
]

Flags = [
	Flags("PLAYERFLAG", PlayerFlags),
	Flags("GAMEFLAG", GameFlags),
	Flags("GAMESTATEFLAG", GameStateFlags)
]

Objects = [

	NetObject("PlayerInput", [
		NetIntAny("m_Direction"),
		NetIntAny("m_TargetX"),
		NetIntAny("m_TargetY"),

		NetIntAny("m_Jump"),
		NetIntAny("m_Fire"),
		NetIntAny("m_Hook"),
		NetIntAny("m_Charge"),
		NetIntAny("m_Down"),

		NetIntRange("m_PlayerFlags", 0, 256),

		NetIntAny("m_WantedWeapon"),
		NetIntAny("m_NextWeapon"),
		NetIntAny("m_PrevWeapon"),
	]),

	NetObject("Projectile", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_VelX"),
		NetIntAny("m_VelY"),
		NetIntAny("m_Vel2X"),
		NetIntAny("m_Vel2Y"),

		NetIntAny("m_Type"),
		NetTick("m_StartTick"),
	]),

	NetObject("Laser", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_FromX"),
		NetIntAny("m_FromY"),
		NetIntAny("m_Charge"),

		NetTick("m_StartTick"),
	]),
	
	NetObject("LaserFail", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_FromX"),
		NetIntAny("m_FromY"),
		NetIntRange("m_PowerLevel", 0, 100),

		NetTick("m_StartTick"),
	]),

	NetObject("Pickup", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetBool("m_Mirror"),
		NetIntAny("m_Angle"),
		NetIntRange("m_Type", 0, 'max_int'),
		NetIntAny("m_Subtype"),
	]),
	
	NetObject("Weapon", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_WeaponType"),
		NetIntAny("m_AttackTick"),
		NetIntAny("m_Angle")
	]),
	
	NetObject("Droid", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_Angle"),
		NetIntRange("m_AttackTick", 0, 'max_int'),

		NetIntRange("m_Type", 0, 16),
		NetIntRange("m_Status", 0, 16),
		NetIntRange("m_Anim", 0, 8),
		NetIntRange("m_Dir", -1, 1),
	]),

	NetObject("Building", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntRange("m_Status", 0, 'max_int'),
		NetIntRange("m_Type", 0, 'max_int'),
		NetIntAny("m_Team")
	]),
	
	NetObject("Block", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_Type")
	]),
	
	NetObject("Turret:Building", [
		NetIntAny("m_Angle"),
		NetIntAny("m_Weapon"),
		NetIntRange("m_AttackTick", 0, 'max_int')
	]),
	
	NetObject("Powerupper:Building", [
		NetIntRange("m_Item", -1, 9)
	]),
	
	NetObject("Shop:Building", [
		NetIntAny("m_Item1"),
		NetIntAny("m_Item2"),
		NetIntAny("m_Item3"),
		NetIntAny("m_Item4")
	]),

	NetObject("Flag", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),

		NetIntRange("m_Team", 'TEAM_RED', 'TEAM_BLUE')
	]),
	
	NetObject("Radar", [
		NetIntAny("m_TargetX"),
		NetIntAny("m_TargetY"),
		NetIntAny("m_Type")
	]),

	NetObject("GameInfo", [
		NetIntRange("m_GameFlags", 0, 256),
		NetIntRange("m_GameStateFlags", 0, 256),
		NetTick("m_RoundStartTick"),
		NetIntRange("m_WarmupTimer", 0, 'max_int'),

		NetIntRange("m_ScoreLimit", 0, 'max_int'),
		NetIntRange("m_TimeLimit", 0, 'max_int'),

		NetIntRange("m_RoundNum", 0, 'max_int'),
		NetIntRange("m_RoundCurrent", 0, 'max_int'),
	]),

	NetObject("GameData", [
		NetIntAny("m_TeamscoreRed"),
		NetIntAny("m_TeamscoreBlue"),

		NetIntRange("m_FlagCarrierRed", 'FLAG_MISSING', 'MAX_CLIENTS-1'),
		NetIntRange("m_FlagCarrierBlue", 'FLAG_MISSING', 'MAX_CLIENTS-1'),
	]),

	NetObject("BallCore", [
		NetIntAny("m_Tick"),
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_VelX"),
		NetIntAny("m_VelY"),
		NetIntAny("m_Angle"),
		NetIntAny("m_AngleForce"),
		NetIntAny("m_Status"),
	]),
	
	NetObject("Ball:BallCore", [
	]),
	
	NetObject("CharacterCore", [
		NetIntAny("m_Tick"),
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
		NetIntAny("m_VelX"),
		NetIntAny("m_VelY"),
		NetIntAny("m_Movement1"),
		
		NetIntRange("m_Health", 0, 100),
		NetIntRange("m_HookedPlayer", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_HookState", -1, 6),
		NetTick("m_HookTick"),

		NetIntAny("m_HookX"),
		NetIntAny("m_HookY"),
		NetIntAny("m_HookDx"),
		NetIntAny("m_HookDy"),

		NetIntAny("m_Angle"),
		NetIntRange("m_Direction", -1, 1),
		NetIntRange("m_Down", 0, 1),
		NetIntAny("m_Anim"),
		NetIntAny("m_LockDirection"),
		NetIntRange("m_HandJetpack", 0, 1),
		NetIntRange("m_Jetpack", 0, 1),
		NetIntRange("m_JetpackPower", 0, 200),
		NetIntRange("m_Wallrun", -100, 100),
		NetIntRange("m_Roll", 0, 32),
		NetIntRange("m_Slide", -10, 32),
		
		NetIntRange("m_JumpTimer", -10, 10),

		NetIntRange("m_Charge", 0, 1),
		NetIntRange("m_ChargeLevel", -50, 100),
		
		NetIntAny("m_Status"),
		NetIntAny("m_DamageTick"),
		
		NetIntRange("m_Action", 0, 64),
		NetIntAny("m_ActionState"),
		
		NetIntRange("m_Jumped", 0, 3),
		
		NetBool("m_Sliding"),
		NetBool("m_Grounded"),
		NetIntRange("m_Slope", -1, 1),
	]),

	NetObject("Character:CharacterCore", [
		NetIntRange("m_PlayerFlags", 0, 256),
		NetIntRange("m_Armor", 0, 100),
		NetIntRange("m_AmmoCount", 0, 30),
		NetIntAny("m_Weapon"),
		NetIntRange("m_Emote", 0, len(Emotes)),
		NetIntRange("m_AttackTick", 0, 'max_int'),
		NetIntAny("m_Movement"),
	]),

	NetObject("PlayerInfo", [
		NetIntRange("m_Local", 0, 1),
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
		NetIntRange("m_Spectating", 0, 1),

		NetIntAny("m_Score"),
		NetIntAny("m_Latency"),
		
		NetIntRange("m_WeaponSlot", 0, 3),
		NetIntAny("m_Weapon1"),
		NetIntAny("m_Weapon2"),
		NetIntAny("m_Weapon3"),
		NetIntAny("m_Weapon4"),
		
		NetIntRange("m_Kits", 0, 99),
	]),

	NetObject("ClientInfo", [
		# 4*4 = 16 charachters
		NetIntAny("m_Name0"), NetIntAny("m_Name1"), NetIntAny("m_Name2"),
		NetIntAny("m_Name3"),

		# 4*3 = 12 charachters
		NetIntAny("m_Clan0"), NetIntAny("m_Clan1"), NetIntAny("m_Clan2"),

		NetIntAny("m_Country"),
		
		# 4*6 = 24 charachters
		NetIntAny("m_Topper0"), NetIntAny("m_Topper1"), NetIntAny("m_Topper2"),
		NetIntAny("m_Topper3"), NetIntAny("m_Topper4"), NetIntAny("m_Topper5"),
		
		# 4*6 = 24 charachters
		NetIntAny("m_Eye0"), NetIntAny("m_Eye1"), NetIntAny("m_Eye2"),
		NetIntAny("m_Eye3"), NetIntAny("m_Eye4"), NetIntAny("m_Eye5"),
		
		# 4*6 = 24 charachters
		NetIntAny("m_Head0"), NetIntAny("m_Head1"), NetIntAny("m_Head2"),
		NetIntAny("m_Head3"), NetIntAny("m_Head4"), NetIntAny("m_Head5"),
		
		# 4*6 = 24 charachters
		NetIntAny("m_Body0"), NetIntAny("m_Body1"), NetIntAny("m_Body2"),
		NetIntAny("m_Body3"), NetIntAny("m_Body4"), NetIntAny("m_Body5"),
		
		# 4*6 = 24 charachters
		NetIntAny("m_Hand0"), NetIntAny("m_Hand1"), NetIntAny("m_Hand2"),
		NetIntAny("m_Hand3"), NetIntAny("m_Hand4"), NetIntAny("m_Hand5"),
		
		# 4*6 = 24 charachters
		NetIntAny("m_Foot0"), NetIntAny("m_Foot1"), NetIntAny("m_Foot2"),
		NetIntAny("m_Foot3"), NetIntAny("m_Foot4"), NetIntAny("m_Foot5"),

		NetIntAny("m_ColorBody"),
		NetIntAny("m_ColorFeet"),
		NetIntAny("m_ColorTopper"),
		NetIntAny("m_ColorSkin"),
		
		NetIntRange("m_IsBot", 0, 1),
		NetIntRange("m_BloodColor", 0, 3),
	]),

	NetObject("SpectatorInfo", [
		NetIntRange("m_SpectatorID", 'SPEC_FREEVIEW', 'MAX_CLIENTS-1'),
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
	]),

	## Events

	NetEvent("Common", [
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
	]),

	# for buildings
	NetEvent("AmmoFill:Common", [
		NetIntRange("m_Weapon", 0, 'NUM_WEAPONS-1'),
	]),
	NetEvent("Repair:Common", []),
	
	NetEvent("BuildingHit:Common", []),
	NetEvent("FlameHit:Common", []),
	
	NetEvent("Explosion:Common", [
		NetIntAny("m_Weapon"),
	]),

	NetEvent("FlameExplosion:Common", []),
	NetEvent("Spawn:Common", []),
	NetEvent("HammerHit:Common", []),
	
	NetEvent("FX:Common", [
		NetIntRange("m_FX", '1', 'NUMFX-1'),
	]),
	
	NetEvent("Lazer:Common", [
		NetIntAny("m_Height"),
	]),
	
	NetEvent("Swordtracer:Common", [
		NetIntAny("m_Angle"),
	]),

	NetEvent("Death:Common", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
	]),

	NetEvent("SoundGlobal:Common", [ #TODO 0.7: remove me
		NetIntRange("m_SoundID", 0, 'NUM_SOUNDS-1'),
	]),

	NetEvent("SoundWorld:Common", [
		NetIntRange("m_SoundID", 0, 'NUM_SOUNDS-1'),
	]),

	NetEvent("Block:Common", [
		NetIntAny("m_Type")
	]),
	
	NetEvent("DamageInd:Common", [
		NetIntAny("m_Angle"),
		NetIntAny("m_Damage"),
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
	]),
	
	NetEvent("Effect:Common", [
		NetIntRange("m_EffectID", 0, 'NUM_EFFECTS-1'),
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
	]),
]

# todo: remove unnecessary ones
Messages = [

	### Server messages
	#NetMessage("Sv_Motd", [
	#	NetString("m_pMessage"),
	#]),

	NetMessage("Sv_Broadcast", [
		NetString("m_pMessage"),
	]),
	
	NetMessage("Sv_GameVote", [
		NetString("m_pName"),
		NetString("m_pDescription"),
		NetString("m_pImage"),
		NetString("m_pPlayers"),
		NetIntAny("m_Index"),
		NetIntAny("m_TimeLeft"),
	]),

	NetMessage("Sv_Chat", [
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
		NetIntRange("m_ClientID", -1, 'MAX_CLIENTS-1'),
		NetStringStrict("m_pMessage"),
	]),

	NetMessage("Sv_KillMsg", [
		NetIntRange("m_Killer", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_Victim", 0, 'MAX_CLIENTS-1'),
		NetIntAny("m_Weapon"),
		NetIntAny("m_ModeSpecial"),
	]),

	NetMessage("Sv_SoundGlobal", [
		NetIntRange("m_SoundID", 0, 'NUM_SOUNDS-1'),
	]),

	NetMessage("Sv_TuneParams", []),
	NetMessage("Sv_ExtraProjectile", []),
	NetMessage("Sv_ReadyToEnter", []),

	NetMessage("Sv_WeaponPickup", [
		NetIntAny("m_Weapon"),
	]),

	NetMessage("Sv_Emoticon", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_Emoticon", 0, 'NUM_EMOTICONS-1'),
	]),

	NetMessage("Sv_VoteClearOptions", [
	]),

	NetMessage("Sv_VoteOptionListAdd", [
		NetIntRange("m_NumOptions", 1, 15),
		NetStringStrict("m_pDescription0"), NetStringStrict("m_pDescription1"),	NetStringStrict("m_pDescription2"),
		NetStringStrict("m_pDescription3"),	NetStringStrict("m_pDescription4"),	NetStringStrict("m_pDescription5"),
		NetStringStrict("m_pDescription6"), NetStringStrict("m_pDescription7"), NetStringStrict("m_pDescription8"),
		NetStringStrict("m_pDescription9"), NetStringStrict("m_pDescription10"), NetStringStrict("m_pDescription11"),
		NetStringStrict("m_pDescription12"), NetStringStrict("m_pDescription13"), NetStringStrict("m_pDescription14"),
	]),

	NetMessage("Sv_VoteOptionAdd", [
		NetStringStrict("m_pDescription"),
	]),

	NetMessage("Sv_VoteOptionRemove", [
		NetStringStrict("m_pDescription"),
	]),

	NetMessage("Sv_VoteSet", [
		NetIntRange("m_Timeout", 0, 60),
		NetStringStrict("m_pDescription"),
		NetStringStrict("m_pReason"),
	]),

	NetMessage("Sv_VoteStatus", [
		NetIntRange("m_Type", 0, 1),
		NetIntRange("m_Yes", 0, 'MAX_CLIENTS'),
		NetIntRange("m_No", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Pass", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Total", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Option5", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Option6", 0, 'MAX_CLIENTS'),
	]),
	
	NetMessage("Sv_Inventory", [
		NetIntAny("m_Item1"),
		NetIntAny("m_Item2"),
		NetIntAny("m_Item3"),
		NetIntAny("m_Item4"),
		NetIntAny("m_Item5"),
		NetIntAny("m_Item6"),
		NetIntAny("m_Item7"),
		NetIntAny("m_Item8"),
		NetIntAny("m_Item9"),
		NetIntAny("m_Item10"),
		NetIntAny("m_Item11"),
		NetIntAny("m_Item12"),
		NetIntRange("m_Gold", 0, 999),
	]),

	### Client messages / 13
	NetMessage("Cl_Say", [
		NetBool("m_Team"),
		NetStringStrict("m_pMessage"),
	]),

	NetMessage("Cl_SetTeam", [
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
	]),

	NetMessage("Cl_SetSpectatorMode", [
		NetIntRange("m_SpectatorID", 'SPEC_FREEVIEW', 'MAX_CLIENTS-1'),
	]),

	NetMessage("Cl_StartInfo", [
		NetStringStrict("m_pName"),
		NetStringStrict("m_pClan"),
		NetIntAny("m_Country"),
		NetStringStrict("m_pTopper"),
		NetStringStrict("m_pEye"),
		NetStringStrict("m_pHead"),
		NetStringStrict("m_pBody"),
		NetStringStrict("m_pHand"),
		NetStringStrict("m_pFoot"),
		NetIntAny("m_ColorBody"),
		NetIntAny("m_ColorFeet"),
		NetIntAny("m_ColorTopper"),
		NetIntAny("m_ColorSkin"),
		NetIntAny("m_BloodColor"),
		NetIntRange("m_IsBot", 0, 1),
		NetIntRange("m_Language", 0, 999),
	]),

	NetMessage("Cl_ChangeInfo", [
		NetStringStrict("m_pName"),
		NetStringStrict("m_pClan"),
		NetIntAny("m_Country"),
		NetStringStrict("m_pTopper"),
		NetStringStrict("m_pEye"),
		NetStringStrict("m_pHead"),
		NetStringStrict("m_pBody"),
		NetStringStrict("m_pHand"),
		NetStringStrict("m_pFoot"),
		NetIntAny("m_ColorBody"),
		NetIntAny("m_ColorFeet"),
		NetIntAny("m_ColorTopper"),
		NetIntAny("m_ColorSkin"),
		NetIntAny("m_BloodColor"),
		NetIntRange("m_IsBot", 0, 1),
        NetIntRange("m_Language", 0, 999),
	]),

	NetMessage("Cl_Kill", []),

	NetMessage("Cl_Emoticon", [
		NetIntRange("m_Emoticon", 0, 'NUM_EMOTICONS-1'),
	]),
	
	
	NetMessage("Cl_DropWeapon", []),
	
	#NetMessage("Cl_SwitchGroup", []),
	
	NetMessage("Cl_SelectItem", [
		NetIntRange("m_Item", 0, 99),
	]),
	
	NetMessage("Cl_UseKit", [
		NetIntRange("m_Kit", 0, 99),
		NetIntAny("m_X"),
		NetIntAny("m_Y"),
	]),
	
	NetMessage("Cl_Vote", [
		NetIntRange("m_Vote", -1, 1),
	]),
	
	NetMessage("Cl_VoteGameMode", [
		NetIntRange("m_Vote", 0, 6),
	]),

	NetMessage("Cl_CallVote", [
		NetStringStrict("m_Type"),
		NetStringStrict("m_Value"),
		NetStringStrict("m_Reason"),
	]),
	
	NetMessage("Cl_InventoryAction", [
		NetIntRange("m_Type", 0, 6),
		NetIntAny("m_Item1"),
		NetIntAny("m_Item2"),
		NetIntAny("m_Slot"),
	]),
]
