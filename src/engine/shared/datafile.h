

#ifndef ENGINE_SHARED_DATAFILE_H
#define ENGINE_SHARED_DATAFILE_H
#include <base/system.h>

// FIXME: Original definition in "graphics" (CImageInfo)... clone like CImageInfoFile for server/client compatibility (really redefined in client side :/)
class CImageInfoFile
{
public:
	enum
	{
		FORMAT_AUTO=-1,
		FORMAT_RGB=0,
		FORMAT_RGBA=1,
		FORMAT_ALPHA=2,
	};

	/* Variable: width
		Contains the width of the image */
	int m_Width;

	/* Variable: height
		Contains the height of the image */
	int m_Height;

	/* Variable: format
		Contains the format of the image. See <Image Formats> for more information. */
	int m_Format;

	/* Variable: data
		Pointer to the image data. */
	void *m_pData;
};

// raw datafile access
class CDataFileReader
{
	struct CDatafile *m_pDataFile;
	void *GetDataImpl(int Index, int Swap);
public:
	CDataFileReader() : m_pDataFile(0) {}
	~CDataFileReader() { Close(); }

	bool IsOpen() const { return m_pDataFile != 0; }

	bool Open(class IStorage *pStorage, const char *pFilename, int StorageType);
	bool Close();

	static bool GetCrcSize(class IStorage *pStorage, const char *pFilename, int StorageType, unsigned *pCrc, unsigned *pSize);

	void *GetData(int Index);
	void *GetDataSwapped(int Index); // makes sure that the data is 32bit LE ints when saved
	int GetDataSize(int Index);
	void UnloadData(int Index);
	void *GetItem(int Index, int *pType, int *pID);
	int GetItemSize(int Index);
	void GetType(int Type, int *pStart, int *pNum);
	void *FindItem(int Type, int ID);
	int NumItems();
	int NumData();
	void Unload();

	unsigned Crc();
};

// write access
class CDataFileWriter
{
	struct CDataInfo
	{
		int m_UncompressedSize;
		int m_CompressedSize;
		void *m_pCompressedData;
	};

	struct CItemInfo
	{
		int m_Type;
		int m_ID;
		int m_Size;
		int m_Next;
		int m_Prev;
		void *m_pData;
	};

	struct CItemTypeInfo
	{
		int m_Num;
		int m_First;
		int m_Last;
	};

	enum
	{
		MAX_ITEM_TYPES=0xffff,
		MAX_ITEMS=1024,
		MAX_DATAS=1024,
	};

	IOHANDLE m_File;
	int m_NumItems;
	int m_NumDatas;
	int m_NumItemTypes;
	CItemTypeInfo *m_pItemTypes;
	CItemInfo *m_pItems;
	CDataInfo *m_pDatas;

public:
	CDataFileWriter();
	~CDataFileWriter();
	bool Open(class IStorage *pStorage, const char *Filename);
	int AddData(int Size, void *pData);
	int AddDataSwapped(int Size, void *pData);
	int AddItem(int Type, int ID, int Size, void *pData);
	int Finish();

	// MapGen
	bool CreateEmptyMap(class IStorage *pStorage, const char *pFileName, int w, int h, CImageInfoFile *pTileset = 0x0);
	bool SaveMap(class IStorage *pStorage, CDataFileReader *pFileMap, const char *pFileName, char *pBlocksData = 0x0, int BlocksDataSize = 0);
};


#endif
