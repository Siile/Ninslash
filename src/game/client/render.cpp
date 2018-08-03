#include <math.h>
#include <base/math.h>
#include <base/vmath.h>
#include <cstring>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/map.h>
#include <game/generated/client_data.h>
#include <game/generated/protocol.h>
#include <game/layers.h>
#include "render.h"
#include "skelebank.h"
#include "animdata.h"

#include <game/client/customstuff.h>
#include <game/client/customstuff/playerinfo.h>
#include <game/client/customstuff/droidanim.h>

static float gs_SpriteWScale;
static float gs_SpriteHScale;

#define RAD 0.017453292519943295769236907684886f

//inline int GetPart(int Weapon, int Group){ return (Weapon & (15<<(2+Group*4)))>>(2+Group*4); }

static mat33 CalcTransformationMatrix(vec2 Trans, vec2 Scale, float Rotation)
{
	float s = sin(Rotation);
	float c = cos(Rotation);

	return mat33(
			Scale.x * c, -Scale.x * s, Trans.x,
			Scale.y * s, Scale.y * c, Trans.y,
			0.0f, 0.0f, 1.0f
		);
}

CAnimSkeletonInfo::~CAnimSkeletonInfo()
{
	m_lBones.delete_all();
	m_lSlots.delete_all();

	// TODO: free skin map
}

void CAnimSkeletonInfo::UpdateBones(float Time, CSpineAnimation *pAnimation, CSkeletonAnimation *pAnimData, int WeaponAngle, CDroidAnim *pDroidAnim)
{
	// we assume, that the bones are ordered by their place in the skeleton hierarchy, important!
	for(int i = 0; i < m_lBones.size(); i++)
	{
		CAnimBone *pBone = m_lBones[i];

		vec2 Position = vec2(0.0f, 0.0f);
		vec2 Scale = vec2(1.0f, 1.0f);
		float Rotation = 0.0f;

		if(pAnimation)
		{
			CSpineBoneTimeline *pBoneTimeline = 0x0;
			{
				auto BoneTimelineIter = pAnimation->m_lBoneTimeline.find(pBone->m_Name);
				if(BoneTimelineIter != pAnimation->m_lBoneTimeline.end())
					pBoneTimeline = &BoneTimelineIter->second;
			}

			if(pBoneTimeline)
			{
				CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeTranslate>(pBoneTimeline->m_lTranslations.base_ptr(), pBoneTimeline->m_lTranslations.size(), Time, (float *)&Position);
				CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeScale>(pBoneTimeline->m_lScales.base_ptr(), pBoneTimeline->m_lScales.size(), Time, (float *)&Scale);
				CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeRotate>(pBoneTimeline->m_lRotations.base_ptr(), pBoneTimeline->m_lRotations.size(), Time, (float *)&Rotation);
			}

			// convert values
			Position.y *= -1.0f;
			Rotation = (360.0f - Rotation)*pi/180.0f;


			if (pBone->m_SpecialType == BST_WEAPON || pBone->m_SpecialType == BST_WEAPON_TEAM)
			{
				if (WeaponAngle > 180)
				{
					Scale.x *= -1;
					WeaponAngle = 360 - WeaponAngle;
				}
				Rotation = WeaponAngle*pi/180.0f;
			}
			

			
			// star droids
			if (pDroidAnim)
			{
				if (pBone->m_SpecialType == BST_THRUST1 || pBone->m_SpecialType == BST_THRUST2)
				{
					Rotation += pDroidAnim->m_aValue[CDroidAnim::VEL_X] - pDroidAnim->m_aValue[CDroidAnim::BODY_ANGLE];

					if (pBone->m_SpecialType == BST_THRUST1)
					{
						//pDroidAnim->m_aVectorValue[CDroidAnim::THRUST1_POS] = Position;
						pDroidAnim->m_aVectorValue[CDroidAnim::THRUST1_VEL] = vec2(sin(Rotation), cos(Rotation));
					}
					else if (pBone->m_SpecialType == BST_THRUST2)
					{
						//pDroidAnim->m_aVectorValue[CDroidAnim::THRUST2_POS] = Position;
						pDroidAnim->m_aVectorValue[CDroidAnim::THRUST2_VEL] = vec2(sin(Rotation), cos(Rotation));
					}
				}
				
				else if (pBone->m_SpecialType == BST_BODY)
					Rotation += pDroidAnim->m_aValue[CDroidAnim::BODY_ANGLE];
				
				else if (pBone->m_SpecialType == BST_WEAPON0 || pBone->m_SpecialType == BST_WEAPON1 || pBone->m_SpecialType == BST_WEAPON2)
					Rotation = pDroidAnim->m_aValue[CDroidAnim::TURRET_ANGLE]*pi/180.0f  - pDroidAnim->m_aValue[CDroidAnim::BODY_ANGLE];
			}
			else
			// walkers
			if (pBone->m_SpecialType == BST_WEAPON0 || pBone->m_SpecialType == BST_WEAPON1 || pBone->m_SpecialType == BST_WEAPON2)
			{
				Rotation = WeaponAngle*pi/180.0f;
			}
			
			if (pAnimData)
			{
				if (pAnimData->m_Anim != PANIM_IDLE && (pBone->m_SpecialType == BST_FFOOT || pBone->m_SpecialType == BST_BFOOT))
				{
					Position += pAnimData->m_FeetDir;
				}
				
				if (pBone->m_SpecialType == BST_BODY)
				{
					Position += vec2(pAnimData->m_HeadOffset.y, pAnimData->m_HeadOffset.x) * 0.5f;
					Rotation += pAnimData->m_BodyTilt;
				}
				
				if (pBone->m_SpecialType == BST_HEAD)
				{
					Position += pAnimData->m_HeadOffset;
					Rotation += pAnimData->m_HeadTilt + pAnimData->m_HeadTiltCorrect;
					pAnimData->m_HeadTargetAngle = (pAnimData->m_HeadTilt + pAnimData->m_HeadTiltCorrect)*0.5f; // - pBone->m_Rotation;
					pAnimData->UpdateHead();
					Rotation = Rotation + pAnimData->m_HeadAngle;
				}
			}
		}

		Position += pBone->m_Position;
		Scale *= pBone->m_Scale; // TODO: ?
		Rotation += pBone->m_Rotation;

		pBone->m_Transform = CalcTransformationMatrix(Position, Scale, Rotation);
		if(pBone->m_pParent != 0x0)
			pBone->m_Transform = pBone->m_pParent->m_Transform * pBone->m_Transform;
	}
}



void CRenderTools::LoadSkeletonFromSpine(CAnimSkeletonInfo *pSkeleton,
		const array<CSpineBone> &lBones,
		const array<CSpineSlot>& lSlots,
		const SkinMap &mSkins,
		const std::map<string, CSpineAnimation> &mAnimations)
{
	if(!pSkeleton)
		return;
	
	// create bones
	int NumBones = lBones.size();
	for(int i = 0; i < NumBones; i++)
	{
		const CSpineBone &rBone = lBones[i];
		CAnimBone *pBone = new CAnimBone;
		pBone->m_Name = rBone.m_Name;
		pBone->m_SpecialType = BST_NONE;
		
		if (strcmp(pBone->m_Name, "weapon") == 0)
			pBone->m_SpecialType = BST_WEAPON;
		
		else if (strcmp(pBone->m_Name, "weapon0") == 0)
			pBone->m_SpecialType = BST_WEAPON0;
		
		else if (strcmp(pBone->m_Name, "weapon1") == 0)
			pBone->m_SpecialType = BST_WEAPON1;
		
		else if (strcmp(pBone->m_Name, "weapon2") == 0)
			pBone->m_SpecialType = BST_WEAPON2;
		
		else if (strcmp(pBone->m_Name, "weapon_team") == 0)
			pBone->m_SpecialType = BST_WEAPON_TEAM;
		
		else if (strcmp(pBone->m_Name, "thrust1") == 0)
			pBone->m_SpecialType = BST_THRUST1;
		
		else if (strcmp(pBone->m_Name, "thrust2") == 0)
			pBone->m_SpecialType = BST_THRUST2;
		
		else if (strcmp(pBone->m_Name, "body") == 0)
			pBone->m_SpecialType = BST_BODY;
		
		else if (strcmp(pBone->m_Name, "ffoot") == 0)
			pBone->m_SpecialType = BST_FFOOT;
		
		else if (strcmp(pBone->m_Name, "bfoot") == 0)
			pBone->m_SpecialType = BST_BFOOT;
		
		else if (strcmp(pBone->m_Name, "head") == 0)
			pBone->m_SpecialType = BST_HEAD;
		
		pBone->m_pParent = 0x0;
		pBone->m_Position.x = rBone.m_X;
		pBone->m_Position.y = -rBone.m_Y;
		pBone->m_Scale.x = rBone.m_ScaleX;
		pBone->m_Scale.y = rBone.m_ScaleY;
		pBone->m_Rotation = (360.0f - rBone.m_Rotation)*pi/180.0f;

		pBone->m_Length = rBone.m_Length;

		pSkeleton->m_lBones.add(pBone);
	}

	// create bone hierarchy
	// TODO: slow
	for(int i = 0; i < NumBones; i++)
	{
		const string& rParent = lBones[i].m_Parent;
		for(int j = 0; j < NumBones; j++)
		{
			if(rParent == lBones[j].m_Name)
			{
				pSkeleton->m_lBones[i]->m_pParent = pSkeleton->m_lBones[j];
				break;
			}
		}
	}

	pSkeleton->UpdateBones();

	// slots
	for(int i = 0; i < lSlots.size(); i++)
	{
		const CSpineSlot& rSlot = lSlots[i];
		CAnimAttachmentSlot *pSlot = new CAnimAttachmentSlot;

		// TODO: linear search
		for(int j = 0; j < lBones.size(); j++)
			if(lBones[j].m_Name == rSlot.m_Bone)
			{
				pSlot->m_pBone = pSkeleton->m_lBones[j];
				break;
			}

		pSlot->m_Name = rSlot.m_Name;
		pSlot->m_AttachmentSetup = rSlot.m_Attachment;
		pSkeleton->m_lSlots.add(pSlot);
	}

	// TODO: skins
	for(SkinMap::const_iterator IterSkin = mSkins.begin(); IterSkin != mSkins.end(); IterSkin++)
	{
		string SkinName = IterSkin->first;

		for(std::map<string, std::map<string, CSpineAttachment> >::const_iterator IterSlot = IterSkin->second.begin(); IterSlot != IterSkin->second.end(); IterSlot++)
		{
			string SlotName = IterSlot->first;

			for(std::map<string, CSpineAttachment>::const_iterator IterAttachment = IterSlot->second.begin(); IterAttachment != IterSlot->second.end(); IterAttachment++)
			{
				string AttachmentName = IterAttachment->first;
				CSpineAttachment Attachment = IterAttachment->second;

				switch(Attachment.m_Type)
				{
				case SPINE_ATTACHMENT_REGION:
					{
						CAnimAttachmentSprite *pAnimAttachment = new CAnimAttachmentSprite();
						pAnimAttachment->m_Name = Attachment.m_Name;
						
						if (strcmp(pAnimAttachment->m_Name, "mask") == 0)
							pAnimAttachment->m_SpecialType = AST_MASK;
						
						else if (strcmp(pAnimAttachment->m_Name, "1_team") == 0)
							pAnimAttachment->m_SpecialType = AST_1_TEAM;
						
						else if (strcmp(pAnimAttachment->m_Name, "2_team") == 0)
							pAnimAttachment->m_SpecialType = AST_2_TEAM;
						
						else if (strcmp(pAnimAttachment->m_Name, "3_team") == 0)
							pAnimAttachment->m_SpecialType = AST_3_TEAM;
						
						else if (strcmp(pAnimAttachment->m_Name, "4_team") == 0)
							pAnimAttachment->m_SpecialType = AST_4_TEAM;
						
						else if (strcmp(pAnimAttachment->m_Name, "weapon") == 0)
							pAnimAttachment->m_SpecialType = AST_WEAPON;
						
						else if (strcmp(pAnimAttachment->m_Name, "weapon_team") == 0)
							pAnimAttachment->m_SpecialType = AST_WEAPON_TEAM;
						
						else if (strcmp(pAnimAttachment->m_Name, "eyes") == 0)
							pAnimAttachment->m_SpecialType = AST_EYES;
						
						else if (strcmp(pAnimAttachment->m_Name, "hat") == 0)
							pAnimAttachment->m_SpecialType = AST_HAT;
						
						else if (strcmp(pAnimAttachment->m_Name, "body") == 0)
							pAnimAttachment->m_SpecialType = AST_BODY;
						
						else if (strcmp(pAnimAttachment->m_Name, "head") == 0)
							pAnimAttachment->m_SpecialType = AST_HEAD;
						
						else if (strcmp(pAnimAttachment->m_Name, "head_asset") == 0)
							pAnimAttachment->m_SpecialType = AST_HEAD_ASSET;
						
						else if (strcmp(pAnimAttachment->m_Name, "arm") == 0)
							pAnimAttachment->m_SpecialType = AST_ARM;
						
						else if (strcmp(pAnimAttachment->m_Name, "foot") == 0)
							pAnimAttachment->m_SpecialType = AST_FOOT;
						
						else if (strcmp(pAnimAttachment->m_Name, "hand") == 0)
							pAnimAttachment->m_SpecialType = AST_HAND;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter1") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER1;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter2") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER2;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter3") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER3;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter4") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER4;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter5") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER5;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter6") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER6;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter7") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER7;
						
						else if (strcmp(pAnimAttachment->m_Name, "splatter8") == 0)
							pAnimAttachment->m_SpecialType = AST_SPLATTER8;
						
						else if (strcmp(pAnimAttachment->m_Name, "team") == 0)
							pAnimAttachment->m_SpecialType = AST_TEAM;
						
						else if (strcmp(pAnimAttachment->m_Name, "jet") == 0)
							pAnimAttachment->m_SpecialType = AST_JET;
						
						else if (strcmp(pAnimAttachment->m_Name, "jet1") == 0)
							pAnimAttachment->m_SpecialType = AST_JET1;
						
						else if (strcmp(pAnimAttachment->m_Name, "jet2") == 0)
							pAnimAttachment->m_SpecialType = AST_JET2;
						

						pAnimAttachment->m_Position = vec2(Attachment.m_Region.m_X, -Attachment.m_Region.m_Y);
						pAnimAttachment->m_Scale = vec2(Attachment.m_Region.m_ScaleX, Attachment.m_Region.m_ScaleY);
						pAnimAttachment->m_Rotation = (360.0f - Attachment.m_Region.m_Rotation)*pi/180.0f;
						pAnimAttachment->m_Width = Attachment.m_Region.m_Width;
						pAnimAttachment->m_Height = Attachment.m_Region.m_Height;

						pSkeleton->m_lSkins[SkinName][SlotName][AttachmentName] = pAnimAttachment;
					} break;

				case SPINE_ATTACHMENT_SKINNED_MESH:
					{
						CAnimAttachmentSkinnedMesh *pAnimAttachment = new CAnimAttachmentSkinnedMesh();
						pAnimAttachment->m_Name = Attachment.m_Name;

						pAnimAttachment->m_aTriangles = Attachment.m_SkinnedMesh.m_aTriangles;
						pAnimAttachment->m_aEdges = Attachment.m_SkinnedMesh.m_aEdges;

						for(int uv = 0; uv < Attachment.m_SkinnedMesh.m_aUV.size(); uv+=2)
						{
							pAnimAttachment->m_aUVs.add(vec2(Attachment.m_SkinnedMesh.m_aUV[uv], Attachment.m_SkinnedMesh.m_aUV[uv+1]));
						}

						for(int v = 0; v < Attachment.m_SkinnedMesh.m_aVertices.size();)
						{
							int NumBones = Attachment.m_SkinnedMesh.m_aVertices[v++];
							CAnimAttachmentSkinnedMesh::SVertex Vertex;

							for(int vb = 0; vb < NumBones; vb++, v+=4)
							{
								Vertex.m_aBones.add(Attachment.m_SkinnedMesh.m_aVertices[v]);
								Vertex.m_aPoints.add(vec2(Attachment.m_SkinnedMesh.m_aVertices[v+1], -Attachment.m_SkinnedMesh.m_aVertices[v+2]));
								Vertex.m_aWeights.add(Attachment.m_SkinnedMesh.m_aVertices[v+3]);
							}

							pAnimAttachment->m_aVertices.add(Vertex);
						}

						pSkeleton->m_lSkins[SkinName][SlotName][AttachmentName] = pAnimAttachment;
					} break;

				default:
					dbg_msg("render", "Unkown spine attachment type (%d)", Attachment.m_Type);
					continue;
				}					
			}
		}
	}

	// TODO: heavy operation?
	pSkeleton->m_lAnimations = mAnimations;
}

