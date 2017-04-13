

#include <base/system.h>
#include <engine/map.h>
#include <engine/storage.h>
#include "datafile.h"

class CMap : public IEngineMap
{
	CDataFileReader m_DataFile;
public:
	CMap() {}

	virtual void *GetData(int Index) { return m_DataFile.GetData(Index); }
	virtual void *GetDataSwapped(int Index) { return m_DataFile.GetDataSwapped(Index); }
	virtual void UnloadData(int Index) { m_DataFile.UnloadData(Index); }
	virtual void *GetItem(int Index, int *pType, int *pID) { return m_DataFile.GetItem(Index, pType, pID); }
	virtual void GetType(int Type, int *pStart, int *pNum) { m_DataFile.GetType(Type, pStart, pNum); }
	virtual void *FindItem(int Type, int ID) { return m_DataFile.FindItem(Type, ID); }
	virtual int NumItems() { return m_DataFile.NumItems(); }

	virtual void Unload()
	{
		m_DataFile.Close();
	}

	virtual bool Load(const char *pMapName)
	{
		IStorage *pStorage = Kernel()->RequestInterface<IStorage>();
		if(!pStorage)
			return false;
		return m_DataFile.Open(pStorage, pMapName, IStorage::TYPE_ALL);
	}

	virtual bool IsLoaded()
	{
		return m_DataFile.IsOpen();
	}

	virtual unsigned Crc()
	{
		return m_DataFile.Crc();
	}

	virtual CDataFileReader* GetFileReader() { return &m_DataFile; } // MapGen
};

extern IEngineMap *CreateEngineMap() { return new CMap; }
