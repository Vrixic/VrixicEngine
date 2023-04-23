/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/StringDefines.h>
#include <Runtime/Memory/Core/MemoryHeap.h>
#include <Runtime/Memory/Core/MemoryUtils.h>

#include <iostream>
#include <type_traits>

/*
* @TODO:
*	- 256 alignment restriction solution? Special case solution maybe.
*	- Find a way to keep track of freed memory in O(1) time complexity (constant time)
*		- As of right now memory is 'freed' by setting te pointer to nullptr for indetification
*	- Differentiate between game and editor memory heaps
*	
*	- As of now, Resize() option is given, but MemoryAllocaters will be invalidated if resized(),
*		- Solution: Make Multiple Heaps which can be used, instead of resizing() current heap, 
*			make a new one.
*/

class VRIXIC_API MemoryManager
{
private:
	/* Memory handle to the main block of memory, main memory pool */
	TMemoryHeap<uint8>* MemoryHeapHandle;

	/* amount of memory allocated for main memory pool */
	uint64 MemoryHeapSize;

	/* Memory handle to the pool for memory pages */
	TMemoryHeap<MemoryPage>* MemoryPageHeapHandle;

	/* amount of memory allocated for memory page pool */
	uint64 MemoryPageHeapSize;

	/** Is this manager active? */
	bool bIsActive;

public:
	MemoryManager() :MemoryHeapHandle(nullptr), MemoryHeapSize(0),
		MemoryPageHeapHandle(nullptr), MemoryPageHeapSize(0), bIsActive(false) { }

	~MemoryManager()
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		Shutdown();
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
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		VE_ASSERT(!bIsActive, VE_TEXT("[MemoryManager]: Memory manager should not be created again.... MemoryManager::StartUp() SHOULD only be called once!"));
		Shutdown();
#endif
		bIsActive = true;
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
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		uint64 LastAllocationSize = MemoryHeapSize;
		MemoryHeapSize = MEBIBYTES_TO_BYTES(inSizeInMebibytes);

		// Re-Allocate more memory, copy of data is done by the pool
		uint8* NewMemoryHandle = MemoryHeapHandle->ResizeAndFlushByBytes(MemoryHeapSize);

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
	T** MallocAligned(uint32 inSizeInBytes, uint32 inAlignment = sizeof(T))
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		// Allocate a new memory page, only 1
		MemoryPage* MemPage = MemoryPageHeapHandle->Malloc(1);

		// Find the worse case number of bytes we might have to shift
		inSizeInBytes += inAlignment; // allocate extra

		uint8* RawMemPtr = MemoryHeapHandle->Malloc(inSizeInBytes);

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		std::cout << "[Memory Manager] Memory Allocated, size in bytes: " << inSizeInBytes <<
			", with alignment: " << inAlignment << "\n";
#endif

		// Align the pointer
		uint8* AlignedPtr = AlignPointerAndShift(RawMemPtr, inAlignment);

		// Calculate the data offset from heap start + the alignment that will get applied
		MemPage->OffsetFromHeapStart = (MemoryHeapHandle->GetHeapUsed() - inSizeInBytes) + (uint8)AlignedPtr[-1];
		MemPage->Data =AlignedPtr;
		MemPage->MemorySize = inSizeInBytes;

		return (T**)&MemPage->Data;
	}

	/**
	* Constructs the object T, calls its constructor then returns a pointer to it.
	* Allocate memory based on the count, also align pointer by sizeof(T),
	* sizeof(T) must be a power of 2
	*
	* @param inNumCount - count of how many to allocate
	*
	* @return T** - pointer pointing to the memory location
	*/
	template<typename T, typename... ArgTypes>
	T** MallocConstructAligned(uint32 inSizeInBytes, uint32 inAlignment = sizeof(T), ArgTypes&&... inArgs)
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		// Allocate a new memory page, only 1
		MemoryPage* MemPage = MemoryPageHeapHandle->Malloc(1);

		// Find the worse case number of bytes we might have to shift
		inSizeInBytes += inAlignment; // allocate extra

		uint8* RawMemPtr = MemoryHeapHandle->Malloc(inSizeInBytes);

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		std::cout << "[Memory Manager] Memory Allocated, size in bytes: " << inSizeInBytes <<
			", with alignment: " << inAlignment << "\n";
#endif

		// Align the pointer
		uint8* AlignedPtr = AlignPointerAndShift(RawMemPtr, inAlignment);

		// Calculate the data offset from heap start + the alignment that will get applied
		MemPage->OffsetFromHeapStart = (MemoryHeapHandle->GetHeapUsed() - inSizeInBytes) + (uint8)AlignedPtr[-1];
		MemPage->Data = (uint8*)(new (AlignedPtr) T((inArgs)...));
		MemPage->MemorySize = inSizeInBytes;

		return (T**)&MemPage->Data;
	}

	/**
	* 
	* @return T** - pointer pointing to the memory location
	*/
	template<typename T, typename... ArgTypes>
	T** MallocAllocater(uint32 inSizeInBytesForAllocater, uint32 inAllocaterAlignment = sizeof(T), ArgTypes&&... inArgs)
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		bool IsAllocater = std::is_base_of<MemoryAllocater, T>::value;
		ASSERT(IsAllocater, "[MemoryManager]: Trying to allocate an Object that is not a child of MemoryAllocater");
