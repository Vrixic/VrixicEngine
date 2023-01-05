#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Memory/Core/MemoryAllocater.h>
#include <Runtime/Memory/Core/MemoryPool.h>

#include <iostream>
#include <type_traits>

/**
* Singleton
*
* Memory pool -> double ended start buffer, ends store the memory infos
*/
class MemoryManager
{
private:
	/* Instance for singleton */
	static MemoryManager* Instance;

	// PostInit should only be called once 
	bool HasPostInitialized;

	/* Memory handle to the main block of memory, main memory pool */
	MemoryPool* MemoryPoolHandle;

	/* amount of memory allocated for main memory pool */
	uint64 MemoryAllocationSize;

	/* Memory handle to the pool for memory infos */
	MemoryPool* MemoryInfoPoolHandle;

	/* amount of memory allocated for memory info pool */
	uint64 MemoryInfoAllocationSize;

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
		ASSERT(MemoryAllocationSize == 0);
		ASSERT(MemoryInfoAllocationSize == 0);
#endif
		MemoryAllocationSize = MEBIBYTES_TO_BYTES(100);
		MemoryInfoAllocationSize = MEBIBYTES_TO_BYTES(10);

		MemoryPoolHandle = new MemoryPool(MemoryAllocationSize);
		MemoryInfoPoolHandle = new MemoryPool(MemoryInfoAllocationSize);
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
		uint64 LastAllocationSize = MemoryAllocationSize;
		MemoryAllocationSize = MEBIBYTES_TO_BYTES(inSizeInMebibytes);
		
		// Re-Allocate more memory, copy of data is done by the pool
		char* NewMemPtr = MemoryPoolHandle->Resize(MemoryAllocationSize);

		// Update the memory infos to point to the new memory 
		char* MemInfoMemPtr = MemoryInfoPoolHandle->GetMemoryHandle();
		uint64 MemInfoByteOffset = MemoryInfoPoolHandle->GetByteOffsetFromStart();
		uint64 ByteOffsetFromStart = 0;

		while (MemInfoByteOffset != ByteOffsetFromStart)
		{
			MemoryInfo* MemInfo = (MemoryInfo*)MemInfoMemPtr;
			MemInfo->MemoryStartPtr = NewMemPtr;

			ByteOffsetFromStart += sizeof(MemoryInfo);

			MemInfoMemPtr += ByteOffsetFromStart;
			NewMemPtr += MemInfo->MemorySize;
		}
		
		HasPostInitialized = true;
	}

	/**
	* Allocate memory based on the item and count
	*u
	* @param inNumCount - count of how many to allocate
	*
	* @return char** pointer pointing to the memory location
	*/
	template<typename T>
	char** Malloc(uint32 inNumCount)
	{
		uint64 RequestedSize = sizeof(T) * inNumCount;

		MemoryInfo* MemInfo = MemoryInfoPoolHandle->Malloc<MemoryInfo>(1);
		MemInfo->MemoryStartPtr = (char*)(new (MemoryPoolHandle->Malloc(RequestedSize)) T());
		MemInfo->MemorySize = RequestedSize;

		return &MemInfo->MemoryStartPtr;
	}


	/**
	* Allocate memory for allocater and also create one, calls constructor
	*
	* @param inSizeInBytes - memory size the allocater will get
	* @param inExtraBytesPercent - how much to increase the bytes size by for extra memory, by default 10%
	*
	* @return char** pointer pointing to the allocater
	*/
	template<typename T>
	T** MallocAllocater(uint64 inSizeInBytes, float inExtraBytesPercent = 0.1f)
	{
		// Check if we can allocate enough memory
#if _DEBUG | _EDITOR
		bool IsAllocater = std::is_base_of<MemoryAllocater, T>::value;
		ASSERT(IsAllocater);
#endif

		float AlignmentCheck = static_cast<float>(inSizeInBytes * inExtraBytesPercent) / sizeof(MemoryInfo);
		AlignmentCheck = ceilf(AlignmentCheck);

		inSizeInBytes += static_cast<uint64>(AlignmentCheck * sizeof(MemoryInfo)) + 32;

		MemoryInfo* MemInfo = MemoryInfoPoolHandle->Malloc<MemoryInfo>(1);
		T* TInstance = (new (MemoryPoolHandle->Malloc(inSizeInBytes)) T()); // placement-new
		MemInfo->MemoryStartPtr =(char*) TInstance; 
		MemInfo->MemorySize = inSizeInBytes;

		TInstance->Init(MemInfo->MemoryStartPtr, inSizeInBytes);

		return (T**)&MemInfo->MemoryStartPtr;
	}

	/**
	* Frees all memory
	*/
	void FreeAllMemory()
	{
		delete MemoryPoolHandle;
		delete MemoryInfoPoolHandle;
	}

	/**
	* Returns amount of memory in use
	*/
	uint64 GetMemoryUsed()
	{
		return MemoryPoolHandle->GetMemoryUsed() + MemoryInfoPoolHandle->GetMemoryUsed();
	}
};