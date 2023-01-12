#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Memory/Core/MemoryAllocater.h>
#include <Runtime/Memory/Core/MemoryPool.h>

#include <iostream>
#include <type_traits>

/*
* @TODO:
*	- 256 alignment restriction solution? Special case solution maybe.
*	- Find a way to keep track of freed memory in O(1) time complexity (constant time)
*		- As of right now memory is 'freed' by setting te pointer to nullptr for indetification
*	- Differentiate between game and editor memory heaps
*/

/**
* Represents a freed memory location/block/Node, 16-byte aligned
*
*/
struct FreeMemoryNode
{
	/** Pointer to the memory location */
	char* MemoryPtr;

	/** Size of memory */
	uint64 Size;

	/** How far from this node is the next node in bytes */
	uint64 Next;
};

/**
* Singleton
*
* Memory Manager - just a general purpose heap
*/
class MemoryManager
{
private:
	/* Instance for singleton */
	//static MemoryManager* Instance;

	// PostInit should only be called once 
	bool HasPostInitialized;

	/* Memory handle to the main block of memory, main memory pool */
	MemoryPool* MemoryPoolHandle;

	/* amount of memory allocated for main memory pool */
	uint128 MemoryAllocationSize;

	/* Memory handle to the pool for memory infos */
	MemoryPool* MemoryInfoPoolHandle;

	/* amount of memory allocated for memory info pool */
	uint128 MemoryInfoAllocationSize;

public:
	MemoryManager() : HasPostInitialized(false), MemoryPoolHandle(nullptr), MemoryAllocationSize(0),
		MemoryInfoPoolHandle(nullptr), MemoryInfoAllocationSize(0) { }

	~MemoryManager() { }

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
#if _DEBUG
		ASSERT(!HasPostInitialized);
#endif
		uint64 LastAllocationSize = MemoryAllocationSize;
		MemoryAllocationSize = MEBIBYTES_TO_BYTES(inSizeInMebibytes);

		// Re-Allocate more memory, copy of data is done by the pool
		char* OldMemPtr = nullptr;
		char* NewMemPtr = MemoryPoolHandle->Resize(MemoryAllocationSize, OldMemPtr);

		// Update the memory infos to point to the new memory 
		char* MemInfoMemPtr = MemoryInfoPoolHandle->GetMemoryHandle();
		uint64 MemInfoByteOffset = MemoryInfoPoolHandle->GetByteOffsetFromStart();
		uint64 ByteOffsetFromStart = 0;

		while (MemInfoByteOffset != ByteOffsetFromStart)
		{
			MemoryInfo* MemInfo = (MemoryInfo*)(MemInfoMemPtr + ByteOffsetFromStart);
			// Extract the shift preform for memory alignment
			intptr Shift = ((uint8*)MemInfo->MemoryStartPtr)[-1];
			if (Shift == 0)
			{
				Shift = 256;
			}
			MemInfo->MemoryStartPtr = (NewMemPtr + Shift);

			ByteOffsetFromStart += sizeof(MemoryInfo);

			NewMemPtr += MemInfo->MemorySize;
		}

