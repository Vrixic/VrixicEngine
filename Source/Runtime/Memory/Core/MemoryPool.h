#pragma once
#include <Misc/Defines/GenericDefines.h>

#define MEBIBYTES_TO_BYTES(inMiB) inMiB * 1048576

/**
* A pool of memory, used by the Memory Manager
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

public:
	MemoryPool(uint64 inSizeInBytes)
		: MemoryUsed(0), MemoryUsedTotal(0)
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
	* Allocate memory based on the item and count, Calls constructors
	*
	* @param inNumCount - count of how many to allocate
	*
	* @return char* pointer pointing to the memory location
	*/
	template<typename T>
	T* Malloc(uint32 inNumCount)
	{
		uint64 RequestedSize = sizeof(T) * inNumCount;

		// Check if we can allocate enough memory
#if _DEBUG | _EDITOR
		ASSERT((MemoryUsed + RequestedSize) < PoolSize);
#endif

		char* PtrToMem = MemoryPtr + MemoryUsed;
		T* MemPtr = (new (PtrToMem) T()); // placement-new, since memory is already allocated
		MemoryUsed += RequestedSize;
		MemoryUsedTotal += RequestedSize;

		return MemPtr;
	}

	/**
	* Allocate memory based on the item and count, Does not call constructors
	*
	* @param inSizeInBytes - amount of memory to allocate
	*
	* @return char* pointer pointing to the memory location
	*/
	char* Malloc(uint64 inSizeInBytes)
	{
		// Check if we can allocate enough memory
#if _DEBUG | _EDITOR
		ASSERT((MemoryUsed + inSizeInBytes) < PoolSize);
#endif

		char* MemPtr = MemoryPtr + MemoryUsed;
		MemoryUsed += inSizeInBytes;
		MemoryUsedTotal += inSizeInBytes;

		return MemPtr;
	}

	/**
	* Resize the pool, allocates more memory, do not scale down
	*
	* @return char** pointer pointing to the memory location
	*/
	char* Resize(uint64 inSizeInBytes)
	{
		uint64 LastPoolSize = PoolSize;
		PoolSize = inSizeInBytes;

#if _DEBUG | _EDITOR
		ASSERT(PoolSize > LastPoolSize);
#endif // _DEBUG | _EDITOR

		char* NewMemoryPtr = new char[PoolSize];
		memcpy(NewMemoryPtr, MemoryPtr, MemoryUsed);

		delete[] MemoryPtr;

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
