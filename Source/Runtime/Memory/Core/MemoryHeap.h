#pragma once

#include <Misc/Defines/GenericDefines.h>

/**
* Information on how the memory block is spliced
*/
struct MemoryPage
{
	/** The size of the memory, used as offset to the end of memory */
	uint64 MemorySize;

	/** Amount of bytes form HeapStartPointer to MemoryStartPointer */
	uint64 OffsetFromHeapStart;

	/** The pointer pointing to the start of the memory / pointing to data */
	uint8* Data;
};

/**
* A chunk of memory on the Heap (Pool of memory)
* Does not defragment by itself
*/
class MemoryHeap
{
private:
	/** Pointer/Handle to the memory */
	uint8* MemoryHandle;

	/** Pointer to the end of Used memory */
	uint8* MemoryUsedPtr;

	/** The size of the allocated memory in bytes */
	uint64 HeapSize;

	/** Amount of memory in use in bytes */
	uint64 MemoryUsed;

	/** Amount of memory used on the Heap, from start to end (MemoryHandle to MemoryUsedPtr) */
	uint64 HeapUsed;

	/** Count of all allocations made to this heap */
	uint64 MemoryAllocationCount;

public:
	MemoryHeap(uint64 inSizeInBytesToAllocate)
		: MemoryUsed(0), MemoryAllocationCount(0), HeapUsed(0)
	{
		HeapSize = inSizeInBytesToAllocate;
		MemoryHandle = new uint8[inSizeInBytesToAllocate];

		MemoryUsedPtr = MemoryHandle;
	}

	~MemoryHeap()
	{
		if (MemoryHandle != nullptr)
		{
			delete[] MemoryHandle;
		}
	}

public:
	/**
	* Allocate memory based on the object, Does not call constructors
	*
	* @param inSizeInBytes - amount of memory to allocate
	*
	* @return T* - pointer pointing to the memory location
	*/
	template<typename T>
	T* Malloc(uint64 inSizeInBytes)
	{
		// Check if we can allocate enough memory
#if _DEBUG | _DEBUG_EDITOR
		ASSERT((HeapUsed + inSizeInBytes) < HeapSize);
#endif

		uint8* PointerToMemory = MemoryUsedPtr;

		MemoryUsedPtr = MemoryUsedPtr + inSizeInBytes;
		MemoryUsed += inSizeInBytes;
		HeapUsed += inSizeInBytes;

		MemoryAllocationCount++;

		return (T*)PointerToMemory;
	}

	/**
	* Resize the pool, allocates more memory, do not scale down,
	* Frees last memory heap
	*
	* @return uint8* pointer pointing to the new memory location
	*/
	uint8* ResizeAndFlush(uint64 inSizeInBytes)
	{
		uint64 LastHeapSize = HeapSize;
		HeapSize = inSizeInBytes;

#if _DEBUG | _DEBUG_EDITOR
		ASSERT(HeapSize > LastHeapSize);
#endif // _DEBUG | _EDITOR

		uint8* NewMemoryPtr = new uint8[HeapSize];
		memcpy(NewMemoryPtr, MemoryHandle, HeapUsed);

		delete[] MemoryHandle;

		MemoryHandle = NewMemoryPtr;
		MemoryUsedPtr = MemoryHandle + HeapUsed;

		return MemoryHandle;
	}

	/**
	* Calculated total memory in used currently
	*/
	void Free(uint64 inSize)
	{
		MemoryUsed -= inSize;
	}

	/**
	* Flushs the Heap, but doesn't delete memory
	*/
	void FlushNoDelete()
	{
		MemoryUsed = 0;
		HeapUsed = 0;

		MemoryUsedPtr = MemoryHandle;
	}

	/**
	* Frees/deletes all memory
	*/
	void Flush()
	{
		if (MemoryHandle != nullptr)
		{
			delete[] MemoryHandle;
		}
	}

public:
	inline uint64 GetHeapSize() const
	{
		return HeapSize;
	}

	inline uint8* GetMemoryHandle() const
	{
		return MemoryHandle;
	}

	/**
	* @returns uint128 - memory in use, in bytes
	*/
	inline uint64 GetMemoryUsed() const
	{
		return MemoryUsed;
	}

	/**
	* @returns uint128 - memory used from start to current heap pointer (CurrentHeapPointer - StartHeapPointer)
	*/
	inline uint64 GetHeapUsed() const
	{
		return HeapUsed;
	}
};
