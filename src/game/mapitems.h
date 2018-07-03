

#ifndef GAME_MAPITEMS_H
#define GAME_MAPITEMS_H

// layer types
enum
{
	LAYERTYPE_INVALID=0,
	LAYERTYPE_GAME,
	LAYERTYPE_TILES,
	LAYERTYPE_QUADS,
	LAYERTYPE_SKELETONS,

	MAPITEMTYPE_VERSION=0,
	MAPITEMTYPE_INFO,
	MAPITEMTYPE_IMAGE,
	MAPITEMTYPE_ENVELOPE,
	MAPITEMTYPE_GROUP,
	MAPITEMTYPE_LAYER,
	MAPITEMTYPE_ENVPOINTS,
	MAPITEMTYPE_SKELETON,
	MAPITEMTYPE_ATLAS,

	CURVETYPE_STEP=0,
	CURVETYPE_LINEAR,
	CURVETYPE_SLOW,
	CURVETYPE_FAST,
	CURVETYPE_SMOOTH,
	NUM_CURVETYPES,

	// game layer tiles
	// row 1
	ENTITY_NULL=0,
	ENTITY_SPAWN,
	ENTITY_SPAWN_RED,
	ENTITY_SPAWN_BLUE,
	ENTITY_FLAGSTAND_RED,
	ENTITY_FLAGSTAND_BLUE,
	ENTITY_AMMO_1,
	ENTITY_HEALTH_1,
	ENTITY_ARMOR_1,
	ENTITY_KIT,
	ENTITY_RANDOM_WEAPON,
	ENTITY_BALL,
	
	// row 2
	ENTITY_SWITCH = 16*1 + 1,
	ENTITY_DOOR1,
	ENTITY_SPEAKER,
	ENTITY_BARREL,
	ENTITY_POWERBARREL,
	ENTITY_LIGHTNINGWALL,
	ENTITY_REACTOR,
	ENTITY_TESLACOIL,
	ENTITY_SCREEN,
	ENTITY_STAND,
	ENTITY_FLAMETRAP_RIGHT,
	ENTITY_FLAMETRAP_LEFT,
	ENTITY_EMPTY,
	ENTITY_SAWBLADE,
	ENTITY_LAZER,
	ENTITY_POWERUPPER,
	
	// row 3
	ENTITY_SHOP = 16*2 + 1,
	
	// row 4
	ENTITY_ENEMYSPAWN = 16*3 + 1,
	ENTITY_TURRET,
	ENTITY_DROID_WALKER,
	ENTITY_DROID_STAR,
	ENTITY_DROID_CRAWLER,
	NUM_ENTITIES,

	TILE_AIR=0,
	TILE_SOLID,
	TILE_DEATH,
	TILE_INSTADEATH,
	TILE_RAMP_LEFT,
	TILE_RAMP_RIGHT,
	TILE_ROOFSLOPE_LEFT,
	TILE_ROOFSLOPE_RIGHT,
	TILE_DAMAGEFLUID,
	TILE_MOVELEFT,
	TILE_MOVERIGHT,
	TILE_HANG,

	TILEFLAG_VFLIP=1,
	TILEFLAG_HFLIP=2,
	TILEFLAG_OPAQUE=4,
	TILEFLAG_ROTATE=8,

	LAYERFLAG_DETAIL=1,
	TILESLAYERFLAG_GAME=1,

	ENTITY_OFFSET=255-16*4,
};

struct CPoint
{
	int x, y; // 22.10 fixed point
};

struct CColor
{
	int r, g, b, a;
};

struct CQuad
{
	CPoint m_aPoints[5];
	CColor m_aColors[4];
	CPoint m_aTexcoords[4];

	int m_PosEnv;
	int m_PosEnvOffset;

	int m_ColorEnv;
	int m_ColorEnvOffset;
};

class CTile
{
public:
	unsigned char m_Index;
	unsigned char m_Flags;
	unsigned char m_Skip;
	unsigned char m_Reserved;
};

struct CMapItemInfo
{
	int m_Version;
	int m_Author;
	int m_MapVersion;
	int m_Credits;
	int m_License;
} ;

struct CMapItemImage_v1
{
	int m_Version;
	int m_Width;
	int m_Height;
	int m_External;
	int m_ImageName;
	int m_ImageData;
} ;

