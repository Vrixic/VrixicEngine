/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

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
public:
	MemoryPool(uint32 inSizeInBytes)
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
		uint32 RequestedSize = sizeof(T) * inNumCount;

		// Check if we can allocate enough memory
		VE_ASSERT((MemoryUsed + RequestedSize) < PoolSize);

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
	T* Malloc(uint32 inSizeInBytes)
	{
		// Check if we can allocate enough memory
		VE_ASSERT((MemoryUsed + inSizeInBytes) < PoolSize);

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
	char* ResizeAndFlush(uint32 inSizeInBytes)
	{
		uint32 LastPoolSize = PoolSize;
		PoolSize = inSizeInBytes;

		VE_ASSERT(PoolSize > LastPoolSize);

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
	char* Resize(uint32 inSizeInBytes, char* outLastMemHandle)
	{
		uint32 LastPoolSize = PoolSize;
		PoolSize = inSizeInBytes;

		VE_ASSERT(PoolSize > LastPoolSize);

		char* NewMemoryPtr = new char[PoolSize];
		memcpy(NewMemoryPtr, MemoryPtr, MemoryUsed);

		outLastMemHandle = MemoryPtr;
		MemoryPtr = NewMemoryPtr;

		return MemoryPtr;
	}

	/**
	* Calculated total memory in used currently
	*/
	void Free(uint32 inSize)
	{
		MemoryUsedTotal -= inSize;
	}

	/**
	* Frees memory from current location 
	* 
	* Frees last allocated object 
	*/
	void FreeLast(uint32 inSize)
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

	void AlignMemoryHandle(uint32 inAlignment)
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

		VE_ASSERT(Shift > 0 && Shift <= 256);

		AlignedPtr[-1] = static_cast<uint8>(Shift & 0xff);

		MemoryPtr = (char*)AlignedPtr;
	}

public:
	inline uint32 GetPoolSize() const
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
	inline uint32 GetMemoryUsed() const
	{
		return MemoryUsedTotal;
	}

	/**
	* Returns the current location of memory to be given next
	*/
	inline uint32 GetByteOffsetFromStart() const
	{
		return MemoryUsed;
	}

private:
    /** Pointer to the memory */
    char* MemoryPtr;

    /** size of the memory pool */
    uint32 PoolSize;

    /** locates the next memory block which can be given */
    uint32 MemoryUsed;

    /** total amount of memory being used */
    uint32 MemoryUsedTotal;

    /** Count of all allocations made to this Pool */
    uint32 MemoryAllocationsCount;
};