bool CRenderTools::LoadAtlasFromSpine(CTextureAtlas *pAtlas, const CSpineAtlas &rSpineAtlas)
{
	if(!pAtlas)
		return false;

	for(int i = 0; i < rSpineAtlas.m_lPages.size(); i++)
	{
		// add page
		CTextureAtlasPage Page;
		const CSpineAtlasPage& rSpinePage = rSpineAtlas.m_lPages[i];

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "data/anim/%s", rSpinePage.m_Name.cstr());

		if(!Graphics()->LoadPNG(&Page, aBuf, IStorage::TYPE_ALL))
		{
			dbg_msg("render", "couldn't load texture: %s", aBuf);
			return false;
		}

		Page.m_TexId = Graphics()->LoadTextureRaw(Page.m_Width, Page.m_Height, Page.m_Format, Page.m_pData, CImageInfo::FORMAT_AUTO, 0);

		int PageIndex = pAtlas->m_lPages.add(Page);

		// add sprites
		for(int j = 0; j < rSpinePage.m_lRegions.size(); j++)
		{
			CSpineAtlasRegion const& rSpineRegion = rSpinePage.m_lRegions[j];

			CTextureAtlasSprite Sprite;
			Sprite.m_PageId = PageIndex;

			Sprite.m_Rotate = rSpineRegion.m_Rotate;
			Sprite.m_X = rSpineRegion.m_X;
			Sprite.m_Y = rSpineRegion.m_Y;
			Sprite.m_Width = rSpineRegion.m_Width;
			Sprite.m_Height = rSpineRegion.m_Height;

			pAtlas->m_lSprites[rSpineRegion.m_Name] = Sprite;
		}
	}

	return true;
}



	
	

void CRenderTools::SelectSprite(CDataSprite *pSpr, int Flags, int sx, int sy, float ox1, float ox2)
{
	int x = pSpr->m_X+sx;
	int y = pSpr->m_Y+sy;
	int w = pSpr->m_W;
	int h = pSpr->m_H;
	int cx = pSpr->m_pSet->m_Gridx;
	int cy = pSpr->m_pSet->m_Gridy;

	float f = sqrtf(h*h + w*w);
	gs_SpriteWScale = w/f;
	gs_SpriteHScale = h/f;

	float x1 = x/(float)cx;
	float x2 = (x+w)/(float)cx;
	float y1 = y/(float)cy;
	float y2 = (y+h)/(float)cy;
	float Temp = 0;

	if (ox1 > 0.0f || ox2 < 1.0f)
	{
		x1 += ox1/(float)cx;
		x2 -= (1.0f-ox2)/(float)cx;
	}
	
	if(Flags&SPRITE_FLAG_FLIP_Y)
	{
		Temp = y1;
		y1 = y2;
		y2 = Temp;
	}

	if(Flags&SPRITE_FLAG_FLIP_X)
	{
		Temp = x1;
		x1 = x2;
		x2 = Temp;
	}

	if (ox1 > 0.0f || ox2 < 1.0f)
		Graphics()->QuadsSetSubset(x1, y1, x2, y2, true);
	else
		Graphics()->QuadsSetSubset(x1, y1, x2, y2);
}

void CRenderTools::SelectSprite(int Id, int Flags, int sx, int sy, float x1, float x2)
{
	if(Id < 0 || Id >= g_pData->m_NumSprites)
		return;
	SelectSprite(&g_pData->m_aSprites[Id], Flags, sx, sy, x1, x2);
}

void CRenderTools::DrawSprite(float x, float y, float Size)
{
	IGraphics::CQuadItem QuadItem(x, y, Size*gs_SpriteWScale, Size*gs_SpriteHScale);
	Graphics()->QuadsDraw(&QuadItem, 1);
}

void CRenderTools::DrawRoundRectExt(float x, float y, float w, float h, float r, int Corners)
{
	IGraphics::CFreeformItem ArrayF[32];
	int NumItems = 0;
	int Num = 8;
	for(int i = 0; i < Num; i+=2)
	{
		float a1 = i/(float)Num * pi/2;
		float a2 = (i+1)/(float)Num * pi/2;
		float a3 = (i+2)/(float)Num * pi/2;
		float Ca1 = cosf(a1);
		float Ca2 = cosf(a2);
		float Ca3 = cosf(a3);
		float Sa1 = sinf(a1);
		float Sa2 = sinf(a2);
		float Sa3 = sinf(a3);

		if(Corners&1) // TL
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+r, y+r,
			x+(1-Ca1)*r, y+(1-Sa1)*r,
			x+(1-Ca3)*r, y+(1-Sa3)*r,
			x+(1-Ca2)*r, y+(1-Sa2)*r);

		if(Corners&2) // TR
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+w-r, y+r,
			x+w-r+Ca1*r, y+(1-Sa1)*r,
			x+w-r+Ca3*r, y+(1-Sa3)*r,
			x+w-r+Ca2*r, y+(1-Sa2)*r);

		if(Corners&4) // BL
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+r, y+h-r,
			x+(1-Ca1)*r, y+h-r+Sa1*r,
			x+(1-Ca3)*r, y+h-r+Sa3*r,
			x+(1-Ca2)*r, y+h-r+Sa2*r);

		if(Corners&8) // BR
		ArrayF[NumItems++] = IGraphics::CFreeformItem(
			x+w-r, y+h-r,
			x+w-r+Ca1*r, y+h-r+Sa1*r,
			x+w-r+Ca3*r, y+h-r+Sa3*r,
			x+w-r+Ca2*r, y+h-r+Sa2*r);
	}
	Graphics()->QuadsDrawFreeform(ArrayF, NumItems);

	IGraphics::CQuadItem ArrayQ[9];
	NumItems = 0;
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+r, y+r, w-r*2, h-r*2); // center
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+r, y, w-r*2, r); // top
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+r, y+h-r, w-r*2, r); // bottom
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x, y+r, r, h-r*2); // left
	ArrayQ[NumItems++] = IGraphics::CQuadItem(x+w-r, y+r, r, h-r*2); // right

	if(!(Corners&1)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x, y, r, r); // TL
	if(!(Corners&2)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x+w, y, -r, r); // TR
	if(!(Corners&4)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x, y+h, r, -r); // BL
	if(!(Corners&8)) ArrayQ[NumItems++] = IGraphics::CQuadItem(x+w, y+h, -r, -r); // BR

	Graphics()->QuadsDrawTL(ArrayQ, NumItems);
}

void CRenderTools::DrawRoundRect(float x, float y, float w, float h, float r)
{
	DrawRoundRectExt(x,y,w,h,r,0xf);
}

void CRenderTools::DrawUIRect(const CUIRect *r, vec4 Color, int Corners, float Rounding)
{
	Graphics()->TextureSet(-1);

	// TODO: FIX US
	Graphics()->QuadsBegin();
	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
	DrawRoundRectExt(r->x,r->y,r->w,r->h,Rounding*UI()->Scale(), Corners);
	Graphics()->QuadsEnd();
}




void CRenderTools::RenderFullScreenLayer(vec2 Center)
{
	if (!g_Config.m_GfxMultiBuffering)
		return;
	
	Graphics()->MapScreen(0,0,Graphics()->ScreenWidth(),Graphics()->ScreenHeight());
	
	
	
	// render blood splatter to tiles
	Graphics()->RenderToTexture(RENDERBUFFER_TILES);
	
	// test, embed image to texture buffer
	//Graphics()->ShaderBegin(SHADER_GROUND);
	/*
	vec2 s = vec2(Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_METAL].m_Id);
	
	Graphics()->BlendBuffer();

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(0.25f, 0.25f, 0.25f, 0.5f);
	
	vec2 BGSize = vec2(s.x/2*Graphics()->ScreenAspect(), s.y/2*Graphics()->ScreenAspect());
	
	{
		vec2 Size = vec2(1, 1/Graphics()->ScreenAspect())*2.5f;
		BGSize /= Size;
	
		vec2 Scroll = Center/BGSize;
		
		Graphics()->QuadsSetSubsetFree(	0.0f+Scroll.x, 0.0f+Scroll.y,
										Size.x+Scroll.x, 0.0f+Scroll.y,
										0.0f+Scroll.x, Size.y+Scroll.y,
										Size.x+Scroll.x, Size.y+Scroll.y);
									
		IGraphics::CFreeformItem Freeform(0, 0, s.x, 0, 0, s.y, s.x, s.y);
		Graphics()->QuadsDrawFreeform(&Freeform, 1);
	}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	*/
	
	Graphics()->ShaderBegin(SHADER_BLOOD);
	Graphics()->TextureSet(-2, RENDERBUFFER_SPLATTER);
	
	Graphics()->BlendBuffer();

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	//Graphics()->SetColor(0.6f, 0.0f, 0.0f, 0.8f);
	Graphics()->SetColor(1, 1, 1, 0.8f);
	
	{
		IGraphics::CQuadItem QuadItem(Graphics()->ScreenWidth() / 2, Graphics()->ScreenHeight() / 2, Graphics()->ScreenWidth(), -Graphics()->ScreenHeight());
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	// render all to screen
	Graphics()->RenderToScreen();
	Graphics()->BlendNormal();
		
	// blood
	Graphics()->ShaderBegin(SHADER_BLOOD);
	Graphics()->TextureSet(-2, RENDERBUFFER_BLOOD);

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	//Graphics()->SetColor(1.0f, 0, 0, 0.8f);
	Graphics()->SetColor(1, 1, 1, 0.8f);
	
	{
		IGraphics::CQuadItem QuadItem(Graphics()->ScreenWidth() / 2, Graphics()->ScreenHeight() / 2, Graphics()->ScreenWidth(), -Graphics()->ScreenHeight());
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
	
	// bloody tiles
	Graphics()->TextureSet(-2, RENDERBUFFER_TILES);

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	{
		IGraphics::CQuadItem QuadItem(Graphics()->ScreenWidth() / 2, Graphics()->ScreenHeight() / 2, Graphics()->ScreenWidth(), -Graphics()->ScreenHeight());
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
}



vec3 CRenderTools::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}
 


void CRenderTools::RenderShield(vec2 Pos, vec2 Size, float State)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SHIELD].m_Id);
	
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1, 1, 1, 0.5f);
	SelectSprite(SPRITE_SHIELD1 + State*8, 0, 0, 0);
	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size.x, Size.y);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CRenderTools::RenderHeal(vec2 Pos, vec2 Size, float State)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HEAL].m_Id);
	
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1, 1, 1, 0.7f);
	SelectSprite(SPRITE_HEAL1 + State*12, 0, 0, 0);
	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size.x, Size.y);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
}


