#pragma once
#include <Misc/Profiling/Profiler.h>
#include <Runtime/Memory/Core/MemoryManager.h>

/**
* @TODO: remake the allocater system, scrap double-ended stack buffer by default, also allocater should not keep in track of memory usage
*	- Linear -> should be straight forward, no way to free memory w/o fragmentation
*	- Stack -> only one way to free, from the top, though there could be a possible way to remove from middle or start by swaping pointers
*	- Double-Ended stack -> two ways to free memory, from front or back pointer
*	- Pool -> keeps track of objects for repooling
* 
*	- Add resize options 
*/

/**
* A memory allocater interface, has no functionality
*	-- Init() - should only be called once 
*
*	This should be extended but NOT USED
*/
class VRIXIC_API MemoryAllocater
{
protected:
	/** The size of the memory available to be used by this allocater */
	ulong32 MemorySize;

	/** The amount of memory in use by this allocater*/
	ulong32 MemoryUsed;

	/** Pointer to the allocater itself*/
	uint8** MemoryHandle;

#if _DEBUG || _DEBUG_EDITOR ||_DEBUG

	/** Number of allocations done on this allocater */
	uint64 AllocationCount = 0;
#endif

public:
	explicit MemoryAllocater()
		: MemoryHandle(nullptr), MemorySize(0), MemoryUsed(0) { }

	explicit MemoryAllocater(uint8** inMemoryHandle, ulong32 inMemorySize)
		: MemoryHandle(inMemoryHandle), MemorySize(inMemorySize), MemoryUsed(0) { }

	virtual ~MemoryAllocater() 
	{
#if VE_PROFILE_MEMORY_ALLOCATERS
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_ALLOCATERS

		if (MemoryHandle != nullptr && MemoryManager::Get().GetIsActive())
		{
			MemoryManager::Get().Free((void**)MemoryHandle);
		}		
	}

public:

	/**
	* Frees all memory in the allocater to be reused
	*	- Doesn't free the allocater itself
	*/
	virtual void Flush()
	{
		MemoryUsed = 0;
	}

protected:

	/**
	* Allocates memory for this allocater to use
	* SHOULD ONLY BE CALLED ONCE IF ALLOCATER IS STATICALLY MADE
	*
	* @param inSizeInBytes - amount of memory to allocate in bytes
	* @param inAlignment - alignment of the allocated memory, has to be of power 2, default = 4
	*/
	void Init(ulong32 inSizeInBytes, ulong32 inAlignment = 4)
	{
#if VE_PROFILE_MEMORY_ALLOCATERS
		VE_PROFILE_FUNCTION();
#endif // VE_PROFILE_MEMORY_ALLOCATERS

#if _DEBUG || _DEBUG_EDITOR || _EDITOR
		ASSERT(MemoryHandle == nullptr, "[MemoryAllocater]: MemoryHandle is nullptr, was MemoryManger deactivated???");
		ASSERT(inAlignment > 0, "[MemoryAllocater]: Memory alignment has be an unsigned integer");
#endif
		MemoryHandle = MemoryManager::Get().MallocAligned<uint8>(inSizeInBytes, inAlignment);
		MemorySize = inSizeInBytes;
	}

public:
	// Deprecated 
	/*template<typename T>
	inline T* Get(ulong32 inIndex)
	{
		return (T*)&(Data()[inIndex]);
	}*/

	/**
	* Returns the start of the allocater 
	*/
	inline uint8* Data() const
	{
		return *MemoryHandle;
	}

	/*
	* Returns how much memory this allocater is alloted
	*/
	inline ulong32 GetMemorySize() const
	{
		return MemorySize;
	}

	/**
	* Returns how much memory is in used currently
	*/
	inline virtual ulong32 GetMemoryUsed() const
	{
		return MemoryUsed;
	}
};
