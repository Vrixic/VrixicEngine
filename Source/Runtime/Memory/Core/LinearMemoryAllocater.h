#pragma once
#include <Runtime/Memory/Core/MemoryAllocater.h>

/**
* Linear memory allocater
*/

class LinearMemoryAllocater : public MemoryAllocater
{
public:
	LinearMemoryAllocater()
		: MemoryAllocater() { }

	virtual ~LinearMemoryAllocater() { }

public:

	/**
	* Allocates memory
	*
	* @param inCount - How many objects to create
	*
	* @return char** pointer pointing to the allocated memory
	*/
	template<typename T>
	T** Malloc(uint32 inCount)
	{
		uint64 RequestedSize = sizeof(T) * inCount;
#if _DEBUG | _EDITOR
		ASSERT((MemoryUsed + MemoryUsedByMemInfo + RequestedSize + sizeof(MemoryInfo)) < MemorySize);
#endif
		MemoryInfo* MemInfo = (MemoryInfo*)((MemoryStartPtr + MemorySize) - MemoryUsedByMemInfo - sizeof(MemoryInfo));
		MemInfo->MemoryStartPtr = MemoryStartPtr + MemoryUsed;
		MemInfo->MemorySize = RequestedSize;

		MemoryUsed += RequestedSize;
		MemoryUsedByMemInfo += sizeof(MemoryInfo);

		return (T**)&MemInfo->MemoryStartPtr;
	}
};