#endif
		// Extra bytes for alignment (* 2)
		ulong32 SizeOfAllocaterInBytes = sizeof(T) * 2;

		// Allocate a new memory page, only 1 for allocater
		MemoryPage* MemPageForAllocater = MemoryPageHeapHandle->Malloc(1);

		// Raw pointer to the allocater 
		uint8* RawAllocaterPtr = MemoryHeapHandle->Malloc(SizeOfAllocaterInBytes);

		// Align the pointer
		uint8* AlignedAllocaterPtr = AlignPointerAndShift(RawAllocaterPtr, sizeof(T));

		// Calculate the data offset from heap start + the alignment that will get applied
		MemPageForAllocater->OffsetFromHeapStart = (MemoryHeapHandle->GetHeapUsed() - SizeOfAllocaterInBytes) + (uint8)AlignedAllocaterPtr[-1];
		MemPageForAllocater->Data = (uint8*)(new (AlignedAllocaterPtr) T((inArgs)...));
		MemPageForAllocater->MemorySize = SizeOfAllocaterInBytes;

		((T*)(MemPageForAllocater->Data))->Init(inSizeInBytesForAllocater, inAllocaterAlignment);

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		std::cout << "[Memory Manager] Memory Allocater Allocated, size in bytes: " << inSizeInBytesForAllocater + SizeOfAllocaterInBytes <<
			", with alignment: " << inAllocaterAlignment << "\n";
#endif

		return (T**)&MemPageForAllocater->Data;
	}

	/**
	* Frees the memory at the pointer passed in
	*
	* @param inPtrToMemory - Pointer to the memory to be freed
	*/
	void Free(void** inPtrToMemory)
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		MemoryPageHeapHandle->Free(sizeof(MemoryPage));
	}

	/**
	* Flushs the Heap, but doesn't delete memory
	*/
	void FlushNoDelete()
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		MemoryHeapHandle->FlushNoDelete();
		MemoryPageHeapHandle->FlushNoDelete();
	}

	/**
	* Shuts down the manager, Releases All allocated memory
	*/
	void Shutdown()
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		bIsActive = false;
		Flush();
	}

private:
	/**
	* Pre-Inits the memory manager with default memory until size if know to allocate more memory
	*	Should be called on launch
	*/
	void PreInit()
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

#if _DEBUG
		VE_ASSERT(MemoryHeapSize == 0, VE_TEXT("[Memory Manager]: Memory manager cannot initialize with 0 bytes as the size!"));
		VE_ASSERT(MemoryPageHeapSize == 0, VE_TEXT("[Memory Manager]: Memory managers page heap size cannot start with 0 bytes!"));
#endif
		MemoryHeapSize = MEBIBYTES_TO_BYTES(100);
		MemoryPageHeapSize = MEBIBYTES_TO_BYTES(50);

		MemoryHeapHandle = new TMemoryHeap<uint8>();
		MemoryHeapHandle->AllocateByBytes(MemoryHeapSize);

		MemoryPageHeapHandle = new TMemoryHeap<MemoryPage>();
		MemoryPageHeapHandle->AllocateByBytes(MemoryPageHeapSize);
	}

	/**
	* Aligns pointer and stores the shift [-1] of the pointer
	*
	* @param inPtrToAlign - the pointer that will be aligned
	* @param inAlignment - n-byte alignment
	* @returns uint8* - aligned pointer
	*/
	uint8* AlignPointerAndShift(uint8* inPtrToAlign, uint32 inAlignment)
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		// Align the block, if their isn't alignment, shift it up the full 'align' bytes, so we always 
		// have room to store the shift 
		uint8* AlignedPtr = MemoryUtils::AlignPointer<uint8>(inPtrToAlign, inAlignment);
		if (AlignedPtr == inPtrToAlign)
		{
			AlignedPtr += inAlignment;
		}

		// Determine the shift, and store it for later when freeing
		// (This works for up to 256-byte alignment.)
		intptr Shift = AlignedPtr - inPtrToAlign;
#if _DEBUG || _DEBUG_EDITOR
		VE_ASSERT(Shift > 0 && Shift <= 256, VE_TEXT("[Memory Manager]: invalid amount of bytes are trying to get shifted"));
#endif

		AlignedPtr[-1] = static_cast<uint8>(Shift & 0xff);

		return AlignedPtr;
	}

	/**
	* Frees/deletes all memory
	*/
	void Flush()
	{
#if VE_PROFILE_MEMORY_MANAGER
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_MANAGER

		if (MemoryHeapHandle != nullptr)
		{
			delete MemoryHeapHandle;
			MemoryHeapHandle = nullptr;
		}

		if (MemoryPageHeapHandle != nullptr)
		{
			delete MemoryPageHeapHandle;
			MemoryPageHeapHandle = nullptr;
		}
	}

public:
	/**
	* Returns amount of memory in use
	*/
	inline uint64 GetMemoryUsed() const
	{
		return MemoryHeapHandle->GetMemoryUsed() + MemoryPageHeapHandle->GetMemoryUsed();
	}

	inline uint64 GetAllocationsCount() const
	{
		return MemoryHeapHandle->GetMemoryAllocationCount();
	}

	inline bool GetIsActive() const
	{
		return bIsActive;
	}
};
