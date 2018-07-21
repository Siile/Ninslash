

#ifndef GAME_CLIENT_RENDER_H
#define GAME_CLIENT_RENDER_H

#include <base/vmath.h>
#include <base/tl/array.h>
#include <engine/graphics.h>
#include <game/mapitems.h>
#include "ui.h"
#include "spine.h"

#include <map>

class CTeeRenderInfo
{
public:
	CTeeRenderInfo()
	{
		m_Texture = -1;
		m_TopperTexture = -1;
		m_EyeTexture = -1;
		m_Body = 0;
		m_ColorBody = vec4(1,1,1,1);
		m_ColorFeet = vec4(1,1,1,1);
		m_ColorTopper = vec4(1,1,1,1);
		m_ColorSkin = vec4(1,1,1,1);
		m_Size = 1.0f;
		m_GotAirJump = 1;
		m_Mask = 0;
		m_IsBot = false;
		m_BloodColor = 0;
	};

	int m_Texture;
	int m_TopperTexture;
	int m_EyeTexture;
	int m_Body;
	int m_Mask;
	vec4 m_ColorBody;
	vec4 m_ColorFeet;
	vec4 m_ColorTopper;
	vec4 m_ColorSkin;
	
	bool m_IsBot;
	int m_BloodColor;
	
	float m_Size;
	int m_GotAirJump;
};

// sprite renderings
enum
{
	SPRITE_FLAG_FLIP_Y=1,
	SPRITE_FLAG_FLIP_X=2,

	LAYERRENDERFLAG_OPAQUE=1,
	LAYERRENDERFLAG_TRANSPARENT=2,

	TILERENDERFLAG_EXTEND=4,
};


// bone animation
class CAnimBone
{
public:
	CAnimBone *m_pParent; // NULL if root

	string m_Name; // needed for animation

	vec2 m_Position;
	vec2 m_Scale;
	float m_Rotation;

	mat33 m_Transform; // absolute transformation, needs to be updated on change of position/scale/rotation
	float m_Length; // debug only, remove later?
};

class CAnimAttachmentSlot
{
public:
	CAnimBone *m_pBone;

	string m_Name;
	string m_AttachmentSetup;
};

enum
{
	ATTACHMENT_SPRITE=1,
	ATTACHMENT_BBOX,
	ATTACHMENT_SKINNED_MESH,
};

class CAnimAttachment
{
public:
	string m_Name;
	int m_Type;
};

class CAnimAttachmentSprite : public CAnimAttachment
{
public:
	CAnimAttachmentSprite() { m_Type = ATTACHMENT_SPRITE; }

	vec2 m_Position;
	vec2 m_Scale;
	float m_Rotation;
	float m_Width, m_Height;
};

class CAnimAttachmentSkinnedMesh : public CAnimAttachment
{
public:
	CAnimAttachmentSkinnedMesh() { m_Type = ATTACHMENT_SKINNED_MESH; }

	struct SVertex
	{
		array<int> m_aBones;
		array<vec2> m_aPoints;
		array<float> m_aWeights;
	};

	array<vec2> m_aUVs;
	array<int> m_aTriangles;
	array<int> m_aEdges;
	array<SVertex> m_aVertices;
};

class CAnimSkeletonInfo
{
public:
	array<CAnimBone *> m_lBones; // care on removing bones, owned ptrs
	array<CAnimAttachmentSlot *> m_lSlots; // owned ptrs
	std::map<string, std::map<string, std::map<string, CAnimAttachment *> > > m_lSkins; // owned ptrs

	// lazy mode (:
	std::map<string, CSpineAnimation> m_lAnimations;

public:

	CAnimSkeletonInfo() {}
	virtual ~CAnimSkeletonInfo();

	void UpdateBones(float Time = 0.0f, CSpineAnimation *pAnimation = 0x0, class CSkeletonAnimation *pAnimData = 0x0, int WeaponAngle = 0, struct CDroidAnim *pDroidAnim = 0x0);
};

// texture atlas
class CTextureAtlasSprite
{
public:
	int m_PageId;

	bool m_Rotate;
	float m_X, m_Y;
	float m_Width, m_Height;
};

class CTextureAtlasPage : public CImageInfo
{
public:
	int m_TexId;
};

class CTextureAtlas
{
public:
	array<CTextureAtlasPage> m_lPages;
	std::map<string, CTextureAtlasSprite> m_lSprites;
};

typedef void (*ENVELOPE_EVAL)(float TimeOffset, int Env, float *pChannels, void *pUser);
typedef float (*ANIM_EVAL)(float TimeOffset, void *pUser);


class CRenderTools
{
public:
	class IGraphics *m_pGraphics;
	class CUI *m_pUI;
	class CSkelebank *m_pSkelebank;

	class IGraphics *Graphics() const { return m_pGraphics; }
	class CUI *UI() const { return m_pUI; }
	class CSkelebank *Skelebank() { return m_pSkelebank; }

	// spine import
	void LoadSkeletonFromSpine(CAnimSkeletonInfo *pSkeleton,
		const array<CSpineBone> &lBones,
		const array<CSpineSlot> &lSlots,
		const SkinMap &mSkins,
		const std::map<string, CSpineAnimation> &mAnimations);

