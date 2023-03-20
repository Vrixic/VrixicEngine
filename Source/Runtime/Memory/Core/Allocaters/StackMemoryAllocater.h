#pragma once
#include "MemoryAllocater.h"

/**
* Stack Allocater
*	- Allocates in a stack fashion
*	- Can be rolled back to a Marker 
*	- Insanely fast and efficient 
*/
class StackAllocater : public MemoryAllocater
{
public:
	typedef ulong32 Marker;

	explicit StackAllocater() : MemoryAllocater() { }

	virtual ~StackAllocater() { }

public:

	/**
	* Allocates T objects for stack to use
	*
	* @param inSizeInBytes - size of the stack 
	* @param inAlignment - alignment of memory, by default 16
	*/
	void Init(ulong32 inSizeInBytes, ulong32 inAlignment = 16)
	{
		MemoryAllocater::Init(inSizeInBytes, inAlignment);
	}

	/**
	* Allocates a block
	*
	* @param inSizeInBytesToAllocate - how many bytes to allocate
	* @return T* - pointer to the memory location
	*/
	template<typename T>
	T* Alloc(ulong32 inSizeInBytesToAllocate)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + inSizeInBytesToAllocate < (MemorySize + 1), "[Stack Memory Allocater]: Trying to allocate more bytes then have on allocater heap!");
#endif
		uint8* RawMemoryPtr = ((*MemoryHandle) + MemoryUsed);
		MemoryUsed += inSizeInBytesToAllocate;

#if _DEBUG || _DEBUG_EDITOR ||_DEBUG
		AllocationCount++;
#endif

		return (T*)RawMemoryPtr;
	}

	/**
	* Allocates a block and calls constructor
	*
	* @param inSizeInBytesToAllocate - how many bytes to allocate	*
	* @return T* - pointer to the memory location
	*/
	template<class T>
	T* AllocConstruct(ulong32 inSizeInBytesToAllocate)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + inSizeInBytesToAllocate < (MemorySize + 1), "[Stack Memory Allocater]: Trying to allocate more bytes then have on allocater heap!");
#endif
		uint8* RawMemPtr = (*MemoryHandle + MemoryUsed);
		T* MemHandle = (new (RawMemPtr) T()); // placement-new

		MemoryUsed += inSizeInBytesToAllocate;

#if _DEBUG || _DEBUG_EDITOR ||_DEBUG
		AllocationCount++;
#endif

		return MemHandle;
	}

	/**
	* Frees stack back to the marker supplied 
	* 
	* @param 'inMarker' - how much to free from top of stack
	*/
	virtual void FreeToMarker(Marker inMarker)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed != 0 && inMarker < (MemoryUsed + 1), "[Stack Memory Allocater]: Invalid marker being freed");
#endif
		MemoryUsed = inMarker;
	}

	/**
	* Flushes memory, doesn't release it.
	* AKA. Clear
	*/
	virtual void Flush() override
	{
		MemoryAllocater::Flush();
	}

	/**
	* Called when allocater release memory
	*/
	virtual void Dispose()
	{

	}

public:
	/** 
	* @returns 'Marker' - the current top stack position
	*/
	Marker GetMarker() const
	{
		return MemoryUsed;
	}
};


/**
* ----------------------------------------------------------------
* - Previous Implementation of StackBlockAllocater, allocates by -
* - indexes														 -
* ----------------------------------------------------------------
*/
/**
* Stack Block Allocater
*	- Allocates in a stack fashion
*	- Only allocate in blocks of T
*	- Offers block size allocations, making it very efficient and fast
*/

//template<typename T>
//class StackBlockAllocater : public MemoryAllocater
//{
//private:
//	ulong32 Top;
//
//public:
//	explicit StackBlockAllocater() : MemoryAllocater(), Top(0) { }
//
//	virtual ~StackBlockAllocater() { }
//
//public:
//
//	/**
//	* Allocates T objects for stack to use
//	*
//	* @param inCount - the count of objects to create
//	* @param inAlignment - alignment of memory
//	*/
//	virtual void Init(ulong32 inCount, ulong32 inAlignment = sizeof(T))
//	{
//		ulong32 Size = inCount * sizeof(T);
//		MemoryAllocater::Init(Size, inAlignment);
//	}
//
//	/**
//	* Allocates a block
//	*
//	* @param inSizeInBytes - how many bytes to allocate	*
//	* @return uint32 - index to the block
//	*/
//	ulong32 Alloc()
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT(MemoryUsed + sizeof(T) < (MemorySize + 1));
//#endif
//		MemoryUsed += sizeof(T);
//		return Top++;
//	}
//
//	/**
//	* Allocates a block and calls constructor
//	*
//	* @param inSizeInBytes - how many bytes to allocate	*
//	* @return uint32 - index to the block
//	*/
//	ulong32 AllocConstruct()
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT(MemoryUsed + sizeof(T) < (MemorySize + 1));
//#endif
//		uint8* RawMemPtr = (*MemoryHandle + MemoryUsed);
//		T* MemHandle = (new (RawMemPtr) T()); // placement-new
//
//		MemoryUsed += sizeof(T);
//
//		return Top++;
//	}
//
//	/**
//	* Frees the last allocated memory at the top of the stack
//	*/
//	virtual void Free()
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT(MemoryUsed != 0);
//#endif
//		MemoryUsed -= sizeof(T);
//		Top--;
//	}
//
//	/**
//	* Flushes memory, doesn't release it
//	*/
//	virtual void Flush() override
//	{
//		MemoryAllocater::Flush();
//		Top = 0;
//	}
//
//public:
//	inline T* Get(ulong32 inIndex)
//	{
//		return &(((T*)(Data()))[inIndex]);
//	}
//};