void CRenderTools::RenderTopper(CTeeRenderInfo *pInfo, vec2 Pos)
{
	Graphics()->TextureSet(pInfo->m_TopperTexture);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorTopper.r, pInfo->m_ColorTopper.g, pInfo->m_ColorTopper.b, 1);

	Graphics()->QuadsSetRotation(0);
		
	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, pInfo->m_Size, pInfo->m_Size);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CRenderTools::RenderEye(CTeeRenderInfo *pInfo, vec2 Pos)
{
	Graphics()->TextureSet(pInfo->m_EyeTexture);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1, 1, 1, 1);

	Graphics()->QuadsSetRotation(0);
	Graphics()->QuadsSetSubset(0, 0, 1.0f/4, 1);
		
	IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, pInfo->m_Size, pInfo->m_Size/2);
	Graphics()->QuadsDraw(&QuadItem, 1);
	Graphics()->QuadsEnd();
}



void CRenderTools::RenderPortrait(CTeeRenderInfo *pInfo, vec2 Pos, int EyeType)
{
	vec2 Position = Pos;
	
	int Atlas = pInfo->m_Body;
	if (Atlas < 0)
		Atlas = 0;
	
	if (Atlas > NUM_BODIES-1)
		Atlas = NUM_BODIES-1;

	
	CSkeletonAnimation *AnimData = CPlayerInfo::GetIdle()->Animation();
	CAnimSkeletonInfo *pSkeleton = Skelebank()->m_lSkeletons[Atlas];
	CTextureAtlas *pAtlas = Skelebank()->m_lAtlases[Atlas];
		
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	//vec2 Scale = vec2(0.17f, 0.17f);
	vec2 Scale = vec2(1.0f, 1.0f) * pInfo->m_Size * 0.0032f;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, 0.0f);

	float Time = 0.0f;
	
	
	CSpineAnimation *pAnimation = 0x0;
	
	
	{
		auto AnimIter = pSkeleton->m_lAnimations.find(Anim[AnimData->m_Anim]);
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	

	pSkeleton->UpdateBones(Time, pAnimation, AnimData);
	


	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment

			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					if (pAttachment->m_SpecialType != AST_HEAD &&
						pAttachment->m_SpecialType != AST_HEAD_ASSET &&
						pAttachment->m_SpecialType != AST_HAT &&
						pAttachment->m_SpecialType != AST_EYES)
						continue;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);


					int SybsetType = 0;

					// render some slots with user selected texture with 
					if (pAttachment->m_SpecialType == AST_HAT)
					{
						Graphics()->TextureSet(pInfo->m_TopperTexture);
						SybsetType = 1;
					}
					else if (pAttachment->m_SpecialType == AST_EYES)
					{
						Graphics()->TextureSet(pInfo->m_EyeTexture);
						SybsetType = 2;
					}
					else
						Graphics()->TextureSet(pPage->m_TexId);
					
					Graphics()->QuadsBegin();
					

					if (pAttachment->m_SpecialType == AST_HAT)
						Graphics()->SetColor(pInfo->m_ColorTopper.r, pInfo->m_ColorTopper.g, pInfo->m_ColorTopper.b, 1);
					
					if (pAttachment->m_SpecialType == AST_HEAD)
						Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, 1);
					

					if (SybsetType == 1)
						Graphics()->QuadsSetSubsetFree(0, 0,1, 0, 0, 1, 1, 1);
					else if (SybsetType == 2)
					{
						float t0 = EyeType / 4.0f;
						float t1 = (EyeType+1) / 4.0f;
						Graphics()->QuadsSetSubsetFree(t0, 0, t1, 0, t0, 1, t1, 1);
					}
					else
					{
						vec2 t0, t1, t2, t3;
						t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
						t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

						if(pSprite->m_Rotate)
							Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
						else
							Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);
					}

					IGraphics::CFreeformItem FreeFormItem(
							p0.x, p0.y,
							p1.x, p1.y,
							p2.x, p2.y,
							p3.x, p3.y
						);

					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					Graphics()->QuadsEnd();
				} break;
			}
		}
	}
}








void CRenderTools::RenderSkeleton(vec2 Pos, int Atlas, const char *Anim, float Time, vec2 Scale, int Dir, float Angle, int Team)
{
	vec2 Position = Pos;

	CAnimSkeletonInfo *pSkeleton = Skelebank()->m_lSkeletons[Atlas];
	CTextureAtlas *pAtlas = Skelebank()->m_lAtlases[Atlas];
		
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	//vec2 Scale = vec2(1.0f, 1.0f) * 0.15f;
	
	if (Dir == 1)
		Scale.x *= -1;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, 0.0f);
	
	CSpineAnimation *pAnimation = 0x0;
	
	{
		auto AnimIter = pSkeleton->m_lAnimations.find(Anim);
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	
	pSkeleton->UpdateBones(Time, pAnimation, NULL, Angle);
	

	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment

			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);

					bool Screen = false;
					
					if (strlen(pSlot->m_Name) > 3 &&
						pSlot->m_Name[0] == '_' &&
						pSlot->m_Name[1] == 's' &&
						pSlot->m_Name[2] == 'c')
					{
						//Graphics()->ShaderBegin(SHADER_SPAWN, 0.05f);
						Screen = true;
					}
					else if (strlen(pSlot->m_Name) > 3 &&
						pSlot->m_Name[0] == 's' &&
						pSlot->m_Name[1] == 'c' &&
						pSlot->m_Name[2] == 'r')
					{
						Graphics()->ShaderBegin(SHADER_ELECTRIC, 1.0f);
						Screen = true;
					}

					Graphics()->TextureSet(pPage->m_TexId);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(1, 1, 1, 1);
					
					
					if (Screen)
					{
						switch (int(Pos.x/32)%4)
						{
							case 0: Graphics()->SetColor(0.5f, 0.7f, 1.0f, 1); break;
							case 1: Graphics()->SetColor(0.4f, 1.0f, 0.4f, 1); break;
							case 2: Graphics()->SetColor(1.0f, 0.5f, 0.5f, 1); break;
							case 3: Graphics()->SetColor(0.2f, 1.0f, 1.0f, 1); break;
							//case 4: Graphics()->SetColor(1.0f, 1.0f, 0.4f, 1); break;
							default: Graphics()->SetColor(0.5f, 0.7f, 1.0f, 1); break;
						}
					}
					
					else if (pAttachment->m_SpecialType == AST_TEAM)
					{
						if (Team == TEAM_RED)
							Graphics()->SetColor(1.0f, 0.3f, 0.0f, 1);
						else if (Team == TEAM_BLUE)
							Graphics()->SetColor(0.0f, 0.3f, 1.0f, 1);
					}
					
					
					{
						vec2 t0, t1, t2, t3;
						t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
						t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

						if(pSprite->m_Rotate)
							Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
						else
							Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);
					}

					IGraphics::CFreeformItem FreeFormItem(
							p0.x, p0.y,
							p1.x, p1.y,
							p2.x, p2.y,
							p3.x, p3.y
						);

					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					Graphics()->QuadsEnd();
					
					if (Screen)
						Graphics()->ShaderEnd();
					
				} break;
			}
		}
	}
}





void CRenderTools::RenderWalker(vec2 Pos, int Anim, float Time, int Dir, float Angle, int Status)
{
	vec2 Position = Pos;
	int Atlas = ATLAS_DROID_WALKER;
	
	if (Anim > 2)
		Atlas = ATLAS_DRONE;
	
	if (Anim < 0)
		Atlas = ATLAS_WALKER_BOTTOM;

	CAnimSkeletonInfo *pSkeleton = Skelebank()->m_lSkeletons[Atlas];
	CTextureAtlas *pAtlas = Skelebank()->m_lAtlases[Atlas];
		
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	vec2 Scale = vec2(1.0f, 1.0f) * 0.15f;
	
	if (Dir == 1)
		Scale.x *= -1;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, 0.0f);
	
	CSpineAnimation *pAnimation = 0x0;
	
	// ugly
	if (Anim == 0 || Anim == 3)
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("idle");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	else if (Anim < 0)
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("fall");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	else if (Anim < 3)
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("move");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	
	pSkeleton->UpdateBones(Time, pAnimation, NULL, Angle);
	

	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment

			// evaluate animations
			/*
			if(pAnimation)
			{
				CSpineSlotTimeline *pSlotTimeline = 0x0;

				// find timelines
				{
					auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
						pSlotTimeline = &SlotTimelineIter->second;	
				}
			}
			*/

			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);


					Graphics()->TextureSet(pPage->m_TexId);
					Graphics()->QuadsBegin();
					
					
					{
						vec2 t0, t1, t2, t3;
						t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
						t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

						if(pSprite->m_Rotate)
							Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
						else
							Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);
					}

					IGraphics::CFreeformItem FreeFormItem(
							p0.x, p0.y,
							p1.x, p1.y,
							p2.x, p2.y,
							p3.x, p3.y
						);

					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					Graphics()->QuadsEnd();
				} break;
			}
		}
	}
}


void CRenderTools::RenderStarDroid(vec2 Pos, int Anim, float Time, int Dir, float Angle, int Status, CDroidAnim *pDroidAnim)
{
	vec2 Position = Pos;
	int Atlas = ATLAS_DROID_STAR;

	CAnimSkeletonInfo *pSkeleton = Skelebank()->m_lSkeletons[Atlas];
	CTextureAtlas *pAtlas = Skelebank()->m_lAtlases[Atlas];
		
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	vec2 Scale = vec2(1.0f, 1.0f) * 0.6f;
	
	if (Dir == 1)
		Scale.x *= -1;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, 0.0f);
	
	CSpineAnimation *pAnimation = 0x0;
	
	// ugly
	if (Anim == 0)
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("idle");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	else if (Anim == 1)
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("shoot");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	
	pSkeleton->UpdateBones(Time, pAnimation, NULL, Angle, pDroidAnim);
	

	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment


			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);


					int SybsetType = 0;
					
					if (pAttachment->m_SpecialType == AST_JET1 || pAttachment->m_SpecialType == AST_JET2 || pAttachment->m_SpecialType == AST_JET)
					{
						if (Status != DROIDSTATUS_TERMINATED)
						{
							Graphics()->TextureSet(g_pData->m_aImages[IMAGE_JET].m_Id);
							SybsetType = 2;
							
							vec3 p = (p0+p1+p2+p3)/4.0f;
							
							
							if (pAttachment->m_SpecialType == AST_JET1)
							{
								pDroidAnim->m_aVectorValue[CDroidAnim::THRUST1_POS] = vec2(p.x, p.y);
								pDroidAnim->m_aVectorValue[CDroidAnim::THRUST1_VEL] = normalize(vec2(p1.x-p0.x, p1.y-p0.y));
							}
							else if (pAttachment->m_SpecialType == AST_JET2)
							{
								pDroidAnim->m_aVectorValue[CDroidAnim::THRUST2_POS] = vec2(p.x, p.y);
								pDroidAnim->m_aVectorValue[CDroidAnim::THRUST2_VEL] = normalize(vec2(p1.x-p0.x, p1.y-p0.y));
							}
						}
					}
					else
						Graphics()->TextureSet(pPage->m_TexId);

					Graphics()->QuadsBegin();
					
					
					if (SybsetType == 1)
					{
						Graphics()->QuadsSetSubsetFree(0, 0,1, 0, 0, 1, 1, 1);
					}
					else if (SybsetType == 2)
					{
						int i = rand()%3;
						float t0 = i / 4.0f;
						float t1 = (i+1) / 4.0f;
						Graphics()->QuadsSetSubsetFree(t0, 0, t1, 0, t0, 1, t1, 1);
					}
					else
					{
						vec2 t0, t1, t2, t3;
						t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
						t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

						if(pSprite->m_Rotate)
							Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
						else
							Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);
					}

					IGraphics::CFreeformItem FreeFormItem(
							p0.x, p0.y,
							p1.x, p1.y,
							p2.x, p2.y,
							p3.x, p3.y
						);

					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					Graphics()->QuadsEnd();
				} break;
			}
		}
	}
}



