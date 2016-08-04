
#include <engine/storage.h>
#include <engine/external/json-parser/json.h>

#include "spine.h"

// destructive function!
array<string> ParseTuple(char *pStr)
{
	array<string> aTuple;

	char *pToken = strtok(pStr, ",");
	while(pToken)
	{
		aTuple.add(str_skip_whitespaces(pToken));
		pToken = strtok(0x0, ",");
	}

	return aTuple;
}

CSpineReader::CSpineReader()
{

}

CSpineReader::~CSpineReader()
{

}

// parsing spine json format
// esotericsoftware.com/spine-json-format
bool CSpineReader::LoadFromFile(class IStorage *pStorage, const char *pFilename, int StorageType,
		array<CSpineBone> *plBones,
		array<CSpineSlot> *plSlots,
		SkinMap *pmSkins,
		std::map<string, CSpineAnimation>* pmAnimations)
{
	if(!pStorage)
	{
		dbg_msg("spine", "Storage not loaded");
		return false;
	}

	if(!pFilename)
	{
		dbg_msg("spine", "Invalid filename");
		return false;
	}

	dbg_msg("spine", "load file: %s", pFilename);

	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ, StorageType);
	if(!File)
		return false;

	// load file content into memory
	unsigned DataSize = io_length(File);
	char *pData = (char *)mem_alloc(DataSize+1, 1);
	io_read(File, pData, DataSize);
	pData[DataSize] = '\0';
	io_close(File);

	bool Result = Load(pData, plBones, plSlots, pmSkins, pmAnimations);

	mem_free(pData);

	return Result;
}

