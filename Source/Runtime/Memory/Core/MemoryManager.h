#pragma once
#include <Runtime/Memory/Core/MemoryAllocater.h>
#include <Runtime/Memory/Core/MemoryPool.h>
#include <Runtime/Memory/Core/MemoryHeap.h>

#include <iostream>
#include <type_traits>

/*
* @TODO:
*	- 256 alignment restriction solution? Special case solution maybe.
*	- Find a way to keep track of freed memory in O(1) time complexity (constant time)
*		- As of right now memory is 'freed' by setting te pointer to nullptr for indetification
*	- Differentiate between game and editor memory heaps
*/

class MemoryManager
{
private:
	/* Memory handle to the main block of memory, main memory pool */
	MemoryHeap* MemoryHeapHandle;

	/* amount of memory allocated for main memory pool */
	uint64 MemoryHeapSize;

	/* Memory handle to the pool for memory pages */
	MemoryHeap* MemoryPageHeapHandle;

	/* amount of memory allocated for memory page pool */
	uint64 MemoryPageHeapSize;

public:
	MemoryManager() :MemoryHeapHandle(nullptr), MemoryHeapSize(0),
		MemoryPageHeapHandle(nullptr), MemoryPageHeapSize(0) { }

	~MemoryManager()
	{
		if (MemoryHeapHandle != nullptr)
		{
			Shutdown();
		}
	}

	/**
	* Returns the one and only Instance to the Manager
	*/
	static MemoryManager& Get()
	{
		static MemoryManager Instance;
		return Instance;
	}

public:

	/**
	* Initialize the manager, allocates 100 mebibytes of memory by default,
	*	for more memory call Resize() once known
	*/
	void StartUp()
	{
#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		ASSERT(MemoryHeapHandle == nullptr);
		Shutdown();
#endif
		PreInit();
	}

	/**
	* Resize memory manager - Avoid calling this as its EXPENSIVE
	*	1048576 bytes is one MiB(mebibytes)
	*
	* @param inSizeInMebibytes - The size of the memory in mebibytes, 1024 mib = 1 gib
	*/
	void Resize(uint32 inSizeInMebibytes)
	{
		uint64 LastAllocationSize = MemoryHeapSize;
		MemoryHeapSize = MEBIBYTES_TO_BYTES(inSizeInMebibytes);

		// Re-Allocate more memory, copy of data is done by the pool
		uint8* NewMemoryHandle = MemoryHeapHandle->ResizeAndFlush(MemoryHeapSize);

		// Update the memory infos to point to the new memory 
		uint8* MemoryPageHandle = MemoryPageHeapHandle->GetMemoryHandle();
		uint64 MemoryPageBytesUsed = MemoryPageHeapHandle->GetHeapUsed();
		uint64 BytesUsed = 0;

		while (MemoryPageBytesUsed != BytesUsed)
		{
			MemoryPage* MemPage = (MemoryPage*)(MemoryPageHandle + BytesUsed);
			// from the memory page calculate new memory location
			MemPage->Data = (NewMemoryHandle + MemPage->OffsetFromHeapStart);

			BytesUsed += sizeof(MemoryPage);
		}
	}

	/**
	* Allocate memory based on the count, also align pointer by sizeof(T),
	* sizeof(T) must be a power of 2
	*
	* @param inNumCount - count of how many to allocate
	*
	* @return T** - pointer pointing to the memory location
	*/
	template<typename T>
	T** MallocAligned(uint64 inSizeInBytes, uint64 inAlignment = sizeof(T))
	{
		// Allocate a new memory page
		MemoryPage* MemPage = MemoryPageHeapHandle->Malloc<MemoryPage>(sizeof(MemoryPage));

		// Calculate the data offset from heap start + the alignment that will get applied
		MemPage->OffsetFromHeapStart = MemoryHeapHandle->GetHeapUsed() + inAlignment;

		// Find the worse case number of bytes we might have to shift
		inSizeInBytes += inAlignment; // allocate extra

		uint8* RawMemPtr = MemoryHeapHandle->Malloc<uint8>(inSizeInBytes);

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		std::cout << "[Memory Manager] Memory Allocated, size in bytes: " << inSizeInBytes <<
			", with alignment: " << inAlignment << "\n";
#endif

		// Align the block, if their isn't alignment, shift it up the full 'align' bytes, so we always 
		// have room to store the shift 
		uint8* AlignedPtr = MemoryUtils::AlignPointer<uint8>(RawMemPtr, inAlignment);
		if (AlignedPtr == RawMemPtr)
		{
			AlignedPtr += inAlignment;
		}

		// Determine the shift, and store it for later when freeing
		// (This works for up to 256-byte alignment.)
		intptr Shift = AlignedPtr - RawMemPtr;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT(Shift > 0 && Shift <= 256);
#endif

		AlignedPtr[-1] = static_cast<uint8>(Shift & 0xff);

		MemPage->Data = (uint8*)(new (AlignedPtr) T());
		MemPage->MemorySize = inSizeInBytes;

		return (T**)&MemPage->Data;
	}

	/**
	* Frees the memory at the pointer passed in
	*
	* @param inPtrToMemory - Pointer to the memory to be freed
	*/
	void Free(void** inPtrToMemory)
	{
		MemoryPageHeapHandle->Free(sizeof(MemoryPage));
	}

	/**
	* Flushs the Heap, but doesn't delete memory
	*/
	void FlushNoDelete()
	{
		MemoryHeapHandle->FlushNoDelete();
		MemoryPageHeapHandle->FlushNoDelete();
	}

	/**
	* Shuts down the manager, Releases All allocated memory
	*/
	void Shutdown()
	{
		Flush();
	}

private:
	/**
	* Pre-Inits the memory manager with default memory until size if know to allocate more memory
	*	Should be called on launch
	*/
	void PreInit()
	{
#if _DEBUG
		ASSERT(MemoryHeapSize == 0);
		ASSERT(MemoryPageHeapSize == 0);
#endif
		MemoryHeapSize = MEBIBYTES_TO_BYTES(100);
		MemoryPageHeapSize = MEBIBYTES_TO_BYTES(50);

		MemoryHeapHandle = new MemoryHeap(MemoryHeapSize);
		MemoryPageHeapHandle = new MemoryHeap(MemoryPageHeapSize);
	}


	/**
	* Frees/deletes all memory
	*/
	void Flush()
	{
		delete MemoryHeapHandle;
		delete MemoryPageHeapHandle;
	}

public:
	/**
	* Returns amount of memory in use
	*/
	uint64 GetMemoryUsed()
	{
		return MemoryHeapHandle->GetMemoryUsed() + MemoryPageHeapHandle->GetMemoryUsed();
	}
};
