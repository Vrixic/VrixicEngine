#pragma once
#include <Runtime/Memory/Core/MemoryAllocater.h>

/**
* Linear memory allocater
*/
class LinearAllocater : public MemoryAllocater
{
public:
	LinearAllocater(char* inMemoryHandle, uint128 inSize)
		: MemoryAllocater(inMemoryHandle, inSize) { }

	virtual ~LinearAllocater() { }

public:

	/**
	* Allocates memory, does not align memory
	*	For alignment, ask the MemManager to return the aligned memory
	*
	* @param inSizeInBytes - how many bytes to allocate
	*
	* @return T* pointer pointing to the allocated memory/allocated object
	*/
	template<typename T>
	T* Malloc(uint64 inSizeInBytes)
	{
#if _DEBUG | _DEBUG_EDITOR
		ASSERT(MemoryUsed + inSizeInBytes < (MemorySize + 1));
#endif
		T* MemHandle = (new ((MemoryHandle + MemoryUsed)) T()); // placement-new
		MemoryUsed += inSizeInBytes;

		return MemHandle;
	}
};