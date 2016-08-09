#include <math.h>
#include <base/math.h>
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

#include <game/client/customstuff/playerinfo.h>

static float gs_SpriteWScale;
static float gs_SpriteHScale;

#define RAD 0.017453292519943295769236907684886f



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

void CAnimSkeletonInfo::UpdateBones(float Time, CSpineAnimation *pAnimation, CSkeletonAnimation *pAnimData, int WeaponAngle)
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


			if (strcmp(pBone->m_Name, "weapon") == 0 || strcmp(pBone->m_Name, "weapon_team") == 0)
			{
				if (WeaponAngle > 180)
				{
					Scale.x *= -1;
					WeaponAngle = 360 - WeaponAngle;
				}
				Rotation = WeaponAngle*pi/180.0f;
			}
			
			if (pAnimData)
			{
				if (strcmp(pBone->m_Name, "body") == 0)
				{
					Rotation += pAnimData->m_BodyTilt;
				}
				
				if (strcmp(pBone->m_Name, "head") == 0)
				{
					Rotation += pAnimData->m_HeadTilt + pAnimData->m_HeadTiltCorrect;
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

void CAnimSkeletonInfo::UpdateBones(float Time1, float Time2, CSpineAnimation *pAnimation1, CSpineAnimation *pAnimation2, float Mix)
{
	// we assume, that the bones are ordered by their place in the skeleton hierarchy, important!
	for(int i = 0; i < m_lBones.size(); i++)
	{
		CAnimBone *pBone = m_lBones[i];

		vec2 Position = vec2(0.0f, 0.0f);
		vec2 Scale = vec2(1.0f, 1.0f);
		float Rotation = 0.0f;
		
		vec2 Position1 = vec2(0.0f, 0.0f);
		vec2 Scale1 = vec2(1.0f, 1.0f);
		float Rotation1 = 0.0f;
		
		vec2 Position2 = vec2(0.0f, 0.0f);
		vec2 Scale2 = vec2(1.0f, 1.0f);
		float Rotation2 = 0.0f;

		// animation 1
		CSpineBoneTimeline *pBoneTimeline = 0x0;
		{
			auto BoneTimelineIter = pAnimation1->m_lBoneTimeline.find(pBone->m_Name);
			if(BoneTimelineIter != pAnimation1->m_lBoneTimeline.end())
				pBoneTimeline = &BoneTimelineIter->second;
		}

		if(pBoneTimeline)
		{
			CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeTranslate>(pBoneTimeline->m_lTranslations.base_ptr(), pBoneTimeline->m_lTranslations.size(), Time1, (float *)&Position1);
			CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeScale>(pBoneTimeline->m_lScales.base_ptr(), pBoneTimeline->m_lScales.size(), Time1, (float *)&Scale1);
			CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeRotate>(pBoneTimeline->m_lRotations.base_ptr(), pBoneTimeline->m_lRotations.size(), Time1, (float *)&Rotation1);
		}

		Position1.y *= -1.0f;
		Rotation1 = (360.0f - Rotation1)*pi/180.0f;
		
		
		// animation 2
		pBoneTimeline = 0x0;
		{
			auto BoneTimelineIter = pAnimation2->m_lBoneTimeline.find(pBone->m_Name);
			if(BoneTimelineIter != pAnimation2->m_lBoneTimeline.end())
				pBoneTimeline = &BoneTimelineIter->second;
		}

		if(pBoneTimeline)
		{
			CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeTranslate>(pBoneTimeline->m_lTranslations.base_ptr(), pBoneTimeline->m_lTranslations.size(), Time2, (float *)&Position2);
			CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeScale>(pBoneTimeline->m_lScales.base_ptr(), pBoneTimeline->m_lScales.size(), Time2, (float *)&Scale2);
			CRenderTools::RenderEvalSkeletonAnim<CSpineBoneKeyframeRotate>(pBoneTimeline->m_lRotations.base_ptr(), pBoneTimeline->m_lRotations.size(), Time2, (float *)&Rotation2);
		}
		
		Position2.y *= -1.0f;
		Rotation2 = (360.0f - Rotation2)*pi/180.0f;
		
		Position += Position1*Mix + Position2*(1.0f-Mix);
		Scale += Scale1*Mix + Scale2*(1.0f-Mix);
		Rotation += Rotation1*Mix + Rotation2*(1.0f-Mix);
		
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



	
	

void CRenderTools::SelectSprite(CDataSprite *pSpr, int Flags, int sx, int sy)
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

	Graphics()->QuadsSetSubset(x1, y1, x2, y2);
}

void CRenderTools::SelectSprite(int Id, int Flags, int sx, int sy)
{
	if(Id < 0 || Id >= g_pData->m_NumSprites)
		return;
	SelectSprite(&g_pData->m_aSprites[Id], Flags, sx, sy);
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



void CRenderTools::RenderFullScreenLayer()
{
	Graphics()->MapScreen(0,0,Graphics()->ScreenWidth(),Graphics()->ScreenHeight());
	
	// render blood splatter to tiles
	Graphics()->RenderToTexture(RENDERBUFFER_TILES);
	Graphics()->TextureSet(-2, RENDERBUFFER_SPLATTER);
	
	Graphics()->BlendBuffer();

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(255, 255, 255, 1.0f);
	
	{
		IGraphics::CQuadItem QuadItem(Graphics()->ScreenWidth() / 2, Graphics()->ScreenHeight() / 2, Graphics()->ScreenWidth(), -Graphics()->ScreenHeight());
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	// render all to screen
	Graphics()->RenderToScreen();
		
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-2, RENDERBUFFER_TILES);

	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(255, 255, 255, 1.0f);
	
	{
		IGraphics::CQuadItem QuadItem(Graphics()->ScreenWidth() / 2, Graphics()->ScreenHeight() / 2, Graphics()->ScreenWidth(), -Graphics()->ScreenHeight());
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
	
	Graphics()->QuadsEnd();
}

/*
void CRenderTools::RenderTee(CAnimState *pAnim, CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos)
{	
	vec2 Direction = Dir;
	vec2 Position = Pos;

	//Graphics()->TextureSet(data->images[IMAGE_CHAR_DEFAULT].id);
	Graphics()->TextureSet(pInfo->m_Texture);

	
	// TODO: FIX ME
	Graphics()->QuadsBegin();
	//Graphics()->QuadsDraw(pos.x, pos.y-128, 128, 128);

	// first pass we draw the outline
	// second pass we draw the filling
	for(int p = 0; p < 2; p++)
	{
		int OutLine = p==0 ? 1 : 0;

		for(int f = 0; f < 2; f++)
		{
			float AnimScale = pInfo->m_Size * 1.0f/64.0f;
			float BaseSize = pInfo->m_Size;
			if(f == 1)
			{
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);

				// draw body
				Graphics()->SetColor(pInfo->m_ColorBody.r, pInfo->m_ColorBody.g, pInfo->m_ColorBody.b, pInfo->m_ColorBody.a);
				vec2 BodyPos = Position + vec2(pAnim->GetBody()->m_X, pAnim->GetBody()->m_Y)*AnimScale;
				SelectSprite(OutLine?SPRITE_TEE_BODY_OUTLINE:SPRITE_TEE_BODY, 0, 0, 0);
				IGraphics::CQuadItem QuadItem(BodyPos.x, BodyPos.y, BaseSize, BaseSize);
				Graphics()->QuadsDraw(&QuadItem, 1);
				
				// draw eyes
				if(p == 1)
				{
					switch (Emote)
					{
						case EMOTE_PAIN:
							SelectSprite(SPRITE_TEE_EYE_PAIN, 0, 0, 0);
							break;
						case EMOTE_HAPPY:
							SelectSprite(SPRITE_TEE_EYE_HAPPY, 0, 0, 0);
							break;
						case EMOTE_SURPRISE:
							SelectSprite(SPRITE_TEE_EYE_SURPRISE, 0, 0, 0);
							break;
						case EMOTE_ANGRY:
							SelectSprite(SPRITE_TEE_EYE_ANGRY, 0, 0, 0);
							break;
						default:
							SelectSprite(SPRITE_TEE_EYE_NORMAL, 0, 0, 0);
							break;
					}

					float EyeScale = BaseSize*0.40f;
					float h = Emote == EMOTE_BLINK ? BaseSize*0.15f : EyeScale;
					float EyeSeparation = (0.075f - 0.010f*absolute(Direction.x))*BaseSize;
					vec2 Offset = vec2(Direction.x*0.125f, -0.05f+Direction.y*0.10f)*BaseSize;
					IGraphics::CQuadItem Array[2] = {
						IGraphics::CQuadItem(BodyPos.x-EyeSeparation+Offset.x, BodyPos.y+Offset.y, EyeScale, h),
						IGraphics::CQuadItem(BodyPos.x+EyeSeparation+Offset.x, BodyPos.y+Offset.y, -EyeScale, h)};
					Graphics()->QuadsDraw(Array, 2);
				}
			}

			// draw feet
			CAnimKeyframe *pFoot = f ? pAnim->GetFrontFoot() : pAnim->GetBackFoot();

			float w = BaseSize;
			float h = BaseSize/2;

			Graphics()->QuadsSetRotation(pFoot->m_Angle*pi*2);

			bool Indicate = !pInfo->m_GotAirJump && g_Config.m_ClAirjumpindicator;
			float cs = 1.0f; // color scale

			if(OutLine)
				SelectSprite(SPRITE_TEE_FOOT_OUTLINE, 0, 0, 0);
			else
			{
				SelectSprite(SPRITE_TEE_FOOT, 0, 0, 0);
				if(Indicate)
					cs = 0.5f;
			}

			Graphics()->SetColor(pInfo->m_ColorFeet.r*cs, pInfo->m_ColorFeet.g*cs, pInfo->m_ColorFeet.b*cs, pInfo->m_ColorFeet.a);
			IGraphics::CQuadItem QuadItem(Position.x+pFoot->m_X*AnimScale, Position.y+pFoot->m_Y*AnimScale, w, h);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
	}

	Graphics()->QuadsEnd();
}
*/

vec3 CRenderTools::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}
 


void CRenderTools::RenderShield(vec2 Pos, vec2 Size, float State)
{
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SHIELD].m_Id);
	
	Graphics()->QuadsBegin();
	Graphics()->QuadsSetRotation(0);
	Graphics()->SetColor(1, 1, 1, 0.7f);
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

			// evaluate animations
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
					
					// skip all but these
					if (!strcmp(pAttachment->m_Name, "head") == 0 &&
						!strcmp(pAttachment->m_Name, "hat") == 0 &&
						!strcmp(pAttachment->m_Name, "eyes") == 0)
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
					if (strcmp(pAttachment->m_Name, "hat") == 0)
					{
						Graphics()->TextureSet(pInfo->m_TopperTexture);
						SybsetType = 1;
					}
					else if (strcmp(pAttachment->m_Name, "eyes") == 0)
					{
						Graphics()->TextureSet(pInfo->m_EyeTexture);
						SybsetType = 2;
					}
					else
						Graphics()->TextureSet(pPage->m_TexId);
					
					Graphics()->QuadsBegin();
					

					if (strcmp(pAttachment->m_Name, "hat") == 0)
						Graphics()->SetColor(pInfo->m_ColorTopper.r, pInfo->m_ColorTopper.g, pInfo->m_ColorTopper.b, 1);
					
					if (strcmp(pAttachment->m_Name, "head") == 0)
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








void CRenderTools::RenderMonster(vec2 Pos, float Time, int Dir, int Status)
{
	vec2 Position = Pos;
	int Atlas = ATLAS_MONSTER1;

	CAnimSkeletonInfo *pSkeleton = Skelebank()->m_lSkeletons[Atlas];
	CTextureAtlas *pAtlas = Skelebank()->m_lAtlases[Atlas];
		
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	vec2 Scale = vec2(1.0f, 1.0f) * 0.15f;
	
	if (Dir == 1)
		Scale.x *= -1;
	
	mat33 TransformationWorld = CalcTransformationMatrix(Position, Scale, 0.0f);
	
	CSpineAnimation *pAnimation = 0x0;
	
	{
		auto AnimIter = pSkeleton->m_lAnimations.find("move");
		if(AnimIter != pSkeleton->m_lAnimations.end())
			pAnimation = &AnimIter->second;
	}
	
	pSkeleton->UpdateBones(Time, pAnimation, NULL);
	

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
				CSpineSlotTimeline *pSlotTimeline = 0x0;

				// find timelines
				{
					auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
						pSlotTimeline = &SlotTimelineIter->second;	
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
					
					// skip
					if (Status == MONSTERSTATUS_IDLE && strcmp(pAttachment->m_Name, "hurteye1") == 0)
						continue;
					
					if (Status != MONSTERSTATUS_IDLE && strcmp(pAttachment->m_Name, "eye1") == 0)
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
	float HandBaseSize = 10.0f;
		
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HAND].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, 1);

	Graphics()->QuadsSetRotation(0);
		
	// two passes
	//for (int i = 0; i < 2; i++)
	{
		//bool OutLine = i == 0;

		//SelectSprite(OutLine?SPRITE_TEE_HAND_OUTLINE:SPRITE_TEE_HAND);
		SelectSprite(SPRITE_HAND);
		IGraphics::CQuadItem QuadItem(Pos.x-10, Pos.y, HandBaseSize*2, HandBaseSize*2);
		Graphics()->QuadsDraw(&QuadItem, 1);
	}
		
	Graphics()->QuadsEnd();
}


void CRenderTools::RenderPlayer(CPlayerInfo *PlayerInfo, CTeeRenderInfo *pInfo, int WeaponNum, int Emote, vec2 Dir, vec2 Pos)
{
	vec2 Direction = Dir;
	vec2 Position = Pos;

	if (!PlayerInfo)
		return;
	
	vec2 FeetOffset = PlayerInfo->m_FeetOffset;

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
	
	RenderSkeleton(Position+vec2(0, 16), pInfo, PlayerInfo->Animation(), 0, Skelebank()->m_lSkeletons[Atlas], Skelebank()->m_lAtlases[Atlas], PlayerInfo);
	

	
	// render melee weapon
	if (PlayerInfo->m_Weapon == WEAPON_HAMMER || PlayerInfo->m_Weapon == WEAPON_TOOL)
	{
		float WeaponAngle = pi/2.0f - abs(GetAngle(Dir)-pi/2.0f);
		
		vec2 WeaponPos = Position + PlayerInfo->m_Weapon2Recoil;
		WeaponPos.y -= 16;
		
		int WeaponDir = Dir.x < 0 ? -1 : 1;
		bool FlipY = false;

		
		if (PlayerInfo->m_MeleeState == MELEE_UP)
		{
			WeaponAngle -= 140*RAD;
		}
		else
		{
			WeaponAngle += 140*RAD;
			FlipY = true;
		}
		
		
		WeaponPos.x += sin(-WeaponAngle*WeaponDir+90*RAD)*20*WeaponDir;
		WeaponPos.y += cos(-WeaponAngle*WeaponDir+90*RAD)*20*WeaponDir;
		
		
		vec2 Offset = vec2(0, 0);
		float BladeLen = -28;

		Offset.x = sin(-WeaponAngle*WeaponDir-90*RAD)*BladeLen*WeaponDir;
		Offset.y = cos(-WeaponAngle*WeaponDir-90*RAD)*BladeLen*WeaponDir;

		// render sword
		int Sprite = SPRITE_SWORD1 + int(PlayerInfo->m_MeleeAnimState);
			
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_SWORD].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(WeaponAngle*WeaponDir);
		
		SelectSprite(Sprite, (FlipY ? SPRITE_FLAG_FLIP_Y : 0) + (WeaponDir < 0 ? SPRITE_FLAG_FLIP_X : 0));
		IGraphics::CQuadItem QuadItem(WeaponPos.x + Offset.x, WeaponPos.y + Offset.y, 96, 32);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
		
		
		// render hand
		float HandBaseSize = 10.0f;		
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HAND].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, pInfo->m_ColorSkin.a);

		Graphics()->QuadsSetRotation(WeaponAngle*WeaponDir);
		
		// two passes
		//for (int i = 0; i < 2; i++)
		{
			//bool OutLine = i == 0;

			//SelectSprite(OutLine?SPRITE_TEE_HAND_OUTLINE:SPRITE_TEE_HAND, WeaponDir < 0 ? SPRITE_FLAG_FLIP_X : 0);
			SelectSprite(SPRITE_HAND);
			IGraphics::CQuadItem QuadItem(WeaponPos.x, WeaponPos.y, 2*HandBaseSize, 2*HandBaseSize);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
		
		Graphics()->QuadsSetRotation(0);
		Graphics()->QuadsEnd();
	}
}