void CRenderTools::RenderCrawlerDroid(vec2 Pos, int Anim, float Time, int Dir, float Angle, int Status, CDroidAnim *pDroidAnim, bool Render)
{
	vec2 Position = Pos;
	int Atlas = ATLAS_DROID_CRAWLER;

	CAnimSkeletonInfo *pSkeleton = Skelebank()->m_lSkeletons[Atlas];
	CTextureAtlas *pAtlas = Skelebank()->m_lAtlases[Atlas];
		
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	vec2 Scale = vec2(1.0f, 1.0f) * 0.4f;
	
	if (Dir == 1)
		Scale.x *= -1;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, Angle);
	
	CSpineAnimation *pAnimation = 0x0;
	
	// ugly
	if (Anim == 0)
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("idle");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	
	pSkeleton->UpdateBones(Time, pAnimation, NULL, Angle, pDroidAnim);
	

	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment

			// evaluate animations
			/*
			if(pAnimation)
			{
				CSpineSlotTimeline *pSlotTimeline = 0x0;

				// find timelines
				{
					auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
						pSlotTimeline = &SlotTimelineIter->second;	
				}
			}
			*/

			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);


					int SybsetType = 0;
					
					vec3 p = (p0+p1+p2+p3)/4.0f;
					
					if (strcmp(pSlot->m_Name, "body_attach_1") == 0)
						pDroidAnim->m_aVectorValue[CDroidAnim::ATTACH1_POS] = vec2(p.x, p.y);
						
					if (strcmp(pSlot->m_Name, "body_attach_2") == 0)
						pDroidAnim->m_aVectorValue[CDroidAnim::ATTACH2_POS] = vec2(p.x, p.y);
					
					
					if (Render)
					{
						Graphics()->TextureSet(pPage->m_TexId);
						Graphics()->QuadsBegin();
						
						Graphics()->SetColor(1, 1, 1, 1);
						
						if (SybsetType == 1)
						{
							Graphics()->QuadsSetSubsetFree(0, 0,1, 0, 0, 1, 1, 1);
						}
						else if (SybsetType == 2)
						{
							int i = rand()%3;
							float t0 = i / 4.0f;
							float t1 = (i+1) / 4.0f;
							Graphics()->QuadsSetSubsetFree(t0, 0, t1, 0, t0, 1, t1, 1);
						}
						else
						{
							vec2 t0, t1, t2, t3;
							t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
							t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
							t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
							t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

							if(pSprite->m_Rotate)
								Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
							else
								Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);
						}

						IGraphics::CFreeformItem FreeFormItem(
								p0.x, p0.y,
								p1.x, p1.y,
								p2.x, p2.y,
								p3.x, p3.y
							);

						Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
						Graphics()->QuadsEnd();
					}
				} break;
			}
		}
	}
}


void CRenderTools::RenderCrawlerLegs(CDroidAnim *pDroidAnim)
{
	//vec2 Pos = pDroidAnim->m_Pos;
	int Dir = pDroidAnim->m_Dir;
	const vec2 Offset = vec2(0, -32);
	
	if (abs(pDroidAnim->m_aVectorValue[CDroidAnim::ATTACH1_POS].x - pDroidAnim->m_aLegPos[0].x) > 300 || abs(pDroidAnim->m_aVectorValue[CDroidAnim::ATTACH1_POS].y - pDroidAnim->m_aLegPos[0].y) > 300)
		return;	
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CRAWLER_LEG2].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1, 1, 1, 1);
	
	for (int i = 0; i < 4; i++)
	{
		vec2 LegPos = pDroidAnim->m_aLegPos[i]+Offset;

		vec2 p;

		if ((i < 2 && Dir > 0) || (i >= 2 && Dir < 0))
			p = pDroidAnim->m_aVectorValue[CDroidAnim::ATTACH1_POS];
		else
			p = pDroidAnim->m_aVectorValue[CDroidAnim::ATTACH2_POS];
		
		float a = GetAngle(normalize(LegPos - p));
		
		float a1 = a-pi/2.0f;
		float a2 = a-pi/2.0f;
		float a3 = a+pi/2.0f;
		float a4 = a+pi/2.0f;

		float s1 = 4.0f;

		vec2 p1 = LegPos+vec2(cos(a1), sin(a1))*s1;
		vec2 p2 = p+vec2(cos(a2), sin(a2))*s1;
		vec2 p3 = LegPos+vec2(cos(a3), sin(a3))*s1;
		vec2 p4 = p+vec2(cos(a4), sin(a4))*s1;
		
		Graphics()->QuadsSetSubsetFree(0, 0, 1, 0, 0, 1, 1, 1);
		
		IGraphics::CFreeformItem FreeFormItem(
			p1.x, p1.y,
			p2.x, p2.y,
			p3.x, p3.y,
			p4.x, p4.y);
							
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	
	// render leg tips
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CRAWLER_LEG1].m_Id);
	Graphics()->QuadsBegin();
		
	vec2 Size = vec2(64, 256)*0.5f;
		
	for (int i = 0; i < 4; i++)
	{
		/*
		if (i == 0 || i == 1)
		{
			if (pDroidAnim->m_Dir == 1 && pDroidAnim->m_Anim == DROIDANIM_JUMPATTACK)
				Graphics()->QuadsSetRotation(Angle+1.0f);
			else
				Graphics()->QuadsSetRotation(Angle-0.03f);
		}
		else
		{
			if (pDroidAnim->m_Dir == -1 && pDroidAnim->m_Anim == DROIDANIM_JUMPATTACK)
				Graphics()->QuadsSetRotation(Angle-1.0f);
			else
				Graphics()->QuadsSetRotation(Angle+0.03f);
		}
		*/
		
		Graphics()->QuadsSetRotation(pDroidAnim->m_aLegAngle[i]);
		
		SelectSprite(SPRITE_CRAWLER_LEG, i < 2 ? SPRITE_FLAG_FLIP_X : 0);
		
		vec2 p = pDroidAnim->m_aLegPos[i]+Offset;
	
		IGraphics::CQuadItem QuadItem(p.x, p.y, Size.x, Size.y);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
}



void CRenderTools::RenderStaticPlayer(CTeeRenderInfo *pInfo, vec2 Pos)
{
	vec2 Position = Pos;
	
	int Atlas = pInfo->m_Body;
	if (Atlas < 0)
		Atlas = 0;
	
	if (Atlas > NUM_BODIES-1)
		Atlas = NUM_BODIES-1;


	
	RenderSkeleton(Position+vec2(0, 16), 
		pInfo, 
		CPlayerInfo::GetIdle()->Animation(), 
		0, 
		Skelebank()->m_lSkeletons[Atlas], 
		Skelebank()->m_lAtlases[Atlas], 
		CPlayerInfo::GetIdle());

	
	// render hand
	float HandBaseSize = 15.0f;
		
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, 1);

	Graphics()->QuadsSetRotation(90);
		
	// two passes
	//for (int i = 0; i < 2; i++)
	{
		//bool OutLine = i == 0;

		//SelectSprite(OutLine?SPRITE_TEE_HAND_OUTLINE:SPRITE_TEE_HAND);
		SelectSprite(SPRITE_HAND1_1+pInfo->m_Body*4);
		IGraphics::CQuadItem QuadItem(Pos.x-10, Pos.y, HandBaseSize*2, HandBaseSize*2);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
		
	Graphics()->QuadsEnd();
}