		delete[] OldMemPtr;
		HasPostInitialized = true;
	}

	template<typename T>
	inline T* AlignPointer(T* inPtr, uint64 inAlignment)
	{
		const uintptr Address = reinterpret_cast<uintptr>(inPtr);
		const uintptr AddressAligned = AlignAddress(Address, inAlignment);
		return reinterpret_cast<T*>(AddressAligned);
	}

	/**
	* Aligns the address: Shifts the given address upwards if/as necessary to ensure it's aligned to the given
	* number of byes
	*/
	inline uintptr AlignAddress(uintptr inAddress, uint64 inAlignment)
	{
		const uint128 Mask = inAlignment - 1;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT((inAlignment & Mask) == 0); // Power of 2
#endif
		return (inAddress + Mask) & ~Mask;
	}

	/**
	* Allocate memory based on the count, also align pointer by sizeof(T),
	* sizeof(T) must be a power of 2
	*
	* @param inNumCount - count of how many to allocate
	*
	* @return char** pointer pointing to the memory location
	*/
	template<typename T>
	T** MallocAligned(uint128 inSizeInBytes, uint64 inAlignment = sizeof(T*))
	{
		// check if the last allocated item was freed, if so, just used that 
		// otherwise alloc more memory for it 
		char* LastMemInfo = (MemoryInfoPoolHandle->GetMemoryHandle() + MemoryInfoPoolHandle->GetByteOffsetFromStart()) - sizeof(MemoryInfo);
		MemoryInfo* MemInfo = (MemoryInfo*)LastMemInfo;
		if (MemInfo->MemoryStartPtr != nullptr)
		{
			MemInfo = MemoryInfoPoolHandle->Malloc<MemoryInfo>(1);
		}

		// Find the worse case number of bytes we might have to shift
		inSizeInBytes += inAlignment; // allocate extra

		uint8* RawMemPtr = (uint8*)MemoryPoolHandle->Malloc(inSizeInBytes);

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		std::cout << "[Memory Manager] Memory Allocated, size in bytes: " << inSizeInBytes <<
			", with alignment: " << inAlignment << "\n";
#endif

		// Align the block, if their isn't alignment, shift it up the full 'align' bytes, so we always 
		// have room to store the shift 
		uint8* AlignedPtr = AlignPointer<uint8>(RawMemPtr, inAlignment);
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

		MemInfo->MemoryStartPtr = (char*)(new (AlignedPtr) T());
		MemInfo->MemorySize = inSizeInBytes;

		return (T**)&MemInfo->MemoryStartPtr;
	}

	/**
	* Allocate memory for allocater and also create one, calls constructor
	*
	* @param inSizeInBytes - memory size the allocater will get
	*
	* @return char** pointer pointing to the allocater
	*/
	template<typename T>
	T** MallocAllocaterAligned(uint128 inSizeInBytes, uint64 inAlignment = sizeof(T))
	{
		// Check if we can allocate enough memory
#if _DEBUG | _DEBUG_EDITOR
		bool IsAllocater = std::is_base_of<MemoryAllocater, T>::value;
		ASSERT(IsAllocater);
#endif

		MemoryInfo* MemInfo = MemoryInfoPoolHandle->Malloc<MemoryInfo>(1);

		// Find the worse case number of bytes we might have to shift
		uint128 ClassPaddedSize = inSizeInBytes + inAlignment + sizeof(T);

		uint8* RawMemPtr = (uint8*)MemoryPoolHandle->Malloc(ClassPaddedSize);
		// Align the block, if their isn't alignment, shift it up the full 'align' bytes, so we always 
		// have room to store the shift 
		uint8* AlignedPtr = AlignPointer<uint8>(RawMemPtr, inAlignment);
		if (AlignedPtr == RawMemPtr)
		{
			AlignedPtr += inAlignment;
		}

		// Determine the shift, and store it for later when freeing
		// (This works for up to 256-byte alignment.)
		intptr Shift = AlignedPtr - RawMemPtr;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT(Shift > 0 && Shift <= 256);

		std::cout << "[Memory Manager] Memory Allocater allocated, size requested: " << inSizeInBytes
			<< ", with alignment: " << inAlignment << "\n";
#endif

		AlignedPtr[-1] = static_cast<uint8>(Shift & 0xff);

		MemInfo->MemoryStartPtr = (char*)(new (AlignedPtr) T((char*)AlignedPtr + sizeof(T), inSizeInBytes));
		MemInfo->MemorySize = ClassPaddedSize;

		return (T**)&MemInfo->MemoryStartPtr;
	}

	/**
	* Frees the memory at the pointer passed in
	*
	* @param inPtrToMemory - Pointer to the memory to be freed
	*/
	void Free(void** inPtrToMemory)
	{
		*inPtrToMemory = nullptr;
		MemoryInfoPoolHandle->Free(sizeof(MemoryInfo));
	}

	/**
	* Flushs the Heap, but doesn't delete memory
	*/
	void FlushNoDelete()
	{
		MemoryPoolHandle->FlushNoDelete();
		MemoryInfoPoolHandle->FlushNoDelete();
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
		ASSERT(MemoryAllocationSize == 0);
		ASSERT(MemoryInfoAllocationSize == 0);
#endif
		MemoryAllocationSize = MEBIBYTES_TO_BYTES(100);
		MemoryInfoAllocationSize = MEBIBYTES_TO_BYTES(50);

		MemoryPoolHandle = new MemoryPool(MemoryAllocationSize);
		MemoryInfoPoolHandle = new MemoryPool(MemoryInfoAllocationSize);
	}


	/**
	* Frees/deletes all memory
	*/
	void Flush()
	{
		delete MemoryPoolHandle;
		delete MemoryInfoPoolHandle;
	}

public:
	/**
	* Returns amount of memory in use
	*/
	uint128 GetMemoryUsed()
	{
		return MemoryPoolHandle->GetMemoryUsed() + MemoryInfoPoolHandle->GetMemoryUsed();
	}
};