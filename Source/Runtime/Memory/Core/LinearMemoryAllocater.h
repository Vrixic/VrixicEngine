#pragma once
#include <Runtime/Memory/Core/MemoryAllocater.h>

/**
* Linear memory allocater
*/

class LinearMemoryAllocater : public MemoryAllocater
{
public:
	LinearMemoryAllocater(char* inStart, uint64 inSize)
		: MemoryAllocater(inStart, inSize) { }

	/**
	* Allocates memory
	*
	* @param inCount - How many objects to create
	*
	* @return char** pointer pointing to the allocated memory
	*/
	template<typename T>
	const char** Malloc(uint32 inCount)
	{
		uint64 RequestedSize = sizeof(T) * inCount;
#if _DEBUG | _EDITOR
		ASSERT(MemoryUsed + MemoryUsedByMemInfo + RequestedSize + sizeof(MemoryInfo) > Size);
#endif
		MemoryInfo* MemInfo = ((MemoryStartPtr + Size) - MemoryUsedByMemInfo - sizeof(MemoryInfo);
		MemInfo->MemoryStartPtr = MemoryStartPtr + MemoryUsed;
		MemInfo->Size = RequestedSize;

		MemoryUsed += RequestedSize;
		MemoryUsedByMemInfo += sizeof(MemoryInfo);

		return &MemInfo->MemoryStartPtr;
	}
};