void CRenderTools::RenderArm(CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, vec2 Pos, vec2 PlayerPos)
{
	// render arm
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0, 0, 0, 1);
				
	vec2 ArmPos = PlayerPos + PlayerInfo->m_ArmPos;

	vec2 p = Pos;
	
	float a = GetAngle(normalize(ArmPos - p));
	
	float a1 = a-pi/2.0f;
	float a2 = a-pi/2.0f;
	float a3 = a+pi/2.0f;
	float a4 = a+pi/2.0f;
	
	// outline
	{
		float s1 = 2.0f;

		vec2 p1 = ArmPos+vec2(cos(a1), sin(a1))*s1;
		vec2 p2 = p+vec2(cos(a2), sin(a2))*s1;
		vec2 p3 = ArmPos+vec2(cos(a3), sin(a3))*s1;
		vec2 p4 = p+vec2(cos(a4), sin(a4))*s1;
		
		IGraphics::CFreeformItem FreeFormItem(
			p1.x, p1.y,
			p2.x, p2.y,
			p3.x, p3.y,
			p4.x, p4.y);
							
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	// fill
	{
		if (pInfo->m_Body == 6)
			Graphics()->SetColor(pInfo->m_ColorBody.r/2.5f, pInfo->m_ColorBody.g/2.5f, pInfo->m_ColorBody.b/2.5f, pInfo->m_ColorBody.a);
		else
			Graphics()->SetColor(pInfo->m_ColorBody.r/1.5f, pInfo->m_ColorBody.g/1.5f, pInfo->m_ColorBody.b/1.5f, pInfo->m_ColorBody.a);
		
		float s1 = 1.0f;

		vec2 p1 = ArmPos+vec2(cos(a1), sin(a1))*s1;
		vec2 p2 = p+vec2(cos(a2), sin(a2))*s1;
		vec2 p3 = ArmPos+vec2(cos(a3), sin(a3))*s1;
		vec2 p4 = p+vec2(cos(a4), sin(a4))*s1;
		
		IGraphics::CFreeformItem FreeFormItem(
			p1.x, p1.y,
			p2.x, p2.y,
			p3.x, p3.y,
			p4.x, p4.y);
							
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	Graphics()->QuadsEnd();
}



void CRenderTools::RenderMelee(CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, vec2 Dir, vec2 Pos)
{
	if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_SPIN)
	{
		float WeaponAngle = PlayerInfo->MeleeAngle();
		int WeaponDir = PlayerInfo->MeleeFlip() ? -1 : 1;
		bool FlipY = false;
		vec2 Size = vec2(256, 256)*0.4f;
		
		if (WeaponDir > 0)
			FlipY = PlayerInfo->MeleeEffectFlip();
		else
			FlipY = !PlayerInfo->MeleeEffectFlip();
		
		vec2 WeaponPos = Pos + PlayerInfo->m_Weapon2Recoil;
		vec2 Offset = vec2(0, 0) + PlayerInfo->MeleeOffset();
		
		vec2 p = vec2(WeaponPos.x + Offset.x, WeaponPos.y + Offset.y);
		
		RenderArm(PlayerInfo, pInfo, WeaponPos + Offset, Pos);
		
		float WAngle = WeaponAngle;//*WeaponDir;
		int Flags = 0; //(FlipY ? SPRITE_FLAG_FLIP_Y : 0) + (WeaponDir < 0 ? SPRITE_FLAG_FLIP_X : 0);
		Flags = (FlipY ? SPRITE_FLAG_FLIP_Y : 0) ^ (WeaponDir < 0 ? SPRITE_FLAG_FLIP_Y : 0);
		
		float Alpha2 = 1.0f;
		
		SetShadersForWeapon(PlayerInfo);
		
		// "shadow"
		float Speed = PlayerInfo->MeleeSpeed();
		
		if (abs(Speed) > 0.3f)
		{
			float WAngle2 = WAngle - Speed*0.8f;
			RenderWeapon(PlayerInfo->m_Weapon, p, vec2(cos(WAngle2), sin(WAngle2))*WeaponDir, WEAPON_GAME_SIZE, true, Flags, 0.25f);
			
			WAngle2 = WAngle - Speed*0.4f;
			RenderWeapon(PlayerInfo->m_Weapon, p, vec2(cos(WAngle2), sin(WAngle2))*WeaponDir, WEAPON_GAME_SIZE, true, Flags, 0.5f);
		}
		
		RenderWeapon(PlayerInfo->m_Weapon, p, vec2(cos(WAngle), sin(WAngle))*WeaponDir, WEAPON_GAME_SIZE, true, Flags, Alpha2);
		
		
		/*
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SCYTHE].m_Id);
		int Sprite = SPRITE_SCYTHE1+PlayerInfo->MeleeFrame();
			
		Graphics()->QuadsBegin();
		Graphics()->SetColor(0.1f, 1.0f, 0.1f, 1);
		
		Graphics()->QuadsSetRotation(WeaponAngle);
			
		{
			SelectSprite(Sprite, (FlipY ? SPRITE_FLAG_FLIP_Y : 0) + (WeaponDir < 0 ? SPRITE_FLAG_FLIP_X : 0));
			IGraphics::CQuadItem QuadItem(WeaponPos.x + Offset.x, WeaponPos.y + Offset.y, Size.x*PlayerInfo->MeleeSize(), Size.y*PlayerInfo->MeleeSize());
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
		
		Graphics()->QuadsEnd();
		*/
			
		// render hand
		float HandBaseSize = 16.0f;		
			
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

		Graphics()->QuadsSetRotation(WeaponAngle-pi/2);
		{
			SelectSprite(SPRITE_HAND1_1+pInfo->m_Body*4);
			IGraphics::CQuadItem QuadItem(WeaponPos.x+ Offset.x, WeaponPos.y+ Offset.y, 2*HandBaseSize, 2*HandBaseSize);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
			
		Graphics()->QuadsEnd();
	}
	
	
	else if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEE)
	{
		float WeaponAngle = pi/2.0f - abs(GetAngle(Dir)-pi/2.0f);
		
		vec2 WeaponPos = Pos + PlayerInfo->m_WeaponRecoil + PlayerInfo->m_Weapon2Recoil;
		WeaponPos.y -= 10;
		
		int WeaponDir = Dir.x < 0 ? -1 : 1;
		bool FlipY = false;

		vec2 Size = vec2(96, 32);
		float BladeLen = -28;
		float Radius = 20.0f;
		
		WeaponAngle -= PlayerInfo->m_Weapon2Recoil.y*0.05f;
		
		if (PlayerInfo->m_Weapon == WEAPON_TOOL)
		{
			WeaponAngle = 0;
			WeaponAngle -= 40*RAD;
			WeaponAngle += PlayerInfo->m_ToolAngleOffset*RAD;
			Size = vec2(64, 32) * 0.8f;
			BladeLen = -12;
			WeaponPos.y += 16;
			Radius = 8.0f;
		}
		else
		{
			if (PlayerInfo->m_MeleeState == MELEE_UP)
			{
				if (PlayerInfo->m_MeleeAnimState > 0.0f)
				{
					WeaponAngle += 140*RAD - min(140*2*RAD , PlayerInfo->m_MeleeAnimState*3.0f);
					FlipY = true;
				}
				else
				{
					WeaponAngle -= 140*RAD;
					FlipY = false;
				}
			}
			else
			{
				if (PlayerInfo->m_MeleeAnimState > 0.0f)
				{
					WeaponAngle -= 140*RAD - min(140*2*RAD , PlayerInfo->m_MeleeAnimState*3.0f);
					FlipY = false;
				}
				else
				{
					WeaponAngle += 140*RAD;
					FlipY = true;
				}
			}
		}
		
		vec2 Offset = vec2(0, 0);

		WeaponPos.x += sin(-WeaponAngle*WeaponDir+90*RAD)*Radius*WeaponDir;
		WeaponPos.y += cos(-WeaponAngle*WeaponDir+90*RAD)*Radius*WeaponDir;
		
		Offset.x = sin(-WeaponAngle*WeaponDir-90*RAD)*BladeLen*WeaponDir;
		Offset.y = cos(-WeaponAngle*WeaponDir-90*RAD)*BladeLen*WeaponDir;
	
		RenderArm(PlayerInfo, pInfo, WeaponPos, Pos);
		SetShadersForWeapon(PlayerInfo);
		
	
		vec2 p = vec2(WeaponPos.x + Offset.x, WeaponPos.y + Offset.y);
		
		// render hand
		float HandBaseSize = 15.0f;		
			
		//vec2 d = normalize(p - (Pos+vec2(0, -8)));
		vec2 d = normalize(p - (Pos+PlayerInfo->m_ArmPos));
	
		vec2 DirX = d;
		vec2 DirY(-d.y,d.x);

		if (d.x < 0)
			DirY = -DirY;

		Offset = GetWeaponRenderOffset(PlayerInfo->m_Weapon);
		
		vec2 HandPos = vec2(0, 0);
		HandPos += DirX * Offset.x;
		HandPos += DirY * Offset.y;
		
		int Flags = 0; //(FlipY ? 0 : SPRITE_FLAG_FLIP_Y) + (WeaponDir < 0 ? SPRITE_FLAG_FLIP_X : 0);
		Flags = (FlipY ? SPRITE_FLAG_FLIP_Y : 0) ^ (WeaponDir < 0 ? SPRITE_FLAG_FLIP_Y : 0);
	
		float WAngle = WeaponAngle*WeaponDir;
		
		float Alpha2 = 1.0f;
		
		if (PlayerInfo->m_MeleeAnimState > 0.0f)
			Alpha2 = PlayerInfo->m_MeleeAnimState * 0.2f;
		
		SetShadersForWeapon(PlayerInfo);
		RenderWeapon(PlayerInfo->m_Weapon, p + HandPos, vec2(cos(WAngle), sin(WAngle))*WeaponDir, WEAPON_GAME_SIZE, true, Flags, Alpha2);	
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

		float Angle = (WeaponAngle)*WeaponDir+pi/2;
		
		Graphics()->QuadsSetRotation(Angle);
		
		int Frame = (FlipY ? 3 : 0);
		
		WeaponPos.x += -sin(-WeaponAngle*WeaponDir+90*RAD)*2*WeaponDir;
		WeaponPos.y += -cos(-WeaponAngle*WeaponDir+90*RAD)*2*WeaponDir;
		
		if (FlipY)
		{
			//WeaponPos.x += 12;
			WeaponPos.x += -sin(-WeaponAngle*WeaponDir+0*RAD)*2;
			WeaponPos.y += -cos(-WeaponAngle*WeaponDir+0*RAD)*2;
		}
		
		{
			SelectSprite(SPRITE_HAND1_1+pInfo->m_Body*4+Frame, (WeaponDir < 0 && FlipY) ? SPRITE_FLAG_FLIP_X : 0);
			IGraphics::CQuadItem QuadItem(WeaponPos.x, WeaponPos.y, 2*HandBaseSize, 2*HandBaseSize);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
		
		Graphics()->QuadsSetRotation(0);
		Graphics()->QuadsEnd();
	}
	
	else if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEESMALL)
	{
		float WeaponAngle = pi/2.0f - abs(GetAngle(Dir)-pi/2.0f);
		
		vec2 WeaponPos = Pos + PlayerInfo->m_WeaponRecoil + PlayerInfo->m_Weapon2Recoil;
		WeaponPos.y -= 10;
		
		int WeaponDir = Dir.x < 0 ? -1 : 1;
		bool FlipY = false;

		vec2 Size = vec2(64, 32);
		float BladeLen = -28;
		float Radius = 20.0f;
		
		WeaponAngle -= PlayerInfo->m_Weapon2Recoil.y*0.05f;
		
		if (PlayerInfo->m_Weapon == WEAPON_TOOL)
		{
			WeaponAngle = 0;
			WeaponAngle -= 40*RAD;
			WeaponAngle += PlayerInfo->m_ToolAngleOffset*RAD;
			Size = vec2(64, 32) * 0.8f;
			BladeLen = -12;
			WeaponPos.y += 16;
			Radius = 8.0f;
		}
		else
		{
			if (PlayerInfo->m_MeleeState == MELEE_UP)
			{
				if (PlayerInfo->m_MeleeAnimState > 0.0f)
				{
					WeaponAngle += 140*RAD - min(140*2*RAD , PlayerInfo->m_MeleeAnimState*3.0f);
					FlipY = true;
				}
				else
				{
					WeaponAngle -= 140*RAD;
					FlipY = false;
				}
			}
			else
			{
				if (PlayerInfo->m_MeleeAnimState > 0.0f)
				{
					WeaponAngle -= 140*RAD - min(140*2*RAD , PlayerInfo->m_MeleeAnimState*3.0f);
					FlipY = false;
				}
				else
				{
					WeaponAngle += 140*RAD;
					FlipY = true;
				}
			}
		}
		
		vec2 Offset = vec2(0, 0);

		WeaponPos.x += sin(-WeaponAngle*WeaponDir+90*RAD)*Radius*WeaponDir;
		WeaponPos.y += cos(-WeaponAngle*WeaponDir+90*RAD)*Radius*WeaponDir;
		
		Offset.x = sin(-WeaponAngle*WeaponDir-90*RAD)*BladeLen*WeaponDir;
		Offset.y = cos(-WeaponAngle*WeaponDir-90*RAD)*BladeLen*WeaponDir;
	
		RenderArm(PlayerInfo, pInfo, WeaponPos, Pos);
		SetShadersForWeapon(PlayerInfo);
		
	
		vec2 p = vec2(WeaponPos.x + Offset.x, WeaponPos.y + Offset.y);
		
		// render hand
		float HandBaseSize = 15.0f;		
			
		//vec2 d = normalize(p - (Pos+vec2(0, -8)));
		vec2 d = normalize(p - (Pos+PlayerInfo->m_ArmPos));
	
		vec2 DirX = d;
		vec2 DirY(-d.y,d.x);

		if (d.x < 0)
			DirY = -DirY;

		Offset = GetWeaponRenderOffset(PlayerInfo->m_Weapon);
		
		vec2 HandPos = vec2(0, 0);
		HandPos += DirX * Offset.x;
		HandPos += DirY * Offset.y;
		
		int Flags = (FlipY ? 0 : SPRITE_FLAG_FLIP_Y) + (WeaponDir < 0 ? SPRITE_FLAG_FLIP_X : 0);
		Flags = (FlipY ? SPRITE_FLAG_FLIP_Y : 0) ^ (WeaponDir < 0 ? SPRITE_FLAG_FLIP_Y : 0);
	
		float WAngle = WeaponAngle*WeaponDir;
		
		float Alpha2 = 1.0f;
		
		if (PlayerInfo->m_MeleeAnimState > 0.0f)
			Alpha2 = PlayerInfo->m_MeleeAnimState * 0.2f;
		
		SetShadersForWeapon(PlayerInfo);
		RenderWeapon(PlayerInfo->m_Weapon, p + HandPos, vec2(cos(WAngle), sin(WAngle))*WeaponDir, WEAPON_GAME_SIZE, true, Flags, Alpha2);	
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

		float Angle = (WeaponAngle)*WeaponDir+pi/2;
		
		Graphics()->QuadsSetRotation(Angle);
		
		int Frame = (FlipY ? 3 : 0);
		
		WeaponPos.x += -sin(-WeaponAngle*WeaponDir+90*RAD)*2*WeaponDir;
		WeaponPos.y += -cos(-WeaponAngle*WeaponDir+90*RAD)*2*WeaponDir;
		
		if (FlipY)
		{
			//WeaponPos.x += 12;
			WeaponPos.x += -sin(-WeaponAngle*WeaponDir+0*RAD)*2;
			WeaponPos.y += -cos(-WeaponAngle*WeaponDir+0*RAD)*2;
		}
		
		{
			SelectSprite(SPRITE_HAND1_1+pInfo->m_Body*4+Frame, (WeaponDir < 0 && FlipY) ? SPRITE_FLAG_FLIP_X : 0);
			IGraphics::CQuadItem QuadItem(WeaponPos.x, WeaponPos.y, 2*HandBaseSize, 2*HandBaseSize);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
		
		Graphics()->QuadsSetRotation(0);
		Graphics()->QuadsEnd();
	}
}

void CRenderTools::SetShadersForPlayer(const CPlayerInfo *pCustomPlayerInfo)
{
	float Visibility = max(pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING], pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY]);
	float Electro = pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE];
	float Damage = pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE];
	float Deathray = pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY];
	
	Graphics()->PlayerShaderBegin(0.0f, 0.0f, 0.0f, 1.0f - Visibility, Electro, Damage, Deathray);
}

void CRenderTools::SetShadersForWeapon(CPlayerInfo *pCustomPlayerInfo)
{
	float ChargeLevel = 0;
	if (GetWeaponFiringType(pCustomPlayerInfo->m_Weapon) == WFT_THROW)
		ChargeLevel = pCustomPlayerInfo->GetWeaponCharge();
	else if (GetWeaponFiringType(pCustomPlayerInfo->m_Weapon) == WFT_CHARGE)
		ChargeLevel = pCustomPlayerInfo->ChargeIntensity();
	
	if (WeaponMaxLevel(pCustomPlayerInfo->m_Weapon) > 0)
		ChargeLevel = -GetWeaponCharge(pCustomPlayerInfo->m_Weapon) / float(WeaponMaxLevel(pCustomPlayerInfo->m_Weapon));
	
	float Visibility = max(pCustomPlayerInfo->m_EffectIntensity[EFFECT_SPAWNING], pCustomPlayerInfo->m_EffectIntensity[EFFECT_INVISIBILITY]);
	float Electro = pCustomPlayerInfo->m_EffectIntensity[EFFECT_ELECTRODAMAGE];
	float Damage = pCustomPlayerInfo->m_EffectIntensity[EFFECT_DAMAGE];
	float Deathray = pCustomPlayerInfo->m_EffectIntensity[EFFECT_DEATHRAY];
	
	// dont flash shuriken
	if (IsStaticWeapon(pCustomPlayerInfo->m_Weapon) && GetStaticType(pCustomPlayerInfo->m_Weapon) == SW_SHURIKEN)
		ChargeLevel = 0;
	
	SetShadersForWeapon(pCustomPlayerInfo->m_Weapon, ChargeLevel, 1.0f - Visibility, Electro, Damage, Deathray);
}


void CRenderTools::SetShadersForWeapon(int Weapon, float Charge, float Visibility, float Electro, float Damage, float Deathray)
{
	vec2 ColorSwap = GetWeaponColorswap(Weapon);
	
	if (WeaponMaxLevel(Weapon) > 0)
		Charge = -GetWeaponCharge(Weapon) / float(WeaponMaxLevel(Weapon));
	
	Graphics()->PlayerShaderBegin(ColorSwap.x, ColorSwap.y, Charge, Visibility, Electro, Damage, Deathray);
}