	bool LoadAtlasFromSpine(CTextureAtlas *pAtlas, const CSpineAtlas &rSpineAtlas);

	void SelectSprite(struct CDataSprite *pSprite, int Flags=0, int sx=0, int sy=0, float ox1=0.0f, float ox2=1.0f);
	void SelectSprite(int id, int Flags=0, int sx=0, int sy=0, float x1=0.0f, float x2=1.0f);

	void DrawSprite(float x, float y, float size);

	// rects
	void DrawRoundRect(float x, float y, float w, float h, float r);
	void DrawRoundRectExt(float x, float y, float w, float h, float r, int Corners);

	void DrawUIRect(const CUIRect *pRect, vec4 Color, int Corners, float Rounding);

	// larger rendering methods
	void RenderTilemapGenerateSkip(class CLayers *pLayers);

	// object render methods (gc_render_obj.cpp)
	//void RenderTee(class CAnimState *pAnim, CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos);

	void RenderForegroundHand(class CPlayerInfo *PlayerInfo);

	// for selection menu
	void RenderTopper(CTeeRenderInfo *pInfo, vec2 Pos);
	void RenderEye(CTeeRenderInfo *pInfo, vec2 Pos);

	void RenderLine(vec2 p1, vec2 p2, float Size, vec4 Color);

	void RenderShield(vec2 Pos, vec2 Size, float State);
	void RenderHeal(vec2 Pos, vec2 Size, float State);
	
	void RenderMelee(class CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, vec2 Dir, vec2 Pos);
	void RenderScythe(class CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, vec2 Dir, vec2 Pos);
	
	void RenderArm(class CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, vec2 Pos, vec2 PlayerPos);
	
	void RenderFreeHand(class CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, int Hand, vec2 Dir, vec2 Pos, bool Behind = false);

	// render player with custom info (teesplatter, bounciness etc...)
	void RenderPlayer(class CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, int WeaponNum, int Emote, vec2 Dir, vec2 Pos);
	
	void RenderWeapon(int Weapon, vec2 Pos, vec2 Dir, float Size, bool BeginQuads = false, int Flags = 0, float Alpha2 = 1.0f, bool KillMessage = false, bool NoFlags = false);

	void RenderStaticPlayer(CTeeRenderInfo *pInfo, vec2 Pos);
	void RenderPortrait(CTeeRenderInfo *pInfo, vec2 Position, int EyeType);

	void RenderWalker(vec2 Pos, int Anim, float Time, int Dir, float Angle, int Status);
	void RenderStarDroid(vec2 Pos, int Anim, float Time, int Dir, float Angle, int Status, struct CDroidAnim *pDroidAnim = 0x0);
	void RenderCrawlerDroid(vec2 Pos, int Anim, float Time, int Dir, float Angle, int Status, class CDroidAnim *pDroidAnim = 0x0, bool Render = true);

	void RenderCrawlerLegs(class CDroidAnim *pDroidAnim);


	// skeleton render methods
	void RenderSkeleton(vec2 Position, const CTeeRenderInfo *pInfo, class CSkeletonAnimation *AnimData, float Rotation, CAnimSkeletonInfo *pSkeleton, CTextureAtlas *pAtlas, class CPlayerInfo *PlayerInfo = NULL);
	void RenderBuilding(vec2 Position, CAnimSkeletonInfo *pSkeleton, CTextureAtlas *pAtlas, int Team, int WeaponAngle = 0);

	void RenderSkeleton(vec2 Pos, int Atlas, const char *Anim, float Time, vec2 Scale, int Dir, float Angle, int Team = -1);
	
	template<typename TKeyframe>
	static void RenderEvalSkeletonAnim(TKeyframe *pKeyFrame, int NumKeyframes, float Time, typename TKeyframe::KeyframeReturnType *pResult);

	void SetShadersForPlayer(const class CPlayerInfo *pCustomPlayerInfo);
	void SetShadersForWeapon(class CPlayerInfo *pCustomPlayerInfo);
	void SetShadersForWeapon(int Weapon, float Charge = 0.0f, float Visibility = 1.0f, float Electro = 0.0f, float Damage = 0.0f, float Deathray = 0.0f);

	// map render methods (gc_render_map.cpp)
	static void RenderEvalEnvelope(CEnvPoint *pPoints, int NumPoints, int Channels, float Time, float *pResult);
	void RenderQuads(CQuad *pQuads, int NumQuads, int Flags, ENVELOPE_EVAL pfnEval, void *pUser);
	void RenderTilemap(CTile *pTiles, int w, int h, float Scale, vec4 Color, int RenderFlags, ENVELOPE_EVAL pfnEval, void *pUser, int ColorEnv, int ColorEnvOffset);

	// helpers
	void MapscreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
		float OffsetX, float OffsetY, float Aspect, float Zoom, float *pPoints);

	vec3 GetColorV3(int v);


	void RenderFullScreenLayer(vec2 Center);
};

#include "render_anim.inl"

#endif
