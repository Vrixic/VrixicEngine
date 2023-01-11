#pragma once
#include "MemoryAllocater.h"

/**
* Stack Memory Alocater
*	- Allocates in a stack fashion
*	- Pointer to allocated memory inside the stack doedn't need to be kept tracked of as its done for you
*		at the cost of 4 bytes for every allocation
* 
* Memory Diagram = |Data|Size|Data|Size|
*/
class StackAllocater : public MemoryAllocater
{
public:
	StackAllocater(char* inMemoryHandle, uint128 inSizeInBytes) 
		: MemoryAllocater(inMemoryHandle, inSizeInBytes) { }

	virtual ~StackAllocater() { }

public:

	/**
	* Allocates memory
	*	Uses 4 more bytes so bytes usage = inSizeInBytes  + 4
	*
	* @param inSizeInBytes - how many bytes to allocate	*
	* @return T* pointer pointing to the allocated memory/allocated object
	*/
	template<typename T>
	T* Malloc(uint64 inSizeInBytes)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + (inSizeInBytes + sizeof(uint64)) < (MemorySize + 1));
#endif
		char* RawMemPtr = (MemoryHandle + MemoryUsed);
		T* MemHandle = (new (RawMemPtr) T()); // placement-new

		RawMemPtr += inSizeInBytes;
		*((uint64*)(RawMemPtr)) = inSizeInBytes;
		MemoryUsed += (inSizeInBytes + sizeof(uint64));

		return MemHandle;
	}

	/**
	* Frees the last allocated memory at the top of the stack
	*/
	virtual void Free()
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed != 0);
#endif
		MemoryUsed -= 4;
		uint64* PtrToSize = (uint64*)(MemoryHandle + MemoryUsed);
		MemoryUsed -= (*PtrToSize);
	}
};

/**
* Efficient stack memory allocater
*	Pointers need to be kept track of manually for freeing the stack, otherwise no way of knowing
*	how much to free except clearing
*/
class StackMemoryAllocaterE : public LinearAllocater
{
public:
	StackMemoryAllocaterE(char* inMemoryHandle, uint128 inSizeInBytes)
		: LinearAllocater(inMemoryHandle, inSizeInBytes) { }

	virtual ~StackMemoryAllocaterE() { }

public:

	/**
	* Frees the stack up until the pointer passed in the parameter
	* 
	* @param inPtrToMem - a pointer to the memory you want to free 
	*/
	virtual void Free(void* inPtrToMem)
	{
		uint64 MemSizeToFree = ((MemoryHandle + MemoryUsed) - ((char*)inPtrToMem));
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemSizeToFree < (MemorySize + 1));
#endif
		MemoryUsed -= MemSizeToFree;
	}
};
