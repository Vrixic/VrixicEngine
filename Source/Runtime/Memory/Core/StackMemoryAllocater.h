#pragma once
#include "MemoryAllocater.h"

/**
* Stack Memory Alocater
*	- Allocates in a stack fashion
*	- Only allocate in blocks of T
*/
template<typename T>
class StackBlockAllocater : public MemoryAllocater
{
private:
	ulong32 Top;

public:
	StackBlockAllocater() : MemoryAllocater(), Top(0) { }

	virtual ~StackBlockAllocater() { }

public:

	/**
	* Allocates T objects for stack to use
	* 
	* @param inCount - the count of objects to create
	* @param inAlignment - alignment of memory 
	*/
	virtual void Create(ulong32 inCount, ulong32 inAlignment = sizeof(T)) override
	{
		ulong32 Size = inCount * sizeof(T);
		MemoryAllocater::Create(Size, inAlignment);
	}

	/**
	* Allocates a block
	*
	* @param inSizeInBytes - how many bytes to allocate	*
	* @return uint32 - index to the block
	*/
	ulong32 Alloc()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + sizeof(T) < (MemorySize + 1));
#endif
		MemoryUsed += sizeof(T);
		return Top++;
	}

	/**
	* Allocates a block and calls constructor
	*
	* @param inSizeInBytes - how many bytes to allocate	*
	* @return uint32 - index to the block 
	*/
	ulong32 AllocConstruct()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + sizeof(T) < (MemorySize + 1));
#endif
		uint8* RawMemPtr = (*MemoryHandle + MemoryUsed);
		T* MemHandle = (new (RawMemPtr) T()); // placement-new

		MemoryUsed += sizeof(T);

		return Top++;
	}

	/**
	* Frees the last allocated memory at the top of the stack
	*/
	virtual void Free()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed != 0);
#endif
		MemoryUsed -= sizeof(T);
		Top--;
	}

	/**
	* Flushes memory, doesn't release it  
	*/
	virtual void Flush() override
	{
		MemoryAllocater::Flush();
		Top = 0;
	}

public:
	inline T* Get(ulong32 inIndex)
	{
		return &(((T*)(Data()))[inIndex]);
	}
};

///**
//* Efficient stack memory allocater
//*	Pointers need to be kept track of manually for freeing the stack, otherwise no way of knowing
//*	how much to free except clearing
//*/
//class StackMemoryAllocaterE : public LinearAllocater
//{
//public:
//	StackMemoryAllocaterE(uint8* inMemoryHandle, uint64 inSizeInBytes, uint64 inOffsetToData)
//		: LinearAllocater(inMemoryHandle, inSizeInBytes, inOffsetToData) { }
//
//	virtual ~StackMemoryAllocaterE() { }
//
//public:
//
//	/**
//	* Frees the stack up until the pointer passed in the parameter
//	* 
//	* @param inPtrToMem - a pointer to the memory you want to free 
//	*/
//	virtual void Free(void* inPtrToMem)
//	{
//		uint64 MemSizeToFree = ((MemoryHandle + MemoryUsed) - ((uint8*)inPtrToMem));
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT(MemSizeToFree < (MemorySize + 1));
//#endif
//		MemoryUsed -= MemSizeToFree;
//	}
//};
