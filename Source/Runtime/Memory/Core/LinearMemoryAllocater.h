#pragma once
//#include <Runtime/Memory/Core/MemoryAllocater.h>

/**
* Linear memory allocater
* -- Removed for right now, as no uses cases presented 
*/
//class LinearAllocater : public MemoryAllocater
//{
//public:
//	explicit LinearAllocater()
//		: MemoryAllocater() { }
//
//	virtual ~LinearAllocater() { }
//
//public:
//	/**
//	* Initializes the allocater 
//	*/
//	void Init(ulong32 inSizeInBytes, ulong32 inAlignment = 4)
//	{
//		MemoryAllocater::Init(inSizeInBytes, inAlignment);
//	}
//
//	/**
//	* Allocates memory, does not align memory, calls constructors.
//	* If this allocater is 32-byte aligned and requester asks for 32-bytes, it will return aligned memory 
//	*
//	* @param inSizeInBytes - how many bytes to allocate
//	*
//	* @return ulong32 - an index into the Data() pointer 
//	*/
//	template<typename T>
//	ulong32 Malloc(ulong32 inSizeInBytes)
//	{
//#if _DEBUG || _DEBUG_EDITOR || _EDITOR
//		ASSERT(MemoryUsed + inSizeInBytes < (MemorySize + 1));
//
//		AllocationCount++;
//#endif
//		T* MemHandle = (new ((Data() + MemoryUsed)) T()); // placement-new
//
//		uint32 Index = MemoryUsed;
//		MemoryUsed += inSizeInBytes;
//
//		return Index;
//	}
//};
