
#pragma once

#include <base/tl/array.h>
#include <base/tl/sorted_array.h>
#include <base/tl/string.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/shared/linereader.h>

#include <map> // hmm..

// JSON representation of the spine items
class CSpineBone
{
public:
	string m_Name;
	string m_Parent;

	float m_Length;
	float m_X, m_Y;
	float m_ScaleX, m_ScaleY;
	float m_Rotation;
};

class CSpineSlot
{
public:
	string m_Name;
	string m_Bone;
	string m_Color;
	string m_Attachment;
};

enum
{
	SPINE_ATTACHMENT_REGION=0,
	SPINE_ATTACHMENT_REGIONSEQUENCE,
	SPINE_ATTACHMENT_BBOX,
	SPINE_ATTACHMENT_MESH,
	SPINE_ATTACHMENT_SKINNED_MESH,

	SPINE_SEQUENCE_MODE_FORWARD=0,
	SPINE_SEQUENCE_MODE_BACKWARD,
	SPINE_SEQUENCE_MODE_FORWARD_LOOP,
	SPINE_SEQUENCE_MODE_BACKWARD_LOOP,
	SPINE_SEQUENCE_MODE_PING_PONG,
	SPINE_SEQUENCE_MODE_RANDOM,
};

class CSpineAttachment
{
public:
	string m_Name;
	int m_Type;

	struct
	{
		float m_X, m_Y;
		float m_ScaleX, m_ScaleY;
		float m_Rotation;
		float m_Width, m_Height;
	} m_Region;

	struct
	{
		float m_Fps;
		int m_Mode;
	} m_RegionSeq;

	struct
	{
		// TODO: some vertices, need an example..
	} m_BBox;

	struct
	{
		array<float> m_aUV;
		array<int> m_aTriangles;
		array<float> m_aVertices;
		int m_Hull;
		array<int> m_aEdges;
		float m_Width;
		float m_Height;
	} m_SkinnedMesh;
};

class CSpineEvent
{
public:

	// TODO:
};

// animations
enum
{
	SPINE_CURVE_LINEAR=0,
	SPINE_CURVE_STEPPED,
	SPINE_CURVE_BEZIER,
};

// timelines
class CSpineTimelineCurve
{
public:
	int m_Type;
	array<float> m_lPoints; // for bezier
};

// bone timelines
class CSpineBoneKeyframeTranslate
{
public:
	float m_Time;
	CSpineTimelineCurve m_Curve;

	float m_X, m_Y; // relative

public:
	// TODO: shouldn't be here
	bool operator< (CSpineBoneKeyframeTranslate const& rOther) const { return m_Time < rOther.m_Time; }

	//
	typedef float KeyframeReturnType;
	enum { NumValues = 2 };

	int GetCurveType() const { return m_Curve.m_Type; }

	static KeyframeReturnType GetValueDefault() { return 0.0f; }
	KeyframeReturnType GetValue0() const { return m_X; }
	KeyframeReturnType GetValue1() const { return m_Y; }

	static KeyframeReturnType interpolate(KeyframeReturnType a, KeyframeReturnType b, KeyframeReturnType t) { return mix(a, b, t); }
};

class CSpineBoneKeyframeScale
{
public:
	float m_Time;
	CSpineTimelineCurve m_Curve;

	float m_ScaleX, m_ScaleY; // relative

public:
	// TODO: shouldn't be here
	bool operator< (CSpineBoneKeyframeScale const& rOther) const { return m_Time < rOther.m_Time; }

	typedef float KeyframeReturnType;
	enum { NumValues = 2 };

	int GetCurveType() const { return m_Curve.m_Type; }

	static KeyframeReturnType GetValueDefault() { return 1.0f; }
	KeyframeReturnType GetValue0() const { return m_ScaleX; }
	KeyframeReturnType GetValue1() const { return m_ScaleY; }

	static KeyframeReturnType interpolate(KeyframeReturnType a, KeyframeReturnType b, KeyframeReturnType t) { return mix(a, b, t); }
};

class CSpineBoneKeyframeRotate
{
public:
	float m_Time;
	CSpineTimelineCurve m_Curve;

	float m_Rotation; // relative

public:
	// TODO: shouldn't be here
	bool operator< (CSpineBoneKeyframeRotate const& rOther) const { return m_Time < rOther.m_Time; }

	typedef float KeyframeReturnType;
	enum { NumValues = 1 };

	int GetCurveType() const { return m_Curve.m_Type; }

	static KeyframeReturnType GetValueDefault() { return 0.0f; }
	KeyframeReturnType GetValue0() const { return m_Rotation; }
	KeyframeReturnType GetValue1() const { dbg_assert(false, "Invalid value"); return 0.0f; }

