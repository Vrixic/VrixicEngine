#pragma once
#include <Misc/Defines/GenericDefines.h>

/**
* Information on how the memory block is spliced
*/
struct MemoryInfo
{
	// The pointer pointing to the start of the memory 
	char* MemoryStartPtr;

	// The size of the memory, used as offset to the end of memory
	uint64 MemorySize;
};

/**
* A basic memory allocater -> Given 10% more memory than it is asked, it will be sizeof(MemoryInfo) aligned. to make sure memory infos fit in correctly..
*	It is a double ended stack buffer with start and end pointers....
*	Extra memory is used to store memory info for defragmentation later on if need be....
*	MemoryInfos will be stored at the end of the double ended stack buffer..
* 
*	This should be extended but NOT USED
*/
class MemoryAllocater
{
protected:
	// Pointer to the start of the memory this allocater can use
	char* MemoryStartPtr;

	// The size of the memory available to be used by this allocater 
	uint64 MemorySize;

	// The amount of memory in use by this allocater
	uint64 MemoryUsed;

	// Amount of memory being used by memory infos
	uint64 MemoryUsedByMemInfo;

public:
	MemoryAllocater()
		: MemoryStartPtr(nullptr), MemorySize(0), MemoryUsed(0), MemoryUsedByMemInfo(0) { }

	virtual ~MemoryAllocater() { } 

public:
	virtual void Init(char* inStart, uint64 inSize)
	{
		MemoryStartPtr = inStart + sizeof(MemoryAllocater); // for this class
		MemorySize = inSize;
	}

	/**
	* Returns extra bytes required when giving this allocater some memory
	*	Extra 32 bytes counting this classes member variables 
	*	
	* @param inSize - the size of bytes this allocater will be given to use 
	*/
	/*static uint64 GetAllocationBytes(const uint64& inSize)
	{
		float AlignmentCheck = static_cast<float>(inSize * 0.1f) / sizeof(MemoryInfo);
		AlignmentCheck = ceilf(AlignmentCheck);

		return static_cast<uint64>(AlignmentCheck * sizeof(MemoryInfo)) + sizeof(MemoryAllocater);
	}*/

	/*
	* Returns how much memory this allocater is alloted
	*/
	inline uint64 GetMemorySize()
	{
		return MemorySize;
	}

	/**
	* Returns how much memory is in used currently
	*/
	inline uint64 GetMemoryUsed()
	{
		return MemoryUsed;
	}
};