bool CSpineReader::Load(const char *pData,
		array<CSpineBone> *plBones,
		array<CSpineSlot> *plSlots,
		SkinMap *pmSkins,
		std::map<string, CSpineAnimation>* pmAnimations)
{
	// TODO: sanity checks! typecheck json values!

	if(!pData)
	{
		dbg_msg("spine", "Error: missing input data!");
		return false;
	}

	// parse *.json file
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	unsigned int DataSize = str_length(pData);
	json_value *pJsonData = json_parse_ex(&JsonSettings, pData, DataSize+1, aError);

	if(!pJsonData)
	{
		dbg_msg("spine", "Error on parsing spine file: %s", aError);
		return false;
	}

	// parse bones
	const json_value& rBones = (*pJsonData)["bones"];
	if(rBones.type == json_array && plBones)
	{
		for(int i = 0; i < rBones.u.array.length; i++)
		{
			const json_value& rBone = rBones[i];

			CSpineBone Bone;
			Bone.m_Name = (const char *)rBone["name"];
			Bone.m_Parent = (const char *)rBone["parent"]; // defaults to empty string
			Bone.m_Length = (double)rBone["length"]; // defaults to 0.0
			Bone.m_X = (double)rBone["x"]; // defaults to 0.0
			Bone.m_Y = (double)rBone["y"]; // defaults to 0.0
			Bone.m_ScaleX = rBone["scaleX"].type == json_none ? 1.0f : (double)rBone["scaleX"]; // defaults to 1.0
			Bone.m_ScaleY = rBone["scaleY"].type == json_none ? 1.0f : (double)rBone["scaleY"]; // defaults to 1.0
			Bone.m_Rotation = (double)rBone["rotation"]; // defaults to 0.0

			plBones->add(Bone);
		}
	}

	// parse slots
	const json_value& rSlots = (*pJsonData)["slots"];
	if(rBones.type == json_array && plSlots)
	{
		for(int i = 0; i < rSlots.u.array.length; i++)
		{
			const json_value& rSlot = rSlots[i];

			CSpineSlot Slot;
			Slot.m_Name = (const char *)rSlot["name"];
			Slot.m_Bone = (const char *)rSlot["bone"];
			Slot.m_Color = rSlot["color"].type == json_none ? "FFFFFFFF" : (const char *)rSlot["color"];
			Slot.m_Attachment = (const char *)rSlot["attachment"];

			plSlots->add(Slot);
		}
	}

	// parse skins
	if((*pJsonData)["skins"].type != json_none && pmSkins)
	{
		// TODO:
		const json_value& rSkins = (*pJsonData)["skins"];
		for(int i = 0; i < rSkins.u.object.length; i++)
		{
			const char *pSkinName = rSkins.u.object.values[i].name;
			const json_value& rSkin = (*rSkins.u.object.values[i].value);

			for(int j = 0; j < rSkin.u.object.length; j++)
			{
				const char *pSlotName = rSkin.u.object.values[j].name;
				const json_value& rAttachments = (*rSkin.u.object.values[j].value);

				for(int k = 0; k < rAttachments.u.object.length; k++)
				{
					const char *pAttachmentName = rAttachments.u.object.values[k].name;
					const json_value& rAttachment = (*rAttachments.u.object.values[k].value);

					CSpineAttachment Attachment;
					Attachment.m_Name = rAttachment["name"].type == json_none ? pAttachmentName : (const char *)rAttachment["name"];
					
					// attachment type
					{
						int Type = SPINE_ATTACHMENT_REGION;
						if(rAttachment["type"].type != json_none)
						{
							if(str_comp("region", rAttachment["type"]) == 0)
								Type = SPINE_ATTACHMENT_REGION;
							else if(str_comp("regionsequence", rAttachment["type"]) == 0)
								Type = SPINE_ATTACHMENT_REGIONSEQUENCE;
							else if(str_comp("boundingbox", rAttachment["type"]) == 0)
								Type = SPINE_ATTACHMENT_BBOX;
							else if(str_comp("skinnedmesh", rAttachment["type"]) == 0)
								Type = SPINE_ATTACHMENT_SKINNED_MESH;
							else
								dbg_msg("spine", "Unsupported attachment type: %s", rAttachment["type"]);
						}

						Attachment.m_Type = Type;
					}

					if(Attachment.m_Type == SPINE_ATTACHMENT_REGION || Attachment.m_Type == SPINE_ATTACHMENT_REGIONSEQUENCE)
					{
						// region + regionsequence values
						Attachment.m_Region.m_X = (double)rAttachment["x"]; // defaults to 0.0
						Attachment.m_Region.m_Y = (double)rAttachment["y"]; // defaults to 0.0
						Attachment.m_Region.m_ScaleX = rAttachment["scaleX"].type == json_none ? 1.0f : (double)rAttachment["scaleX"]; // defaults to 1.0
						Attachment.m_Region.m_ScaleY = rAttachment["scaleY"].type == json_none ? 1.0f : (double)rAttachment["scaleY"]; // defaults to 1.0
						Attachment.m_Region.m_Rotation = (double)rAttachment["rotation"]; // defaults to 0.0
						Attachment.m_Region.m_Width = (double)rAttachment["width"];
						Attachment.m_Region.m_Height = (double)rAttachment["height"];

						// region sequence values
						Attachment.m_RegionSeq.m_Fps = (double)rAttachment["fps"];

						// region sequence mode
						{
							int SeqMode = SPINE_SEQUENCE_MODE_FORWARD;
							if(rAttachment["mode"].type != json_none)
							{
								if(str_comp("forward", rAttachment["mode"]) == 0)
									SeqMode = SPINE_SEQUENCE_MODE_FORWARD;
								else if(str_comp("backward", rAttachment["mode"]) == 0)
									SeqMode = SPINE_SEQUENCE_MODE_BACKWARD;
								else if(str_comp("forwardLoop", rAttachment["mode"]) == 0)
									SeqMode = SPINE_SEQUENCE_MODE_FORWARD_LOOP;
								else if(str_comp("backwardLoop", rAttachment["mode"]) == 0)
									SeqMode = SPINE_SEQUENCE_MODE_BACKWARD_LOOP;
								else if(str_comp("pingPong", rAttachment["mode"]) == 0)
									SeqMode = SPINE_SEQUENCE_MODE_PING_PONG;
								else if(str_comp("random", rAttachment["mode"]) == 0)
									SeqMode = SPINE_SEQUENCE_MODE_RANDOM;
								else
									dbg_msg("spine", "Unsupported attachment region sequence mode: %s", rAttachment["mode"]);
							}

							Attachment.m_RegionSeq.m_Mode = SeqMode;
						}
					}
					else if(Attachment.m_Type == SPINE_ATTACHMENT_SKINNED_MESH)
					{
						if(rAttachment["uvs"].type != json_none)
						{
							for(int uv = 0; uv < rAttachment["uvs"].u.array.length; uv++)
								Attachment.m_SkinnedMesh.m_aUV.add((double)rAttachment["uvs"][uv]);
						}

						if(rAttachment["triangles"].type != json_none)
						{
							for(int t = 0; t < rAttachment["triangles"].u.array.length; t++)
								Attachment.m_SkinnedMesh.m_aTriangles.add((double)rAttachment["triangles"][t]);
						}

						if(rAttachment["vertices"].type != json_none)
						{
							for(int v = 0; v < rAttachment["vertices"].u.array.length; v++)
								Attachment.m_SkinnedMesh.m_aVertices.add((double)rAttachment["vertices"][v]);
						}
						
						Attachment.m_SkinnedMesh.m_Hull = (int)rAttachment["hull"]; // defaults to 0
						
						if(rAttachment["edges"].type != json_none)
						{
							for(int v = 0; v < rAttachment["edges"].u.array.length; v++)
								Attachment.m_SkinnedMesh.m_aEdges.add((double)rAttachment["edges"][v]);
						}

						Attachment.m_SkinnedMesh.m_Width = (double)rAttachment["width"]; // defaults to 0.0
						Attachment.m_SkinnedMesh.m_Height = (double)rAttachment["height"]; // defaults to 0.0
					}

					// bounding box mode
					// TODO: need an example, no clue about the format out of the documentation

					// TODO: performance? profile!?
					(*pmSkins)[pSkinName][pSlotName][pAttachmentName] = Attachment;
				}
			}
		}
	}

	// parse events
	if((*pJsonData)["events"].type != json_none)
	{
		// TODO:
		const json_value& rEvents = (*pJsonData)["events"];
	}

	// parse animations
	if((*pJsonData)["animations"].type == json_object && pmAnimations)
	{
		// TODO:
		const json_value& rAnimations = (*pJsonData)["animations"];
		for(int i = 0; i < rAnimations.u.object.length; i++)
		{
			const char *pAnimationName = rAnimations.u.object.values[i].name;
			const json_value& rAnimation = (*rAnimations.u.object.values[i].value);

			CSpineAnimation Animation;

			if(rAnimation["slots"].type == json_object)
			{
				const json_value& rSlots = rAnimation["slots"];

				for(int j = 0; j < rSlots.u.object.length; j++)
				{
					const char *pSlotName = rSlots.u.object.values[j].name;
					const json_value& rSlot = (*rSlots.u.object.values[j].value);

					CSpineSlotTimeline SlotTimeline;

					if(rSlot["attachment"].type == json_array)
					{
						const json_value& rSlotAttachments = rSlot["attachment"];

						for(int k = 0; k < rSlotAttachments.u.array.length; k++)
						{
							const json_value& rAttachment = rSlotAttachments[k];

							CSpineSlotKeyframeAttachment AttachmentKeyframe;
							AttachmentKeyframe.m_Time = (double)rAttachment["time"];
							AttachmentKeyframe.m_Attachment = (const char *)rAttachment["name"];

							SlotTimeline.m_lAttachments.add_unsorted(AttachmentKeyframe); // we assume sorted ordering
						}
					}

					Animation.m_lSlotTimeline[pSlotName] = SlotTimeline;
				}

			}
			
			if(rAnimation["bones"].type == json_object)
			{
				const json_value& rBones = rAnimation["bones"];

				for(int j = 0; j < rBones.u.object.length; j++)
				{
					const char *pBoneName = rBones.u.object.values[j].name;
					const json_value& rBone = (*rBones.u.object.values[j].value);

					CSpineBoneTimeline BoneTimeline;

					if(rBone["translate"].type == json_array)
					{
						const json_value& rBoneTranslations = rBone["translate"];

						for(int k = 0; k < rBoneTranslations.u.array.length; k++)
						{
							const json_value& rTranslation = rBoneTranslations[k];

							CSpineBoneKeyframeTranslate TranslationKeyframe;
							TranslationKeyframe.m_Time = (double)rTranslation["time"];
							TranslationKeyframe.m_X = (double)rTranslation["x"];
							TranslationKeyframe.m_Y = (double)rTranslation["y"];

							TranslationKeyframe.m_Curve.m_Type = SPINE_CURVE_LINEAR; // default
							if(rTranslation["curve"].type == json_string)
							{
								if(str_comp("stepped", rTranslation["curve"]) == 0)
									TranslationKeyframe.m_Curve.m_Type = SPINE_CURVE_STEPPED;
								else if(str_comp("linear", rTranslation["curve"]) == 0)
									TranslationKeyframe.m_Curve.m_Type = SPINE_CURVE_LINEAR;
								else
									dbg_msg("spine", "Unsupported keyframe type: %s", rTranslation["curve"]);
							}
							else if(rTranslation["curve"].type == json_array)
							{
								TranslationKeyframe.m_Curve.m_Type = SPINE_CURVE_BEZIER;
								const json_value& rCurve = rTranslation["curve"];
								for(int l = 0; l < rCurve.u.array.length; l++)
									TranslationKeyframe.m_Curve.m_lPoints.add((double)rCurve[l]);
							}

							BoneTimeline.m_lTranslations.add_unsorted(TranslationKeyframe); // we assume sorted ordering
						}
					}

					if(rBone["rotate"].type == json_array)
					{
						const json_value& rBoneRotations = rBone["rotate"];

						for(int k = 0; k < rBoneRotations.u.array.length; k++)
						{
							const json_value& rRotation = rBoneRotations[k];

							CSpineBoneKeyframeRotate RotationKeyframe;

							RotationKeyframe.m_Time = (double)rRotation["time"];
							RotationKeyframe.m_Rotation = (double)rRotation["angle"];

							RotationKeyframe.m_Curve.m_Type = SPINE_CURVE_LINEAR; // default
							if(rRotation["curve"].type == json_string)
							{
								if(str_comp("stepped", rRotation["curve"]) == 0)
									RotationKeyframe.m_Curve.m_Type = SPINE_CURVE_STEPPED;
								else if(str_comp("linear", rRotation["curve"]) == 0)
									RotationKeyframe.m_Curve.m_Type = SPINE_CURVE_LINEAR;
								else
									dbg_msg("spine", "Unsupported keyframe type: %s", rRotation["curve"]);
							}
							else if(rRotation["curve"].type == json_array)
							{
								RotationKeyframe.m_Curve.m_Type = SPINE_CURVE_BEZIER;
								const json_value& rCurve = rRotation["curve"];
								for(int l = 0; l < rCurve.u.array.length; l++)
									RotationKeyframe.m_Curve.m_lPoints.add((double)rCurve[l]);
							}

							BoneTimeline.m_lRotations.add(RotationKeyframe); // we assume sorted ordering
						}
					}


					if(rBone["scale"].type == json_array)
					{
						const json_value& rBoneScales = rBone["scale"];

						for(int k = 0; k < rBoneScales.u.array.length; k++)
						{
							const json_value& rScale = rBoneScales[k];

							CSpineBoneKeyframeScale ScaleKeyframe;

							ScaleKeyframe.m_Time = (double)rScale["time"];
							ScaleKeyframe.m_ScaleX = (double)rScale["x"];
							ScaleKeyframe.m_ScaleY = (double)rScale["y"];

							ScaleKeyframe.m_Curve.m_Type = SPINE_CURVE_LINEAR; // default
							if(rScale["curve"].type == json_string)
							{
								if(str_comp("stepped", rScale["curve"]) == 0)
									ScaleKeyframe.m_Curve.m_Type = SPINE_CURVE_STEPPED;
								else if(str_comp("linear", rScale["curve"]) == 0)
									ScaleKeyframe.m_Curve.m_Type = SPINE_CURVE_LINEAR;
								else
									dbg_msg("spine", "Unsupported keyframe type: %s", rScale["curve"]);
							}
							else if(rScale["curve"].type == json_array)
							{
								ScaleKeyframe.m_Curve.m_Type = SPINE_CURVE_BEZIER;
								const json_value& rCurve = rScale["curve"];
								for(int l = 0; l < rCurve.u.array.length; l++)
									ScaleKeyframe.m_Curve.m_lPoints.add((double)rCurve[l]);
							}

							BoneTimeline.m_lScales.add_unsorted(ScaleKeyframe); // we assume sorted ordering
						}
					}

					Animation.m_lBoneTimeline[pBoneName] = BoneTimeline;
				}
			}

			dbg_msg("spine", "animation: %s", pAnimationName);
			(*pmAnimations)[pAnimationName] = Animation;
		}
	}

	json_value_free(pJsonData);

	return true;
}