	static KeyframeReturnType interpolate(KeyframeReturnType a, KeyframeReturnType b, KeyframeReturnType t) { return mix_angle(a, b, t); }
};

class CSpineBoneTimeline
{
public:
	sorted_array<CSpineBoneKeyframeTranslate> m_lTranslations;
	sorted_array<CSpineBoneKeyframeScale> m_lScales;
	sorted_array<CSpineBoneKeyframeRotate> m_lRotations;
};

// slot timeline
class CSpineSlotKeyframeAttachment
{
public:
	float m_Time;
	string m_Attachment;

public:
	// TODO: shouldn't be here
	bool operator< (CSpineSlotKeyframeAttachment const& rOther) const { return m_Time < rOther.m_Time; }

	typedef string KeyframeReturnType;
	enum { NumValues = 1 };

	int GetCurveType() const { return SPINE_CURVE_STEPPED; }

	KeyframeReturnType GetValue0() const { return m_Attachment; }
};

class CSpineSlotKeyframeColor
{
public:
	float m_Time;
	string m_Color;
	CSpineTimelineCurve m_Curve;

public:
	// TODO: shouldn't be here
	bool operator< (CSpineSlotKeyframeColor const& rOther) const { return m_Time < rOther.m_Time; }

	typedef string KeyframeReturnType;
	enum { NumValues = 1 };

	int GetCurveType() const { return m_Curve.m_Type; }

	KeyframeReturnType GetValue0() const { return m_Color; }
};

class CSpineSlotTimeline
{
public:
	sorted_array<CSpineSlotKeyframeAttachment> m_lAttachments;
	sorted_array<CSpineSlotKeyframeColor> m_lColors;
};

// event timeline
class CSpineEventKeyframe
{
public:
	float m_Time;
	string m_Name;

	bool m_HasInt, m_HasFloat, m_HasString;
	int m_Integer;
	float m_Float;
	string m_String;

public:
	// TODO: shouldn't be here
	bool operator< (CSpineEventKeyframe const& rOther) const { return m_Time < rOther.m_Time; }
};

class CSpineEventTimeline
{
public:
	sorted_array<CSpineEventKeyframe> m_lKeyframes;
};

// draworder timeline
class CSpineDraworderKeyframe
{
public:
	string m_Name;
	array<int> m_Offset;
};

class CSpineAnimation
{
public:
	std::map<string, CSpineSlotTimeline> m_lSlotTimeline;
	std::map<string, CSpineBoneTimeline> m_lBoneTimeline;
};


// texture atlas
class CSpineAtlasRegion
{
public:
	string m_Name;
	bool m_Rotate; // TODO: CW?
	int m_X, m_Y;
	int m_Width, m_Height;
	int m_OrigWidth, m_OrigHeight;
	int m_OffsetX, m_OffsetY;
	int m_Index; // TODO: ?
};

enum
{
	// TODO:
	SPINE_ATLAS_FMT_R8G8B8A8=0,

	// TODO:
	SPINE_ATLAS_FILTER_NEAREST=0,
	SPINE_ATLAS_FILTER_LINEAR,
};

class CSpineAtlasPage
{
public:
	string m_Name;
	int m_Format;
	int m_Width, m_Height;
	int m_FilterMin, m_FilterMag;
	int Repeat; // TODO: ?

	array<CSpineAtlasRegion> m_lRegions;
};

class CSpineAtlas
{
public:
	array<CSpineAtlasPage> m_lPages;
};

typedef std::map<string, std::map<string, std::map<string, CSpineAttachment> > > SkinMap;

// spine file reader
class CSpineReader
{
	CLineReader m_LineReader;

	int ParsePageFormat(const char *pFormat) const;
	int ParsePageFilter(const char *pFilter) const;

public:
	CSpineReader();
	~CSpineReader();

	bool LoadFromFile(class IStorage *pStorage, const char *pFilename, int StorageType,
		array<CSpineBone> *plBones,
		array<CSpineSlot> *plSlots,
		SkinMap *pmSkins,
		std::map<string, CSpineAnimation>* pmAnimations);

	bool Load(const char *pJsonData,
		array<CSpineBone> *plBones,
		array<CSpineSlot> *plSlots,
		SkinMap *pmSkins,
		std::map<string, CSpineAnimation>* pmAnimations);

	bool LoadAtlasFromFile(class IStorage *pStorage , const char *pFilename, int StorageType, CSpineAtlas* pAtlas);
	bool LoadAtlas(const char *pAtlasData, CSpineAtlas *pAtlas);

	void Clear();
};
