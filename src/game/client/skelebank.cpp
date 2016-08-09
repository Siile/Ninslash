#include "skelebank.h"
#include <engine/storage.h>


CSkelebank::CSkelebank(CRenderTools *pRenderTools)
{
	m_pRenderTools = pRenderTools;
}


void CSkelebank::Init(IStorage *pStorage)
{
	m_pStorage = pStorage;
	
	AddSkeleton("data/anim/body1.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/body1.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/body2.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/body2.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/body3.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/body3.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/turret.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/turret.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/monster1.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/monster1.atlas", IStorage::STORAGETYPE_CLIENT);
}

CSkelebank::~CSkelebank()
{
	
}

void CSkelebank::AddSkeleton(const char *pFilename, int StorageType)
{
	// check if we have that skeleton already
	char aBuf[128];
	ExtractName(pFilename, aBuf, sizeof(aBuf));
	for(int i = 0; i < m_lSkeletons.size(); ++i)
	{
		if(!str_comp(m_lSkeletons[i]->m_aName, aBuf))
			return;
	}
	
	// load spine data
	array<CSpineBone> lBones;
	array<CSpineSlot> lSlots;
	SkinMap mSkins;
	std::map<string, CSpineAnimation> mAnimations;
	
	if(!m_SpineReader.LoadFromFile(Storage(), pFilename, StorageType, &lBones, &lSlots, &mSkins, &mAnimations))
	{
		dbg_msg("spinebank", "Unable to load spine skeleton: %s", pFilename);
		return;
	}
	
	CSkelebankSkeleton *pSkeletonInfo = new CSkelebankSkeleton();
	RenderTools()->LoadSkeletonFromSpine(pSkeletonInfo, lBones, lSlots, mSkins, mAnimations);
	pSkeletonInfo->m_External = 1; // external by default;
	str_copy(pSkeletonInfo->m_aName, aBuf, sizeof(pSkeletonInfo->m_aName));
	
	// load file content, uhm double-work actually
	IOHANDLE File = Storage()->OpenFile(pFilename, IOFLAG_READ, StorageType);
	if(!File)
		return;

	//
	unsigned DataSize = io_length(File);
	pSkeletonInfo->m_pJsonData = (char *)mem_alloc(DataSize+1, 1);
	io_read(File, pSkeletonInfo->m_pJsonData, DataSize);
	pSkeletonInfo->m_pJsonData[DataSize] = '\0';
	io_close(File);
	
	//
	m_lSkeletons.add(pSkeletonInfo);
}



void CSkelebank::AddAtlas(const char *pFilename, int StorageType)
{
	// check if we have that atlas already
	char aBuf[128];
	ExtractName(pFilename, aBuf, sizeof(aBuf));
	for(int i = 0; i < m_lAtlases.size(); ++i)
	{
		if(!str_comp(m_lAtlases[i]->m_aName, aBuf))
			return;
	}

	// load spine atlas
	CSpineAtlas SpineAtlas;
	if(!SpineReader()->LoadAtlasFromFile(Storage(), pFilename, StorageType, &SpineAtlas))
	{
		dbg_msg("spinebank", "Unable to load spine atlas: %s", pFilename);
		return;
	}

	// load atlas
	CSkelebankAtlas *pAtlas = new CSkelebankAtlas();
	if(!RenderTools()->LoadAtlasFromSpine(pAtlas, SpineAtlas))
	{
		dbg_msg("spinebank", "Unable to load atlas: %s", pFilename);
		delete pAtlas;
		return;
	}

	pAtlas->m_External = 1; // external by default;
	str_copy(pAtlas->m_aName, aBuf, sizeof(pAtlas->m_aName));

	// load file content, uhm double-work actually
	IOHANDLE File = Storage()->OpenFile(pFilename, IOFLAG_READ, StorageType);
	if(!File)
		return;

	//
	unsigned DataSize = io_length(File);
	pAtlas->m_pAtlasData = (char *)mem_alloc(DataSize+1, 1);
	io_read(File, pAtlas->m_pAtlasData, DataSize);
	pAtlas->m_pAtlasData[DataSize] = '\0';
	io_close(File);

	m_lAtlases.add(pAtlas);
}