bool CSpineReader::LoadAtlasFromFile(class IStorage *pStorage , const char *pFilename, int StorageType, CSpineAtlas* pAtlas)
{
	if(!pStorage)
		return false;

	if(!pFilename)
		return false;

	dbg_msg("spine", "load atlas file: %s", pFilename);

	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ, StorageType);
	if(!File)
		return false;

	// load file content into memory
	unsigned DataSize = io_length(File);
	char *pData = (char *)mem_alloc(DataSize+1, 1);
	io_read(File, pData, DataSize);
	pData[DataSize] = '\0';
	io_close(File);

	bool Result = LoadAtlas(pData, pAtlas);

	mem_free(pData);

	return Result;
}

bool CSpineReader::LoadAtlas(const char *pAtlasData, CSpineAtlas *pAtlas)
{
	if(!pAtlasData)
		return false;

	if(!pAtlas)
		return false;

	// parse atlas file
	m_LineReader.InitString(pAtlasData);

	bool Error = false;
	CSpineAtlasPage *pCurPage = 0x0; // weak ptr

	while(true)
	{
		char *pLine = m_LineReader.Get();
		if(!pLine)
			break;

		pLine = str_skip_whitespaces(pLine);
		if(str_length(pLine) == 0)
		{
			// start new atlas page
			pCurPage = 0x0;
			continue;
		}

		if(!pCurPage)
		{
			// fill new page
			CSpineAtlasPage Page;
			Page.m_Name = pLine;
			//dbg_msg("spine", "atlas page: %s", Page.m_Name);

			{
				// size
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find atlas size");
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pSize = pLine + str_length("size: ");

				// parse tuple
				char aBuf[128];
				str_copy(aBuf, pSize, sizeof(aBuf));
				dbg_msg("spine", "atlas size: %s", aBuf);
				array<string> aSize = ParseTuple(aBuf);

				if(aSize.size() != 2)
				{
					dbg_msg("spine", "Error on parsing atlas size: %s", pSize);
					Error = true;
					break;
				}

				Page.m_Width = str_toint(aSize[0].cstr());
				Page.m_Height = str_toint(aSize[1].cstr());
			}
			{
				// format
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find atlas format");
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pFormat = pLine + str_length("format: ");
				dbg_msg("spine", "atlas format: %s", pFormat);

				int Format = ParsePageFormat(pFormat);
				if(Format == -1)
				{
					dbg_msg("spine", "Unsupported atlas format: %s", pFormat);
					Error = true;
					break;
				}

				Page.m_Format = Format;
			}

			{
				// filter
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find atlas filter");
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pFilter = pLine + str_length("filter: ");

				// parse tuple
				char aBuf[128];
				str_copy(aBuf, pFilter, sizeof(aBuf));
				dbg_msg("spine", "atlas filter: %s", pFilter);
				array<string> aFilters = ParseTuple(aBuf);

				if(aFilters.size() != 2)
				{
					dbg_msg("spine", "Error on parsing atlas filter: %s", pFilter);
					Error = true;
					break;
				}

				int FilterMin = ParsePageFilter(aFilters[0].cstr());
				int FilterMag = ParsePageFilter(aFilters[1].cstr());
				if(FilterMin == -1 || FilterMag == -1)
				{
					dbg_msg("spine", "Unsupported atlas filter: %s, %s", FilterMin, FilterMag);
					Error = true;
					break;
				}

				Page.m_FilterMin = FilterMin;
				Page.m_FilterMag = FilterMag;
			}

			{
				// TODO: repeat
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find atlas `rotate`");
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pFilter = pLine + str_length("rotate: ");

			}

			pAtlas->m_lPages.add(Page);
			pCurPage = &pAtlas->m_lPages[pAtlas->m_lPages.size()-1];
		}
		else
		{
			// new region
			CSpineAtlasRegion Region;
			Region.m_Name = pLine;

			{
				// rotate
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find region `rotate` (%s)", Region.m_Name.cstr());
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pRotate = pLine + str_length("rotate: ");

				Region.m_Rotate = str_comp("true", pRotate) == 0;
			}

			{
				// xy
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find region `xy` (%s)", Region.m_Name.cstr());
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pXY = pLine + str_length("xy: ");

				// parse tuple
				char aBuf[128];
				str_copy(aBuf, pXY, sizeof(aBuf));
				array<string> aTuple = ParseTuple(aBuf);

				if(aTuple.size() != 2)
				{
					dbg_msg("spine", "Error on parsing regio `xy` (%s): %s", Region.m_Name.cstr(), pXY);
					Error = true;
					break;
				}

				Region.m_X = str_toint(aTuple[0]);
				Region.m_Y = str_toint(aTuple[1]);
			}

			{
				// size
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find region `size` (%s)", Region.m_Name.cstr());
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pSize = pLine + str_length("size: ");

				// parse tuple
				char aBuf[128];
				str_copy(aBuf, pSize, sizeof(aBuf));
				array<string> aTuple = ParseTuple(aBuf);

				if(aTuple.size() != 2)
				{
					dbg_msg("spine", "Error on parsing regio `size` (%s): %s", Region.m_Name.cstr(), pSize);
					Error = true;
					break;
				}

				Region.m_Width = Region.m_Rotate ? str_toint(aTuple[1]) : str_toint(aTuple[0]);
				Region.m_Height = Region.m_Rotate ? str_toint(aTuple[0]) : str_toint(aTuple[1]);
			}

			{
				// orig
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find region `orig` (%s)", Region.m_Name.cstr());
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pOrig = pLine + str_length("ori: ");

				// parse tuple
				char aBuf[128];
				str_copy(aBuf, pOrig, sizeof(aBuf));
				array<string> aTuple = ParseTuple(aBuf);

				if(aTuple.size() != 2)
				{
					dbg_msg("spine", "Error on parsing regio `orig` (%s): %s", Region.m_Name.cstr(), pOrig);
					Error = true;
					break;
				}

				Region.m_OrigWidth = str_toint(aTuple[0]);
				Region.m_OrigHeight = str_toint(aTuple[1]);
			}

			{
				// offset
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find region `offset` (%s)", Region.m_Name.cstr());
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pOffset = pLine + str_length("offset: ");

				// parse tuple
				char aBuf[128];
				str_copy(aBuf, pOffset, sizeof(aBuf));
				array<string> aTuple = ParseTuple(aBuf);

				if(aTuple.size() != 2)
				{
					dbg_msg("spine", "Error on parsing regio `orig` (%s): %s", Region.m_Name.cstr(), pOffset);
					Error = true;
					break;
				}

				Region.m_OffsetX = str_toint(aTuple[0]);
				Region.m_OffsetY = str_toint(aTuple[1]);
			}

			{
				// index
				pLine = m_LineReader.Get();
				if(!pLine)
				{
					dbg_msg("spine", "Couldn't find region `index` (%s)", Region.m_Name.cstr());
					Error = true;
					break;
				}

				pLine = str_skip_whitespaces(pLine);
				const char *pIndex = pLine + str_length("index: ");

				Region.m_Index = str_toint(pIndex);
			}

			if(!pCurPage)
			{
				dbg_msg("spine", "Error: missing atlas page");
				Error = true;
				break;
			}

			// dbg_msg("spine", "Region %s, %d %d, %d %d, %d %d, %d %d, %d",
			//	Region.m_Name.cstr(), Region.m_X, Region.m_Y, Region.m_Width, Region.m_Height,
			//	Region.m_OrigWidth, Region.m_OrigHeight, Region.m_OffsetX, Region.m_OffsetY,
			//	Region.m_Index);

			pCurPage->m_lRegions.add(Region);
		}
	}
	
	m_LineReader.Shutdown();
	return (!Error);
}

int CSpineReader::ParsePageFormat(const char *pFormat) const
{
	if(!pFormat)
		return -1;

	// TODO:
	if(str_comp("RGBA8888", pFormat) == 0)
		return SPINE_ATLAS_FMT_R8G8B8A8;

	return -1;
}

int CSpineReader::ParsePageFilter(const char *pFilter) const
{
	if(!pFilter)
		return -1;

	// TODO:
	if(str_comp("Linear", pFilter) == 0)
		return SPINE_ATLAS_FILTER_LINEAR;

	return -1;
}