void CRenderTools::RenderForegroundHand(CPlayerInfo *PlayerInfo)
{
	if (!PlayerInfo->m_InUse || !PlayerInfo->m_Hang)
		return;
	
	bool FlipY = false;
	vec2 p = PlayerInfo->m_FGHandPos + PlayerInfo->HandOffset(HAND_FREE);
	
	// render hand
	float HandBaseSize = 16.0f;		
		
	//vec2 d = normalize(p - (Pos+vec2(0, -8)));
	vec2 d = normalize(p - (PlayerInfo->m_FGHandPos+PlayerInfo->m_ArmPos));
	
	
	if (d.x < 0)
		FlipY = true;
	
	SetShadersForPlayer(PlayerInfo);
	
	// hand
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(PlayerInfo->m_RenderInfo.m_ColorSkin.r, PlayerInfo->m_RenderInfo.m_ColorSkin.g, PlayerInfo->m_RenderInfo.m_ColorSkin.b, PlayerInfo->m_RenderInfo.m_ColorSkin.a);

		
	Graphics()->QuadsSetRotation(GetAngle(d));
	{
		SelectSprite(SPRITE_HAND1_1+PlayerInfo->m_RenderInfo.m_Body*4, (FlipY ? SPRITE_FLAG_FLIP_Y : 0));
		IGraphics::CQuadItem QuadItem(p.x, p.y, 2*HandBaseSize, 2*HandBaseSize);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
		
	Graphics()->QuadsEnd();
	Graphics()->ShaderEnd();
}


void CRenderTools::RenderFreeHand(CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, int Hand, vec2 Dir, vec2 Pos, bool Behind)
{
	bool FlipY = false;
	vec2 p = Pos + PlayerInfo->HandOffset(Hand);
	
	// hax
	PlayerInfo->m_FGHandPos = Pos;
	
	// render hand
	float HandBaseSize = 15.0f;		
		
	//vec2 d = normalize(p - (Pos+vec2(0, -8)));
	vec2 d = normalize(p - (Pos+PlayerInfo->m_ArmPos));
	
	
	if (d.x < 0)
		FlipY = true;
	
	//Graphics()->ShaderEnd();
	
	// render arm / RenderArm
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PIXEL].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0, 0, 0, 1);
				
	vec2 ArmPos = Pos + PlayerInfo->m_ArmPos;

	float a = GetAngle(normalize(ArmPos - p));
	
	float a1 = a-pi/2.0f;
	float a2 = a-pi/2.0f;
	float a3 = a+pi/2.0f;
	float a4 = a+pi/2.0f;
	
	// outline
	{
		float s1 = 4.0f;

		vec2 p1 = ArmPos+vec2(cos(a1), sin(a1))*s1;
		vec2 p2 = p+vec2(cos(a2), sin(a2))*s1;
		vec2 p3 = ArmPos+vec2(cos(a3), sin(a3))*s1;
		vec2 p4 = p+vec2(cos(a4), sin(a4))*s1;
		
		IGraphics::CFreeformItem FreeFormItem(
			p1.x, p1.y,
			p2.x, p2.y,
			p3.x, p3.y,
			p4.x, p4.y);
							
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	// fill
	{
		if (pInfo->m_Body == 6)
			Graphics()->SetColor(pInfo->m_ColorBody.r/2.5f, pInfo->m_ColorBody.g/2.5f, pInfo->m_ColorBody.b/2.5f, pInfo->m_ColorBody.a);
		else
			Graphics()->SetColor(pInfo->m_ColorBody.r/1.5f, pInfo->m_ColorBody.g/1.5f, pInfo->m_ColorBody.b/1.5f, pInfo->m_ColorBody.a);
		
		float s1 = 2.0f;

		vec2 p1 = ArmPos+vec2(cos(a1), sin(a1))*s1;
		vec2 p2 = p+vec2(cos(a2), sin(a2))*s1;
		vec2 p3 = ArmPos+vec2(cos(a3), sin(a3))*s1;
		vec2 p4 = p+vec2(cos(a4), sin(a4))*s1;
		
		IGraphics::CFreeformItem FreeFormItem(
			p1.x, p1.y,
			p2.x, p2.y,
			p3.x, p3.y,
			p4.x, p4.y);
							
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	Graphics()->QuadsEnd();
	
	// turbo effect to hand
	if (PlayerInfo->m_Turbo && Hand == HAND_FREE)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_JETPACK].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1, 1, 1, 1);
		
		Graphics()->QuadsSetRotation(GetAngle(d));
		{
			float s = 3*HandBaseSize/2+rand()%8;
			SelectSprite(SPRITE_JETPACK1+rand()%2, (FlipY ? SPRITE_FLAG_FLIP_Y : 0));
			IGraphics::CQuadItem QuadItem(p.x+d.x*s, p.y+d.y*s, s*2, s*2);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
			
		Graphics()->QuadsEnd();
	}
	
	
	if (Hand == HAND_WEAPON && (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_SPIN || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEESMALL || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEE || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_ITEM1 || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_WEAPON2))
	{
		vec2 DirX = d;
		vec2 DirY(-d.y,d.x);

		if (d.x < 0)
			DirY = -DirY;

		vec2 Offset = GetWeaponRenderOffset(PlayerInfo->m_Weapon);
		
		vec2 HandPos = vec2(0, 0);
		HandPos += DirX * Offset.x;
		HandPos += DirY * Offset.y;
		
		int Flags = 0;
		
		//if ((d.x < 0 && Dir.x > 0) || (d.x > 0 && Dir.x < 0))
		//	Flags = SPRITE_FLAG_FLIP_Y;
		
		PlayerInfo->m_MuzzlePos = p+HandPos;
		PlayerInfo->m_MuzzleDir = d;
		
		vec2 wd = d;
		
		// spinning effect for shuriken
		if (IsStaticWeapon(PlayerInfo->m_Weapon) && GetStaticType(PlayerInfo->m_Weapon) == SW_SHURIKEN)
			wd = GetDirection(GetAngle(d) + PlayerInfo->m_SpinningAngle * (d.x > 0.0f ? -1 : 1)); //  * PlayerInfo->GetWeaponCharge()
		
		SetShadersForWeapon(PlayerInfo);
		RenderWeapon(PlayerInfo->m_Weapon, p+HandPos, wd, WEAPON_GAME_SIZE, true, Flags);
		SetShadersForPlayer(PlayerInfo);
	}
	
	// hand
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HANDS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

	
	int Frame = SPRITE_HAND1_1+PlayerInfo->HandFrame(Hand);
	
	if (Frame == SPRITE_HAND1_1 && Behind)
		Frame++;
	
	Graphics()->QuadsSetRotation(GetAngle(d));
	{
		SelectSprite(Frame+pInfo->m_Body*4, (FlipY ? SPRITE_FLAG_FLIP_Y : 0));
		IGraphics::CQuadItem QuadItem(p.x, p.y, 2*HandBaseSize, 2*HandBaseSize);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
		
	Graphics()->QuadsEnd();
}



void CRenderTools::RenderWeapon(int Weapon, vec2 Pos, vec2 Dir, float Size, bool BeginQuads, int Flags, float Alpha2, bool KillMessage, bool NoFlags)
{
	//Pos.x -= Size / 4;
	
	if (!IsWeapon(Weapon))
		return;
	
	if (KillMessage && IsStaticWeapon(Weapon) && GetStaticType(Weapon) == SW_TOOL)
		return;
	
	ivec2 WSize = GetWeaponVisualSize(Weapon);
	
	//Size *= GetWeaponVisualSize(Weapon);
	Size = int(Size);
	
	// static / non-modular weapons
	if (IsStaticWeapon(Weapon))
	{
		if (BeginQuads)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->QuadsSetRotation(GetAngle(Dir));
			Graphics()->SetColor(1, 1, 1, 1);
		}
	
		SelectSprite(SPRITE_WEAPON_STATIC1+GetStaticType(Weapon), NoFlags ? Flags : ((Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0) ^ Flags));
		//DrawSprite(Pos.x, Pos.y, Size);
		IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size*WSize.x, Size*WSize.y);
		Graphics()->QuadsDraw(&QuadItem, 1);
		
		if (BeginQuads)
			Graphics()->QuadsEnd();
		
		return;
	}
	
	
	// modular weapons
	int Part1 = GetPart(Weapon, 0)-1;
	int Part2 = GetPart(Weapon, 1)-1;
	
	//if (Part1 < 0 || Part2 < 0)
	//	return;
	
	ivec2 WSize2 = GetWeaponVisualSize2(Weapon);

	if (BeginQuads)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(GetAngle(Dir));
		Graphics()->SetColor(1, 1, 1, 1);
	}
	
	if (GetWeaponRenderType(Weapon) == WRT_SPIN || GetWeaponRenderType(Weapon) == WRT_MELEE)
	{
		if (GetWeaponRenderType(Weapon) == WRT_MELEE)
			Pos -= Dir * Size * (WSize.x)/2;
		
		// back outlines
		if (Part1 >= 0)
		{
			Graphics()->SetColor(1, 1, 1, 1);
			SelectSprite(SPRITE_WEAPON_PART1_BG_0+Part1, Flags);
			//DrawSprite(Pos.x, Pos.y, Size);
			IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size*WSize.x, Size*WSize.y);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
		
		// back blade
		if (Part1 == 5)
		{
			Graphics()->QuadsSetRotation(GetAngle(Dir)+pi);
			Graphics()->SetColor(1, 1, 1, Alpha2);
			SelectSprite(SPRITE_WEAPON_PART2_0+Part2, Flags);
			IGraphics::CQuadItem QuadItem2(Pos.x-Dir.x*Size*7/2, Pos.y-Dir.y*Size*7/2, Size*WSize2.x, Size*WSize2.y);
			Graphics()->QuadsDraw(&QuadItem2, 1);
			Graphics()->QuadsSetRotation(GetAngle(Dir));
		}
		
		
		// front / blade
		if (Part2 >= 0)
		{
			Graphics()->SetColor(1, 1, 1, Alpha2);
			SelectSprite(SPRITE_WEAPON_PART2_0+Part2, Flags);
			IGraphics::CQuadItem QuadItem2(Pos.x+Dir.x*Size*7/2, Pos.y+Dir.y*Size*7/2, Size*WSize2.x, Size*WSize2.y);
			Graphics()->QuadsDraw(&QuadItem2, 1);
		}
	
		// back
		if (Part1 >= 0)
		{
			Graphics()->SetColor(1, 1, 1, 1);
			SelectSprite(SPRITE_WEAPON_PART1_0+Part1, Flags);
			//DrawSprite(Pos.x, Pos.y, Size);
			IGraphics::CQuadItem QuadItem3(Pos.x, Pos.y, Size*WSize.x, Size*WSize.y);
			Graphics()->QuadsDraw(&QuadItem3, 1);
		}
	}
	else
	{
		Pos -= Dir * Size * (WSize.x)/4;
	
		// back outlines
		if (Part1 >= 0)
		{
			SelectSprite(SPRITE_WEAPON_PART1_BG_0+Part1, NoFlags ? Flags : (Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0));
			//DrawSprite(Pos.x, Pos.y, Size);
			IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size*WSize.x, Size*WSize.y);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
		
		// front
		if (Part2 >= 0)
		{
			SelectSprite(SPRITE_WEAPON_PART2_0+Part2, NoFlags ? Flags : (Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0));
			IGraphics::CQuadItem QuadItem2(Pos.x+Dir.x*Size*(WSize.x-1), Pos.y+Dir.y*Size*(WSize.x-1), Size*WSize2.x, Size*WSize2.y);
			Graphics()->QuadsDraw(&QuadItem2, 1);
		}
	
		// back
		if (Part1 >= 0)
		{
			SelectSprite(SPRITE_WEAPON_PART1_0+Part1, NoFlags ? Flags : (Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0));
			//DrawSprite(Pos.x, Pos.y, Size);
			IGraphics::CQuadItem QuadItem3(Pos.x, Pos.y, Size*WSize.x, Size*WSize.y);
			Graphics()->QuadsDraw(&QuadItem3, 1);
		}
	}
	
	if (BeginQuads)
		Graphics()->QuadsEnd();
	
	if (BeginQuads && KillMessage && IsTurret(Weapon))
	{
		Graphics()->ShaderEnd();
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(GetAngle(Dir));
		Graphics()->SetColor(1, 1, 1, 1);
		
		SelectSprite(SPRITE_WEAPON_TURRET, Dir.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
		IGraphics::CQuadItem QuadItem3(Pos.x, Pos.y, Size*WSize.x, Size*WSize.y);
		Graphics()->QuadsDraw(&QuadItem3, 1);
		Graphics()->QuadsEnd();
	}
}



void CRenderTools::RenderPlayer(CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, int WeaponNum, int Emote, vec2 Dir, vec2 Pos)
{
	if (!PlayerInfo)
		return;

	vec2 Position = Pos;
	
	PlayerInfo->Animation()->m_ColorBody = pInfo->m_ColorBody;
	PlayerInfo->Animation()->m_ColorFeet = pInfo->m_ColorFeet;

	
	switch (Emote)
	{
		case EMOTE_PAIN:
			PlayerInfo->Animation()->m_Eyes = 3;
			break;
		case EMOTE_HAPPY:
			PlayerInfo->Animation()->m_Eyes = 1;
			break;
		case EMOTE_ANGRY:
			PlayerInfo->Animation()->m_Eyes = 2;
			break;
		case EMOTE_BLINK:
			PlayerInfo->Animation()->m_Eyes = 4;
			break;
		default:
			PlayerInfo->Animation()->m_Eyes = 0;
			break;
	}
	
	int Atlas = pInfo->m_Body;
	if (Atlas < 0)
		Atlas = 0;
	
	if (Atlas > NUM_BODIES-1)
		Atlas = NUM_BODIES-1;
	

	SetShadersForPlayer(PlayerInfo);
	
	
	
	if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_SPIN)
	{
		if (PlayerInfo->m_Hang || !PlayerInfo->MeleeFront())
			RenderMelee(PlayerInfo, pInfo, Dir, Position);
		else
			RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position, true);
	}
	else if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEE || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEESMALL)
	{
		if (PlayerInfo->m_Hang)
			RenderMelee(PlayerInfo, pInfo, Dir, Position);
		else
			RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position, true);
	}
	else if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_ITEM1 || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_WEAPON2)
	{
		if (PlayerInfo->m_Hang)
			RenderFreeHand(PlayerInfo, pInfo, HAND_WEAPON, Dir, Position, true);
		else
			RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position, true);
	}
	else if (PlayerInfo->m_Weapon == WEAPON_NONE)
	{
		RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position, true);
	}
	else
		RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position, true);
	
	
	
	// main body
	SetShadersForPlayer(PlayerInfo);
	RenderSkeleton(Position+vec2(0, 16), pInfo, PlayerInfo->Animation(), 0, Skelebank()->m_lSkeletons[Atlas], Skelebank()->m_lAtlases[Atlas], PlayerInfo);
	SetShadersForPlayer(PlayerInfo);

	if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_SPIN)
	{
		if (!PlayerInfo->m_Hang && PlayerInfo->MeleeFront())
			RenderMelee(PlayerInfo, pInfo, Dir, Position);
		else
			RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position, true);
	}
	else if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEE || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_MELEESMALL)
	{
		if (!PlayerInfo->m_Hang)
			RenderMelee(PlayerInfo, pInfo, Dir, Position);
		else
			RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position);
	}
	else if (GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_ITEM1 || GetWeaponRenderType(PlayerInfo->m_Weapon) == WRT_WEAPON2)
	{
		if (!PlayerInfo->m_Hang)
			RenderFreeHand(PlayerInfo, pInfo, HAND_WEAPON, Dir, Position);
		else
			RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position);
	}
	else if (PlayerInfo->m_Weapon == WEAPON_NONE)
	{
		RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position);
	}
	else
		RenderFreeHand(PlayerInfo, pInfo, HAND_FREE, Dir, Position);
}