struct CMapItemImage : public CMapItemImage_v1
{
	enum { CURRENT_VERSION=2 };
	int m_Format;
};

struct CMapItemGroup_v1
{
	int m_Version;
	int m_OffsetX;
	int m_OffsetY;
	int m_ParallaxX;
	int m_ParallaxY;

	int m_StartLayer;
	int m_NumLayers;
} ;


struct CMapItemGroup : public CMapItemGroup_v1
{
	enum { CURRENT_VERSION=3 };

	int m_UseClipping;
	int m_ClipX;
	int m_ClipY;
	int m_ClipW;
	int m_ClipH;

	int m_aName[3];
} ;

struct CMapItemLayer
{
	int m_Version;
	int m_Type;
	int m_Flags;
} ;

struct CMapItemLayerTilemap
{
	CMapItemLayer m_Layer;
	int m_Version;

	int m_Width;
	int m_Height;
	int m_Flags;

	CColor m_Color;
	int m_ColorEnv;
	int m_ColorEnvOffset;

	int m_Image;
	int m_Data;

	int m_aName[3];
} ;

struct CMapItemLayerQuads
{
	CMapItemLayer m_Layer;
	int m_Version;

	int m_NumQuads;
	int m_Data;
	int m_Image;

	int m_aName[3];
} ;

// TODO: spine
struct CMapItemLayerSkeleton
{
	enum { CURRENT_VERSION=1 };

	CMapItemLayer m_Layer;
	int m_Version;

	int m_NumSkeletons;
	int m_DataSkeletons;

	int m_Skeleton;
	int m_Atlas;

	int m_aName[3];
};

struct CMapItemVersion
{
	int m_Version;
} ;

struct CEnvPoint
{
	int m_Time; // in ms
	int m_Curvetype;
	int m_aValues[4]; // 1-4 depending on envelope (22.10 fixed point)

	bool operator<(const CEnvPoint &Other) { return m_Time < Other.m_Time; }
} ;

struct CMapItemEnvelope_v1
{
	int m_Version;
	int m_Channels;
	int m_StartPoint;
	int m_NumPoints;
	int m_aName[8];
} ;

struct CMapItemEnvelope : public CMapItemEnvelope_v1
{
	enum { CURRENT_VERSION=2 };
	int m_Synchronized;
};
/*
// TODO: spine
// pretty much tailored towards spine format, maybe something more abstract?
struct CBone
{
	int m_Parent; // -1 if root

	int m_X, m_Y; // 22.10 fixed point
	int m_ScaleX, m_ScaleY; // 22.10 fixed point
	int m_Rotation; // 22.10 fixed point

	// debug only
	int Length; // 22.10 fixed point
};



struct CSlot
{
	int m_Bone;
	int m_Color; // debug only?
	int m_Attachment;
};

struct CAttachment
{
	int m_Type;
	int m_Data;
};

struct CAttachmentDataRegion
{
	int m_X, m_Y; // 22.10 fixed point
	int m_ScaleX, m_ScaleY; // 22.10 fixed point
	int m_Rotation; // 22.10 fixed point
	int m_Width, m_Height; // 22.10 fixed point
};

struct CAttachmentDataRegionSeq
{
	// TODO:
};

struct CAttachmentDataBBox
{
	// TODO:
};

*/

// instance of a skeleton with skin
struct CSkeleton
{
	int m_X, m_Y; // 22.10 fixed point
	int m_ScaleX, m_ScaleY; // 22.10 fixed point
	int m_Rotation; // 22.10 fixed point
	int m_Skin;
};

struct CMapItemSkeleton
{
	enum { CURRENT_VERSION=1 };
	int m_Version;

	int m_External;

	int m_Data; // json file content
	int m_SkeletonName;
};

struct CMapItemAtlas
{
	enum { CURRENT_VERSION=1 };
	int m_Version;

	int m_Name;
	int m_External;

	int m_Data; // atlas file content
	
	int m_NumPages;
	int m_DataPages;
};

// will handle images and atlases individual
struct CAtlasPage : CMapItemImage
{
	// filter/format/wrapping are fixed
	enum { CURRENT_VERSION=1 };
};

#endif
