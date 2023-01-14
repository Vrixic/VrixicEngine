#pragma once
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Memory/Core/MemoryUtils.h>

/**
* A pool of memory, used by the Memory Manager
* Has no defragmentation options
* DEPRECATED
*/
class MemoryPool
{
private:
	/* Pointer to the memory */
	char* MemoryPtr;

	/* size of the memory pool */
	uint64 PoolSize;

	/* locates the next memory block which can be given */
	uint64 MemoryUsed;

	/* total amount of memory being used */
	uint64 MemoryUsedTotal;

	/* Count of all allocations made to this Pool */
	uint64 MemoryAllocationsCount;

public:
	MemoryPool(uint64 inSizeInBytes)
		: MemoryUsed(0), MemoryUsedTotal(0), MemoryAllocationsCount(0)
	{
		PoolSize = inSizeInBytes;
		MemoryPtr = new char[PoolSize];
	}

	~MemoryPool()
	{
		if (MemoryPtr != nullptr)
		{
			delete[] MemoryPtr;
		}
	}

public:
	/**
	* Allocate memory based on the item and count, Calls constructors of the class Allocated
	*
	* @param inNumCount - count of how many to allocate
	*
	* @return char* pointer pointing to the memory location
	*/
	template<typename T>
	T* MallocClass(uint32 inNumCount)
	{
		uint64 RequestedSize = sizeof(T) * inNumCount;

		// Check if we can allocate enough memory
#if _DEBUG | _DEBUG_EDITOR
		ASSERT((MemoryUsed + RequestedSize) < PoolSize);
#endif

		char* PtrToMem = MemoryPtr + MemoryUsed;
		T* MemPtr = (new (PtrToMem) T()); // placement-new, since memory is already allocated
		MemoryUsed += RequestedSize;
		MemoryUsedTotal += RequestedSize;

		MemoryAllocationsCount++;

		return MemPtr;
	}

	/**
	* Allocate memory based on the item and count, Does not call constructors
	*
	* @param inSizeInBytes - amount of memory to allocate
	*
	* @return char* pointer pointing to the memory location
	*/
	template<typename T>
	T* Malloc(uint64 inSizeInBytes)
	{
		// Check if we can allocate enough memory
#if _DEBUG | _DEBUG_EDITOR
		ASSERT((MemoryUsed + inSizeInBytes) < PoolSize);
#endif

		char* MemPtr = MemoryPtr + MemoryUsed;
		MemoryUsed += inSizeInBytes;
		MemoryUsedTotal += inSizeInBytes;

		MemoryAllocationsCount++;

		return (T*)MemPtr;
	}

	/**
	* Resize the pool, allocates more memory, do not scale down,
	* Frees last memory handle 
	*
	* @return char** pointer pointing to the memory location
	*/
	char* ResizeAndFlush(uint64 inSizeInBytes)
	{
		uint64 LastPoolSize = PoolSize;
		PoolSize = inSizeInBytes;

#if _DEBUG | _DEBUG_EDITOR
		ASSERT(PoolSize > LastPoolSize);
#endif // _DEBUG | _EDITOR

		char* NewMemoryPtr = new char[PoolSize];
		memcpy(NewMemoryPtr, MemoryPtr, MemoryUsed);

		delete[] MemoryPtr;

		MemoryPtr = NewMemoryPtr;

		return MemoryPtr;
	}

	/**
	* Resize the pool, allocates more memory, do not scale down
	* User in charge of freeing the memory from 'outLastMemHandle'
	*
	* @param outLastMemHandle - handle to the last memory location
	* @return char** pointer pointing to the memory location
	*/
	char* Resize(uint64 inSizeInBytes, char* outLastMemHandle)
	{
		uint64 LastPoolSize = PoolSize;
		PoolSize = inSizeInBytes;

#if _DEBUG | _DEBUG_EDITOR
		ASSERT(PoolSize > LastPoolSize);
#endif // _DEBUG | _EDITOR

		char* NewMemoryPtr = new char[PoolSize];
		memcpy(NewMemoryPtr, MemoryPtr, MemoryUsed);

		outLastMemHandle = MemoryPtr;
		MemoryPtr = NewMemoryPtr;

		return MemoryPtr;
	}

	/**
	* Calculated total memory in used currently
	*/
	void Free(uint64 inSize)
	{
		MemoryUsedTotal -= inSize;
	}

	/**
	* Frees memory from current location 
	* 
	* Frees last allocated object 
	*/
	void FreeLast(uint64 inSize)
	{
		MemoryUsed -= inSize;
		MemoryUsedTotal -= inSize;
	}

	/**
	* Flushs the Heap, but doesn't delete memory
	*/
	void FlushNoDelete()
	{
		MemoryUsed = 0;
		MemoryUsedTotal = 0;
	}

	/**
	* Frees/deletes all memory
	*/
	void Flush()
	{
		if (MemoryPtr != nullptr)
		{
			intptr Shift = ((uint8*)MemoryPtr)[-1];
			if (Shift == 0)
			{
				Shift = 256;
			}
			MemoryPtr = (MemoryPtr - Shift);

			delete[] MemoryPtr;
		}
	}

	void AlignMemoryHandle(uint64 inAlignment)
	{
		// Align the block, if their isn't alignment, shift it up the full 'align' bytes, so we always 
		// have room to store the shift 
		uint8* AlignedPtr = MemoryUtils::AlignPointer<uint8>((uint8*)MemoryPtr, inAlignment);
		if (AlignedPtr == (uint8*)MemoryPtr)
		{
			AlignedPtr += inAlignment;
		}

		// Determine the shift, and store it for later when freeing
		// (This works for up to 256-byte alignment.)
		intptr Shift = AlignedPtr - (uint8*)MemoryPtr;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT(Shift > 0 && Shift <= 256);
#endif

		AlignedPtr[-1] = static_cast<uint8>(Shift & 0xff);

		MemoryPtr = (char*)AlignedPtr;
	}

public:
	inline uint64 GetPoolSize() const
	{
		return PoolSize;
	}

	inline char* GetMemoryHandle() const
	{
		return MemoryPtr;
	}

	/**
	* Call free whenever memory is freed for accurate value
	*/
	inline uint64 GetMemoryUsed() const
	{
		return MemoryUsedTotal;
	}

	/**
	* Returns the current location of memory to be given next
	*/
	inline uint64 GetByteOffsetFromStart() const
	{
		return MemoryUsed;
	}
};

