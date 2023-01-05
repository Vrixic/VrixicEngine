#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Memory/Core/MemoryAllocater.h>

#define MEBIBYTES_TO_BYTES(inMiB) inMiB * 1048576

/**
* Singleton
*
* Memory pool -> double ended start buffer, ends store the memory infos
*/
class MemoryManager
{
private:
	// Instance for singleton
	static MemoryManager* Instance;

	// Giant memory block whose memory will be passed around
	char* MemoryPool;

	// PostInit should only be called once 
	bool HasPostInitialized;

	// Size of memory allocated
	uint64 AllocationSize;

	// amount of memory in use
	uint64 MemoryUsed;

	// amount of memory in use for memory infos
	uint64 MemoryUsedByMemInfo;

public:
	static MemoryManager* GetInstance()
	{
		if (Instance == nullptr)
		{
			Instance = new MemoryManager();
		}

		return Instance;
	}

public:
	/**
	* Pre-Inits the memory manager with default memory until size if know to allocate more memory
	*	Should be called on launch
	*/
	void PreInit()
	{
#if _DEBUG
		ASSERT(AllocationSize == 0);
#endif
		AllocationSize = MEBIBYTES_TO_BYTES(100);
		MemoryPool = new char[AllocationSize];
	}

	/**
	* Final initialization of memory manager with size known
	*	1048576 bytes is one MiB(mebibytes)
	*
	* @param inSizeInMebibytes - The size of the memory in mebibytes, 1024 mib = 1 gib
	*/
	void PostInit(uint32 inSizeInMebibytes)
	{
#if _DEBUG
		ASSERT(!HasPostInitialized);
#endif
		uint64 LastAllocationSize = AllocationSize;
		AllocationSize = MEBIBYTES_TO_BYTES(inSizeInMebibytes);

		// Copy over memory
		char* NewMemPool = new char[AllocationSize];
		char* NewMemPoolEnd = NewMemPool + AllocationSize;

		// Only copy data memory
		memcpy(NewMemPool, MemoryPool, MemoryUsed);

		// Copy over mem info
		memcpy(NewMemPoolEnd - MemoryUsedByMemInfo,(MemoryInfo*) (MemoryPool + LastAllocationSize) - MemoryUsedByMemInfo, MemoryUsedByMemInfo);

		uint64 TempMemUsedByMemInfos = MemoryUsedByMemInfo;
		while (TempMemUsedByMemInfos != 0)
		{
			// Re-Assign data pointer to new memory location
			MemoryInfo* MemInfo = (MemoryInfo*)(NewMemPoolEnd - TempMemUsedByMemInfos);
			MemInfo->MemoryStartPtr = NewMemPool + MemoryUsed - MemInfo->MemorySize;

			TempMemUsedByMemInfos -= sizeof(MemoryInfo);
		}

		delete[] MemoryPool;
		MemoryPool = NewMemPool;
		HasPostInitialized = true;
	}

	/**
	* Allocate memory based on the item and count
	*
	* @param inNumCount - count of how many to allocate
	*
	* @return char** pointer pointing to the memory location
	*/
	template<typename T>
	char** Malloc(uint32 inNumCount)
	{
		uint64 RequestedSize = sizeof(T) * inNumCount;

		// Check if we can allocate enough memory
#if _DEBUG | _EDITOR
		ASSERT((MemoryUsed + RequestedSize + MemoryUsedByMemInfo + sizeof(MemoryInfo)) < AllocationSize);
#endif

		MemoryInfo* MemInfo = (MemoryInfo*)(MemoryPool + AllocationSize) - MemoryUsedByMemInfo - sizeof(MemoryInfo);
		MemInfo->MemoryStartPtr = MemoryPool + MemoryUsed;
		MemInfo->MemorySize = RequestedSize;

		MemoryUsed += RequestedSize;
		MemoryUsedByMemInfo += sizeof(MemInfo);

		return &MemInfo->MemoryStartPtr;
	}


	/**
	* Allocate memory for allocater and also create one, calls constructor
	*
	* @param inRequestedSize - memory size the allocater will get
	*
	* @return char** pointer pointing to the allocater
	*/
	template<typename T>
	char** MallocAllocater(uint64 inRequestedSize)
	{
		// Check if we can allocate enough memory
#if _DEBUG | _EDITOR
		ASSERT(!std::is_base_of<MemoryAllocater>(T));
		ASSERT((MemoryUsed + inRequestedSize + MemoryUsedByMemInfo + sizeof(MemoryInfo)) > AllocationSize);
#endif

		inRequestedSize += ((MemoryAllocater)T)::GetAllocationBytes(inRequestedSize);

		MemoryInfo* MemInfo = (MemoryInfo*)(MemoryPool + AllocationSize) - MemoryUsedByMemInfo - sizeof(MemoryInfo);
		MemInfo->MemoryStartPtr = MemoryPool + MemoryUsed;
		MemInfo->MemorySize = inRequestedSize;

		MemoryUsed += inRequestedSize;
		MemoryUsedByMemInfo += sizeof(MemInfo);

		(*MemInfo->MemoryStartPtr)(MemInfo->MemoryStartPtr, inRequestedSize);

		return &MemInfo->MemoryStartPtr;
	}

	/**
	* Frees all memory
	*/
	void FreeAllMemory()
	{
		delete[] MemoryPool;
		MemoryPool = nullptr;
		MemoryUsed = 0;
		MemoryUsedByMemInfo = 0;
	}

	/**
	* Returns amount of memory in use
	*/
	uint64 GetMemoryUsed()
	{
		return MemoryUsed + MemoryUsedByMemInfo;
	}
};