void CRenderTools::RenderSkeleton(vec2 Position, CTeeRenderInfo *pInfo, CSkeletonAnimation *AnimData, float Rotation, CAnimSkeletonInfo *pSkeleton, CTextureAtlas *pAtlas, CPlayerInfo *PlayerInfo)
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
				CSpineSlotTimeline *pSlotTimeline = 0x0;
				

				// find timelines
				{
					auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
						pSlotTimeline = &SlotTimelineIter->second;	
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
					
					//if (AnimData->m_FlipBody && strcmp(pAttachment->m_Name, "body") == 0)
					//	Scale.x *= -1;
					
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
					if (AnimData->m_Eyes == 4 && strcmp(pAttachment->m_Name, "eyes") == 0)
						HeightScale = 0.4f;
					
					vec3 p0, p1, p2, p3;
					p0 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f*HeightScale, 1.0f);
					p1 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, -pAttachment->m_Height/2.0f*HeightScale, 1.0f);
					p2 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(-pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f*HeightScale, 1.0f);
					p3 = TransformationWorld * pBone->m_Transform * AttachmentParent * vec3(pAttachment->m_Width/2.0f, pAttachment->m_Height/2.0f*HeightScale, 1.0f);

					// TODO: slow! batch!
					
					int SybsetType = 0;
					
					// render some slots with user selected texture with 
					if (strcmp(pAttachment->m_Name, "hat") == 0)
					{
						Graphics()->TextureSet(pInfo->m_TopperTexture);
						SybsetType = 1;
					}
					else if (strcmp(pAttachment->m_Name, "eyes") == 0)
					{
						Graphics()->TextureSet(pInfo->m_EyeTexture);
						SybsetType = 2;
					}
					else
						Graphics()->TextureSet(pPage->m_TexId);
					
					Graphics()->QuadsBegin();
					
					if (strcmp(pAttachment->m_Name, "body") == 0)
						Graphics()->SetColor(pInfo->m_ColorBody.r, pInfo->m_ColorBody.g, pInfo->m_ColorBody.b, 1);
										

					if (strcmp(pAttachment->m_Name, "hat") == 0)
					{
						Graphics()->SetColor(pInfo->m_ColorTopper.r, pInfo->m_ColorTopper.g, pInfo->m_ColorTopper.b, 1);
					}
					
					if (strcmp(pAttachment->m_Name, "head") == 0)
					{
						Graphics()->SetColor(pInfo->m_ColorSkin.r, pInfo->m_ColorSkin.g, pInfo->m_ColorSkin.b, 1);
					}
					
					if (strcmp(pAttachment->m_Name, "foot") == 0)
						Graphics()->SetColor(pInfo->m_ColorFeet.r, pInfo->m_ColorFeet.g, pInfo->m_ColorFeet.b, 1);

					/*
					if (strcmp(pAttachment->m_Name, "head") == 0)
						Graphics()->SetColor(SkinColor*1.4f, SkinColor*1.2f, SkinColor, 1);
					*/
					
					if (PlayerInfo)
					{
						if (strcmp(pAttachment->m_Name, "splatter1") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[0]);
						if (strcmp(pAttachment->m_Name, "splatter2") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[1]);
						if (strcmp(pAttachment->m_Name, "splatter3") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[2]);
						if (strcmp(pAttachment->m_Name, "splatter4") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[3]);
						if (strcmp(pAttachment->m_Name, "splatter5") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[4]);
						if (strcmp(pAttachment->m_Name, "splatter6") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[5]);
						if (strcmp(pAttachment->m_Name, "splatter7") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[6]);
						if (strcmp(pAttachment->m_Name, "splatter8") == 0)
							Graphics()->SetColor(1, 1, 1, PlayerInfo->m_aSplatter[7]);
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
	vec4 TeamColor = vec4(0.3f, 0.3f, 0.3f, 1);
	
	if (Team == TEAM_RED)
		TeamColor = vec4(1.0f, 0.3f, 0.0f, 1);
	else if (Team == TEAM_BLUE)
		TeamColor = vec4(0.0f, 0.3f, 1.0f, 1);
	
	dbg_assert(pSkeleton != 0x0, "missing skeleton information");
	
	vec2 Scale = vec2(0.5f, 0.5f);
		
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
				CSpineSlotTimeline *pSlotTimeline = 0x0;
				

				// find timelines
				{
					auto SlotTimelineIter = pAnimation->m_lSlotTimeline.find(pSlot->m_Name);
					if(SlotTimelineIter != pAnimation->m_lSlotTimeline.end())
						pSlotTimeline = &SlotTimelineIter->second;	
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

					
					if (strcmp(pAttachment->m_Name, "1_team") == 0 || 
						strcmp(pAttachment->m_Name, "2_team") == 0 || 
						strcmp(pAttachment->m_Name, "3_team") == 0 || 
						strcmp(pAttachment->m_Name, "4_team") == 0 || 
						strcmp(pAttachment->m_Name, "weapon_team") == 0)
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
