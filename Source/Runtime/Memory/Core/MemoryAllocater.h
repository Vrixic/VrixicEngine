#pragma once
#include <Misc/Defines/GenericDefines.h>

/**
* @TODO: remake the allocater system, scrap double-ended stack buffer by default, also allocater should not keep in track of memory usage
*	- Linear -> should be straight forward, no way to free memory w/o fragmentation
*	- Stack -> only one way to free, from the top, though there could be a possible way to remove from middle or start by swaping pointers
*	- Double-Ended stack -> two ways to free memory, from front or back pointer
*	- Pool -> keeps track of objects for repooling 
*/

/**
* Information on how the memory block is spliced
*/
struct MemoryInfo
{
	// The size of the memory, used as offset to the end of memory
	uint128 MemorySize;

	// The pointer pointing to the start of the memory 
	char* MemoryStartPtr;
};

/**
* A memory allocater interface, has no functionality 
* 
*	This should be extended but NOT USED
*/
class MemoryAllocater
{
protected:
	// The size of the memory available to be used by this allocater 
	uint128 MemorySize;

	// The amount of memory in use by this allocater
	uint128 MemoryUsed;

	// Pointer to the start of the memory this allocater can use
	char* MemoryHandle;

public:
	MemoryAllocater(char* inMemoryHandle, uint128 inSize)
	{ 
		MemoryHandle = inMemoryHandle;
		MemorySize = inSize;

		MemoryUsed = 0;
	}

	virtual ~MemoryAllocater() { } 

	/**
	* Frees all memory in the allocater to be reused
	*	- Doesn't free the allocater itself
	*/
	virtual void Flush()
	{
		MemoryUsed = 0;
	}

public:

	/*
	* Returns how much memory this allocater is alloted
	*/
	inline uint128 GetMemorySize()
	{
		return MemorySize;
	}

	/**
	* Returns how much memory is in used currently
	*/
	inline uint128 GetMemoryUsed()
	{
		return MemoryUsed;
	}
};