// render player character
void CRenderTools::RenderSkeleton(vec2 Position, const CTeeRenderInfo *pInfo, CSkeletonAnimation *AnimData, float Rotation, CAnimSkeletonInfo *pSkeleton, CTextureAtlas *pAtlas, CPlayerInfo *PlayerInfo)
{
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	vec2 Scale = AnimData->m_Scale;
	
	if (AnimData->m_Flip)
		Scale.x *= -1;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, Rotation);

	//float Time = pfnEval(0.0f, pUser);
	float Time = AnimData->m_Time;
	
	
	CSpineAnimation *pAnimation = 0x0;
	
	{
		auto AnimIter = pSkeleton->m_lAnimations.find(Anim[AnimData->m_Anim]);
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}

	pSkeleton->UpdateBones(Time, pAnimation, AnimData);

	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment

			// evaluate animations
			if(pAnimation)
			{
				//CSpineSlotTimeline *pSlotTimeline = 0x0;
				

				// find timelines
				{
					//auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					//if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
					//	pSlotTimeline = &SlotTimelineIter->second;	
				}
			}

			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					// TODO: range check!
					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					
					// for blinking eyes without changing sprite
					float HeightScale = 1.0f;
					if (AnimData->m_Eyes == 4 && pAttachment->m_SpecialType == AST_EYES)
						HeightScale = 0.4f;
					
					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f*HeightScale, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f*HeightScale, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f*HeightScale, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f*HeightScale, 1.0f);

					// TODO: slow! batch!
					
					int SybsetType = 0;
					
					// render some slots with user selected texture 
					if (pAttachment->m_SpecialType == AST_HAT)
					{
						Graphics()->TextureSet(pInfo->m_TopperTexture);
						SybsetType = 1;
					}
					else if (pAttachment->m_SpecialType == AST_MASK && (PlayerInfo && PlayerInfo->m_Mask > 0))
					{
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MASK1+PlayerInfo->m_Mask-1].m_Id);
						//Graphics()->TextureSet(g_pData->m_aImages[IMAGE_MASK1].m_Id);
						SybsetType = 1;
					}
					else if (pAttachment->m_SpecialType == AST_EYES)
					{
						Graphics()->TextureSet(pInfo->m_EyeTexture);
						SybsetType = 2;
					}
					else
						Graphics()->TextureSet(pPage->m_TexId);
					
					
					// turbo effect to feet
					if (((PlayerInfo && PlayerInfo->m_Jetpack) || AnimData->GetAnimation() == PANIM_SLIDEKICK) && pAttachment->m_SpecialType == AST_FOOT)
					{
						bool Kicking = AnimData->GetAnimation() == PANIM_SLIDEKICK;
						
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_JETPACK].m_Id);
						Graphics()->QuadsBegin();
						Graphics()->SetColor(1, 1, 1, 1);
						
						vec2 di = normalize(vec2(p2.x, p2.y) - vec2(p0.x, p0.y));
						vec3 tp = (p0+p1+p2+p3) / 4.0f;
						vec2 p = vec2(tp.x, tp.y);
						
						Graphics()->QuadsSetRotation(GetAngle(di));
						{
							float s = 3*16/2+rand()%8;
							if (Kicking)
								s *= 1.1f;
							SelectSprite(SPRITE_JETPACK1+rand()%2);
							IGraphics::CQuadItem QuadItem(p.x+di.x*s, p.y+di.y*s, s*2, s*2);
							Graphics()->QuadsDraw(&QuadItem, 1);
						}
							
						Graphics()->QuadsEnd();
						
						Graphics()->TextureSet(pPage->m_TexId);
					}
					
					
					if (pAttachment->m_SpecialType == AST_BODY)
					{
						// bomb for cs / reactor defense
						if (PlayerInfo && PlayerInfo->m_BombCarrier)
						{
							Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BACKBOMB].m_Id);
							Graphics()->QuadsBegin();
							Graphics()->SetColor(1, 1, 1, 1);
							
							vec2 di = normalize(vec2(p0.x, p0.y) - vec2(p2.x, p2.y));
						
							di /= 2;
							vec3 tp = (p0+p1+p2+p3) / 4.0f;
							vec2 p = vec2(tp.x, tp.y);
							
							Graphics()->QuadsSetRotation(GetAngle(di)+pi/2);
							{
								float s = 32;
								IGraphics::CQuadItem QuadItem(p.x+di.x*s, p.y+di.y*s, s*2*(AnimData->m_Flip ? -1 : 1), s*2);
								Graphics()->QuadsDraw(&QuadItem, 1);
							}
								
							Graphics()->QuadsEnd();
							
							Graphics()->TextureSet(pPage->m_TexId);
						}
					}						

					
					Graphics()->QuadsBegin();
					
					
					if (pAttachment->m_SpecialType == AST_BODY)
						Graphics()->SetColor(pInfo->m_ColorBody.r, pInfo->m_ColorBody.g, pInfo->m_ColorBody.b, 1);
						
						
					else if (pAttachment->m_SpecialType == AST_HAT)
					{
						Graphics()->SetColor(pInfo->m_ColorTopper.r, pInfo->m_ColorTopper.g, pInfo->m_ColorTopper.b, 1);
					}
					
					else if (pAttachment->m_SpecialType == AST_HEAD)
					{
						Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, 1);
						
					}
					
					else if (pAttachment->m_SpecialType == AST_ARM)
					{
						vec3 p = (p0+p1+p2+p3) / 4.0f - vec3(Position.x, Position.y, 0);
						if (PlayerInfo)
							PlayerInfo->m_ArmPos = vec2(p.x, p.y+18);
					}
					
					else if (pAttachment->m_SpecialType == AST_FOOT)
						Graphics()->SetColor(pInfo->m_ColorFeet.r, pInfo->m_ColorFeet.g, pInfo->m_ColorFeet.b, 1);
					
					else if (pAttachment->m_SpecialType == AST_HAND)
					{
						if (PlayerInfo)
						{
							PlayerInfo->SetHandTarget(HAND_FREE, (p0+p1+p2+p3) / 4.0f - vec3(Position.x, Position.y, 0));
							PlayerInfo->SetHandTarget(HAND_WEAPON, vec3(0, -4, 0) + (p0+p1+p2+p3) / 4.0f - vec3(Position.x, Position.y, 0));
						}
						
						Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, 0);
					}
					
					if (PlayerInfo)
					{
						if (pAttachment->m_SpecialType == AST_SPLATTER1)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[0].r, PlayerInfo->m_aSplatterColor[0].g, PlayerInfo->m_aSplatterColor[0].b, PlayerInfo->m_aSplatter[0]);
						if (pAttachment->m_SpecialType == AST_SPLATTER2)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[1].r, PlayerInfo->m_aSplatterColor[1].g, PlayerInfo->m_aSplatterColor[1].b, PlayerInfo->m_aSplatter[1]);
						if (pAttachment->m_SpecialType == AST_SPLATTER3)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[2].r, PlayerInfo->m_aSplatterColor[2].g, PlayerInfo->m_aSplatterColor[2].b, PlayerInfo->m_aSplatter[2]);
						if (pAttachment->m_SpecialType == AST_SPLATTER4)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[3].r, PlayerInfo->m_aSplatterColor[3].g, PlayerInfo->m_aSplatterColor[3].b, PlayerInfo->m_aSplatter[3]);
						if (pAttachment->m_SpecialType == AST_SPLATTER5)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[4].r, PlayerInfo->m_aSplatterColor[4].g, PlayerInfo->m_aSplatterColor[4].b, PlayerInfo->m_aSplatter[4]);
						if (pAttachment->m_SpecialType == AST_SPLATTER6)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[5].r, PlayerInfo->m_aSplatterColor[5].g, PlayerInfo->m_aSplatterColor[5].b, PlayerInfo->m_aSplatter[5]);
						if (pAttachment->m_SpecialType == AST_SPLATTER7)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[6].r, PlayerInfo->m_aSplatterColor[6].g, PlayerInfo->m_aSplatterColor[6].b, PlayerInfo->m_aSplatter[6]);
						if (pAttachment->m_SpecialType == AST_SPLATTER8)
							Graphics()->SetColor(PlayerInfo->m_aSplatterColor[7].r, PlayerInfo->m_aSplatterColor[7].g, PlayerInfo->m_aSplatterColor[7].b, PlayerInfo->m_aSplatter[7]);
					}

					if (SybsetType == 1)
					{
						Graphics()->QuadsSetSubsetFree(0, 0,1, 0, 0, 1, 1, 1);
					}
					else if (SybsetType == 2)
					{
						float t0 = AnimData->m_Eyes / 4.0f;
						float t1 = (AnimData->m_Eyes+1) / 4.0f;
						Graphics()->QuadsSetSubsetFree(t0, 0, t1, 0, t0, 1, t1, 1);
					}
					else
					{
						vec2 t0, t1, t2, t3;
						t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
						t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
						t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

						if(pSprite->m_Rotate)
							Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
						else
							Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);
					}

					IGraphics::CFreeformItem FreeFormItem(
							p0.x, p0.y,
							p1.x, p1.y,
							p2.x, p2.y,
							p3.x, p3.y
						);

					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					Graphics()->QuadsEnd();
				} break;

			case ATTACHMENT_SKINNED_MESH:
				{
					CAnimAttachmentSkinnedMesh *pAttachment = (CAnimAttachmentSkinnedMesh *)pAttachmentBase;
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if (!pSprite)
						continue;

					// TODO: range check!
					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if (!pPage)
						continue;

					array<vec2> aVerticesWorld;
					for(int v = 0; v < pAttachment->m_aVertices.size(); v++)
					{
						CAnimAttachmentSkinnedMesh::SVertex const& rVertex = pAttachment->m_aVertices[v];
						vec3 p = vec3(0.0f, 0.0f, 1.0f);
						for(int b = 0; b < rVertex.m_aBones.size(); b++)
						{
							CAnimBone *pLocalBone = pSkeleton->m_lBones[rVertex.m_aBones[b]];
							float weight = rVertex.m_aWeights[b];
							vec3 vp = pLocalBone->m_Transform * vec3(rVertex.m_aPoints[b].x, rVertex.m_aPoints[b].y, 1.0f);
							p.x += vp.x * weight;
							p.y += vp.y * weight;
						}

						aVerticesWorld.add(vec2(p.x, p.y));
					}

					array<vec2> aUVs;
					for(int v = 0; v < pAttachment->m_aUVs.size(); v++)
					{
						vec2 uv;
						if(!pSprite->m_Rotate)
						{
							uv = vec2(
								(pSprite->m_X + pSprite->m_Width * pAttachment->m_aUVs[v].x) / pPage->m_Width,
								(pSprite->m_Y + pSprite->m_Height * pAttachment->m_aUVs[v].y) / pPage->m_Height);
						}
						else
						{
							uv = vec2(
								(pSprite->m_X + pSprite->m_Width * pAttachment->m_aUVs[v].y) / pPage->m_Width,
								(pSprite->m_Y + pSprite->m_Height - pSprite->m_Height * pAttachment->m_aUVs[v].x) / pPage->m_Height);
						}

						aUVs.add(uv);
					}

					Graphics()->TextureSet(pPage->m_TexId);
					Graphics()->TrianglesBegin();

					for(int i = 0; i < pAttachment->m_aTriangles.size(); i += 3)
					{
						IGraphics::CTriangleItem TriangleItem(
								aVerticesWorld[pAttachment->m_aTriangles[i]].x, aVerticesWorld[pAttachment->m_aTriangles[i]].y,
								aVerticesWorld[pAttachment->m_aTriangles[i+1]].x, aVerticesWorld[pAttachment->m_aTriangles[i+1]].y,
								aVerticesWorld[pAttachment->m_aTriangles[i+2]].x, aVerticesWorld[pAttachment->m_aTriangles[i+2]].y							
							);

						Graphics()->QuadsSetSubsetFree(
							aUVs[pAttachment->m_aTriangles[i]].x, aUVs[pAttachment->m_aTriangles[i]].y,
							aUVs[pAttachment->m_aTriangles[i+1]].x, aUVs[pAttachment->m_aTriangles[i+1]].y,
							aUVs[pAttachment->m_aTriangles[i+2]].x, aUVs[pAttachment->m_aTriangles[i+2]].y,
							0.0f, 0.0f);

						Graphics()->TrianglesDraw(&TriangleItem, 1);
					}

					Graphics()->TrianglesEnd();
					
					
					/*
					Graphics()->TextureSet(-1);
					Graphics()->LinesBegin();

					for(int l = 0; l < pAttachment->m_aEdges.size()-1; l++)
					{
						IGraphics::CLineItem LineItem(
								aVerticesWorld[pAttachment->m_aEdges[l]].x, aVerticesWorld[pAttachment->m_aEdges[l]].y,
								aVerticesWorld[pAttachment->m_aEdges[l+1]].x, aVerticesWorld[pAttachment->m_aEdges[l+1]].y								
							);

						Graphics()->LinesDraw(&LineItem, 1);
					}

					Graphics()->LinesEnd();
					*/

				} break;

			}
		}
	}

	/*
	Graphics()->TextureSet(-1);
	Graphics()->LinesBegin();
	Graphics()->SetColor(0.9f, 0.9f, 0.9f, 1.0f);

	// render bones, debug
	int NumBones = pSkeleton->m_lBones.size();
	for(int i = 0; i < NumBones; i++)
	{
		CAnimBone *pBone = pSkeleton->m_lBones[i];
		vec3 Start = vec3(Position.x, Position.y, 0.0f) + pBone->m_Transform * vec3(0.0f, 0.0f, 1.0f);
		vec3 End = vec3(Position.x, Position.y, 0.0f) + pBone->m_Transform * vec3(pBone->m_Length, 0.0f, 1.0f);

		vec3 Direction = End-Start;
		float Offset = 0.1f;
		vec2 Normal = vec2(-Direction.y, Direction.x) * 0.05;

		// draw bone
		IGraphics::CLineItem LineItems[] = {
			IGraphics::CLineItem(Start.x, Start.y, Start.x + Direction.x*Offset + Normal.x, Start.y + Direction.y*Offset + Normal.y),
			IGraphics::CLineItem(Start.x, Start.y, Start.x + Direction.x*Offset - Normal.x, Start.y + Direction.y*Offset - Normal.y),
			IGraphics::CLineItem(Start.x + Direction.x*Offset + Normal.x, Start.y + Direction.y*Offset + Normal.y, End.x, End.y),
			IGraphics::CLineItem(Start.x + Direction.x*Offset - Normal.x, Start.y + Direction.y*Offset - Normal.y, End.x, End.y)
		};
		Graphics()->LinesDraw(LineItems, 4);
	}

	Graphics()->LinesEnd();
	*/

}



