#ifndef GAME_CLIENT_SKELEBANK_H
#define GAME_CLIENT_SKELEBANK_H

#include <math.h>
#include <base/math.h>

#include <game/mapitems.h>
#include <game/client/render.h>
#include <game/client/spine.h>


enum Skeletons
{
	SKELETON_BODY1,
	SKELETON_BODY2,
	SKELETON_BODY3,
	SKELETON_BODY4,
	SKELETON_BODY5,
	SKELETON_BODY6,
	SKELETON_BODY7,
	SKELETON_MONSTER1,
	SKELETON_DRONE,
	SKELETON_WALKER_BOTTOM,
	SKELETON_DOOR1,
	SKELETON_JUMPPAD,
	SKELETON_REACTOR,
	SKELETON_TESLACOIL,
	NUM_SKELETONS
};

enum Atlases
{
	ATLAS_BODY1,
	ATLAS_BODY2,
	ATLAS_BODY3,
	ATLAS_BODY4,
	ATLAS_BODY5,
	ATLAS_BODY6,
	ATLAS_BODY7,
	ATLAS_MONSTER1,
	ATLAS_DRONE,
	ATLAS_WALKER_BOTTOM,
	ATLAS_DOOR1,
	ATLAS_JUMPPAD,
	ATLAS_REACTOR,
	ATLAS_TESLACOIL,
	NUM_ATLASES
};

enum AnimList
{
	ANIM_IDLE,
	ANIM_OPEN,
	ANIM_OPENED,
	ANIM_CLOSED,
	ANIM_TRIGGER,
	NUM_ANIMS
};


static const char *aAnimList[NUM_ANIMS] = 
{
	"idle",
	"open",
	"opened",
	"closed",
	"trigger"
};



class CSkelebankSkeleton : public CAnimSkeletonInfo
{
public:
	//CSkelebank *m_pSkelebank;

	//CSkelebankSkeleton(CSkelebank *pSkelebank)
	CSkelebankSkeleton()
	{
		//m_pSkelebank = pSkelebank;
		m_aName[0] = 0;
		m_External = 0;
		m_pJsonData = 0x0;
	}

	~CSkelebankSkeleton()
	{
		mem_free(m_pJsonData); // eh..
	}
	
	char m_aName[128];
	int m_External;
	char *m_pJsonData; // owned-ptr
};


class CSkelebankAtlas : public CTextureAtlas
{
public:
	//CSkelebank *m_pSkelebank;

	//CSkelebankAtlas(CSkelebank *pSkelebank)
	CSkelebankAtlas()
	{
		//m_pSkelebank = pSkelebank;
		m_aName[0] = 0;
		m_External = 0;
	}

	~CSkelebankAtlas()
	{
		mem_free(m_pAtlasData); // eh..
	}

	char m_aName[128];
	int m_External;
	char *m_pAtlasData; // owned_ptr
};



class CSkelebank
{
	CSpineReader m_SpineReader;
	class IStorage *m_pStorage;
	CRenderTools *m_pRenderTools;
	
public:
	CSkelebank(CRenderTools *pRenderTools);
	~CSkelebank();
	
	void Init(class IStorage *pStorage);
	
	CSpineReader *SpineReader() { return &m_SpineReader; }
	class IStorage *Storage() { return m_pStorage; };
	CRenderTools *RenderTools() { return m_pRenderTools; }
	
	array<CSkelebankSkeleton*> m_lSkeletons;
	array<CSkelebankAtlas*> m_lAtlases;

	
	
	void AddSkeleton(const char *pFilename, int StorageType);
	void AddAtlas(const char *pFilename, int StorageType);
	

	static void ExtractName(const char *pFileName, char *pName, int BufferSize)
	{
		const char *pExtractedName = pFileName;
		const char *pEnd = 0;
		for(; *pFileName; ++pFileName)
		{
			if(*pFileName == '/' || *pFileName == '\\')
				pExtractedName = pFileName+1;
			else if(*pFileName == '.')
				pEnd = pFileName;
		}

		int Length = pEnd > pExtractedName ? min(BufferSize, (int)(pEnd-pExtractedName+1)) : BufferSize;
		str_copy(pName, pExtractedName, Length);
	}
};


#endif
