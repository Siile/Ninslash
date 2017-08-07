#include "skelebank.h"
#include <engine/storage.h>


CSkelebank::CSkelebank(CRenderTools *pRenderTools)
{
	m_pRenderTools = pRenderTools;
}


void CSkelebank::Init(IStorage *pStorage)
{
	m_pStorage = pStorage;

	// mind the order!
	AddSkeleton("data/anim/human.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/human.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/alien.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/alien.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/robo.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/robo.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/foxy.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/foxy.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/skele.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/skele.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/ears.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/ears.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/gasmask.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/gasmask.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/walker.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/walker.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/drone.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/drone.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/walker_bottom.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/walker_bottom.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/door1.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/door1.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/jumppad.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/jumppad.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/reactor.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/reactor.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/teslacoil.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/teslacoil.atlas", IStorage::STORAGETYPE_CLIENT);
	
	AddSkeleton("data/anim/screen.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/screen.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/stardroid.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/stardroid.atlas", IStorage::STORAGETYPE_CLIENT);

	AddSkeleton("data/anim/crawler.json", IStorage::STORAGETYPE_CLIENT);
	AddAtlas("data/anim/crawler.atlas", IStorage::STORAGETYPE_CLIENT);
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
		dbg_msg("spinebank", "FATAL: Unable to load spine skeleton: '%s', game will exit.", pFilename);
		dbg_break(); // game will crash without having the skeletons, so just exit it right away to avoid confusion
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