void CRenderTools::RenderBuilding(vec2 Position, CAnimSkeletonInfo *pSkeleton, CTextureAtlas *pAtlas, int Team, int WeaponAngle)
{
	vec4 TeamColor = vec4(1.0f, 1.0f, 1.0f, 1);
	
	if (Team == TEAM_RED)
		TeamColor = vec4(1.0f, 0.3f, 0.0f, 1);
	else if (Team == TEAM_BLUE)
		TeamColor = vec4(0.0f, 0.3f, 1.0f, 1);
	
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	//vec2 Scale = vec2(0.5f, 0.5f);
	vec2 Scale = vec2(1.0f, 1.0f) * 0.15f;
		
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, 0.0f);

	float Time = 1.0f;
	
	CSpineAnimation *pAnimation = 0x0;
	
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("idle");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}

	pSkeleton->UpdateBones(Time, pAnimation, NULL, WeaponAngle);
	


	if(pAtlas)
	{
		int NumSlots = pSkeleton->m_lSlots.size();
		for(int i = 0; i < NumSlots; i++)
		{
			CAnimAttachmentSlot *pSlot = pSkeleton->m_lSlots[i];
			CAnimBone *pBone = pSlot->m_pBone;

			string Attachment = pSlot->m_AttachmentSetup; // default attachment

			// evaluate animations
			if(pAnimation)
			{
				//CSpineSlotTimeline *pSlotTimeline = 0x0;

				// find timelines
				{
					//auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					//if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
					//	pSlotTimeline = &SlotTimelineIter->second;	
				}
			}

			// find attachment
			auto SlotIter = pSkeleton->m_lSkins["default"].find(pSlot->m_Name);
			if (SlotIter == pSkeleton->m_lSkins["default"].end())
				continue;

			auto AttachmentIter = pSkeleton->m_lSkins["default"][pSlot->m_Name].find(Attachment);
			if (AttachmentIter == pSkeleton->m_lSkins["default"][pSlot->m_Name].end())
				continue;

			CAnimAttachment *pAttachmentBase = pSkeleton->m_lSkins["default"][pSlot->m_Name][Attachment];

			switch(pAttachmentBase->m_Type)
			{
			case ATTACHMENT_SPRITE:
				{
					CAnimAttachmentSprite *pAttachment = (CAnimAttachmentSprite *) pAttachmentBase;
					
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if(!pSprite)
						continue;

					// TODO: range check!
					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if(!pPage)
						continue;

					mat33 AttachmentParent = CalcTransformationMatrix(pAttachment->m_Position, pAttachment->m_Scale, pAttachment->m_Rotation);

					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f, 1.0f);

					// TODO: slow! batch!
					Graphics()->TextureSet(pPage->m_TexId);
					Graphics()->QuadsBegin();

					if (pAttachment->m_SpecialType == AST_1_TEAM || 
						pAttachment->m_SpecialType == AST_2_TEAM || 
						pAttachment->m_SpecialType == AST_3_TEAM || 
						pAttachment->m_SpecialType == AST_4_TEAM || 
						pAttachment->m_SpecialType == AST_WEAPON_TEAM)
						Graphics()->SetColor(TeamColor.r, TeamColor.g, TeamColor.b, TeamColor.a);
					
					
					vec2 t0, t1, t2, t3;
					t0 = vec2(pSprite->m_X / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
					t1 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, pSprite->m_Y / pPage->m_Height);
					t2 = vec2(pSprite->m_X / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);
					t3 = vec2((pSprite->m_X + pSprite->m_Width) / pPage->m_Width, (pSprite->m_Y + pSprite->m_Height) / pPage->m_Height);

					if(pSprite->m_Rotate)
						Graphics()->QuadsSetSubsetFree(t2.x, t2.y, t0.x, t0.y, t3.x, t3.y, t1.x, t1.y);
					else
						Graphics()->QuadsSetSubsetFree(t0.x, t0.y, t1.x, t1.y, t2.x, t2.y, t3.x, t3.y);

					IGraphics::CFreeformItem FreeFormItem(
							p0.x, p0.y,
							p1.x, p1.y,
							p2.x, p2.y,
							p3.x, p3.y
						);

					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
					Graphics()->QuadsEnd();
				} break;

			case ATTACHMENT_SKINNED_MESH:
				{
					CAnimAttachmentSkinnedMesh *pAttachment = (CAnimAttachmentSkinnedMesh *)pAttachmentBase;
					CTextureAtlasSprite *pSprite = &pAtlas->m_lSprites[pAttachment->m_Name];
					if (!pSprite)
						continue;

					// TODO: range check!
					CTextureAtlasPage *pPage = &pAtlas->m_lPages[pSprite->m_PageId];
					if (!pPage)
						continue;

					array<vec2> aVerticesWorld;
					for(int v = 0; v < pAttachment->m_aVertices.size(); v++)
					{
						CAnimAttachmentSkinnedMesh::SVertex const& rVertex = pAttachment->m_aVertices[v];
						vec3 p = vec3(0.0f, 0.0f, 1.0f);
						for(int b = 0; b < rVertex.m_aBones.size(); b++)
						{
							CAnimBone *pLocalBone = pSkeleton->m_lBones[rVertex.m_aBones[b]];
							float weight = rVertex.m_aWeights[b];
							vec3 vp = pLocalBone->m_Transform * vec3(rVertex.m_aPoints[b].x, rVertex.m_aPoints[b].y, 1.0f);
							p.x += vp.x * weight;
							p.y += vp.y * weight;
						}

						aVerticesWorld.add(vec2(p.x, p.y));
					}

					array<vec2> aUVs;
					for(int v = 0; v < pAttachment->m_aUVs.size(); v++)
					{
						vec2 uv;
						if(!pSprite->m_Rotate)
						{
							uv = vec2(
								(pSprite->m_X + pSprite->m_Width * pAttachment->m_aUVs[v].x) / pPage->m_Width,
								(pSprite->m_Y + pSprite->m_Height * pAttachment->m_aUVs[v].y) / pPage->m_Height);
						}
						else
						{
							uv = vec2(
								(pSprite->m_X + pSprite->m_Width * pAttachment->m_aUVs[v].y) / pPage->m_Width,
								(pSprite->m_Y + pSprite->m_Height - pSprite->m_Height * pAttachment->m_aUVs[v].x) / pPage->m_Height);
						}

						aUVs.add(uv);
					}

					Graphics()->TextureSet(pPage->m_TexId);
					Graphics()->TrianglesBegin();

					for(int i = 0; i < pAttachment->m_aTriangles.size(); i += 3)
					{
						IGraphics::CTriangleItem TriangleItem(
								aVerticesWorld[pAttachment->m_aTriangles[i]].x, aVerticesWorld[pAttachment->m_aTriangles[i]].y,
								aVerticesWorld[pAttachment->m_aTriangles[i+1]].x, aVerticesWorld[pAttachment->m_aTriangles[i+1]].y,
								aVerticesWorld[pAttachment->m_aTriangles[i+2]].x, aVerticesWorld[pAttachment->m_aTriangles[i+2]].y							
							);

						Graphics()->QuadsSetSubsetFree(
							aUVs[pAttachment->m_aTriangles[i]].x, aUVs[pAttachment->m_aTriangles[i]].y,
							aUVs[pAttachment->m_aTriangles[i+1]].x, aUVs[pAttachment->m_aTriangles[i+1]].y,
							aUVs[pAttachment->m_aTriangles[i+2]].x, aUVs[pAttachment->m_aTriangles[i+2]].y,
							0.0f, 0.0f);

						Graphics()->TrianglesDraw(&TriangleItem, 1);
					}

					Graphics()->TrianglesEnd();

				} break;

			}
		}
	}
}


static void CalcScreenParams(float Amount, float WMax, float HMax, float Aspect, float *w, float *h)
{
	float f = sqrtf(Amount) / sqrtf(Aspect);
	*w = f*Aspect;
	*h = f;

	// limit the view
	if(*w > WMax)
	{
		*w = WMax;
		*h = *w/Aspect;
	}

	if(*h > HMax)
	{
		*h = HMax;
		*w = *h*Aspect;
	}
}

void CRenderTools::MapscreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
	float OffsetX, float OffsetY, float Aspect, float Zoom, float *pPoints)
{
	float Width, Height;
	CalcScreenParams(1150*1000, 1500, 1050, Aspect, &Width, &Height);
	CenterX *= ParallaxX;
	CenterY *= ParallaxY;
	Width *= Zoom;
	Height *= Zoom;
	pPoints[0] = OffsetX+CenterX-Width/2;
	pPoints[1] = OffsetY+CenterY-Height/2;
	pPoints[2] = pPoints[0]+Width;
	pPoints[3] = pPoints[1]+Height;
}

void CRenderTools::RenderTilemapGenerateSkip(class CLayers *pLayers)
{

	for(int g = 0; g < pLayers->NumGroups(); g++)
	{
		CMapItemGroup *pGroup = pLayers->GetGroup(g);

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = pLayers->GetLayer(pGroup->m_StartLayer+l);

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTmap = (CMapItemLayerTilemap *)pLayer;
				CTile *pTiles = (CTile *)pLayers->Map()->GetData(pTmap->m_Data);
				for(int y = 0; y < pTmap->m_Height; y++)
				{
					for(int x = 1; x < pTmap->m_Width;)
					{
						int sx;
						for(sx = 1; x+sx < pTmap->m_Width && sx < 255; sx++)
						{
							if(pTiles[y*pTmap->m_Width+x+sx].m_Index)
								break;
						}

						pTiles[y*pTmap->m_Width+x].m_Skip = sx-1;
						x += sx;
					}
				}
			}
		}
	}
}



void CRenderTools::RenderLine(vec2 p1, vec2 p2, float Size, vec4 Color)
{
	//Graphics()->BlendNormal();
	//Graphics()->TextureSet(-1);
	//Graphics()->QuadsBegin();

	Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);

	vec2 t = normalize(p2 - p1);
	t = vec2(t.y, -t.x) * Size;
	
	IGraphics::CFreeformItem Freeform(
		p1.x-t.x, p1.y-t.y,
		p1.x+t.x, p1.y+t.y,
		p2.x-t.x, p2.y-t.y,
		p2.x+t.x, p2.y+t.y);
	Graphics()->QuadsDrawFreeform(&Freeform, 1);
		
	//Graphics()->QuadsEnd();
}