/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

//#pragma once
//#include "MemoryAllocater.h"
//
///**
//* Double-Ended Stack Allocater
//*	- Allocates from the bottom and top
//*	- Every allocation costs an extra 4-bytes for keeping in track of last item on stack
//*/
//class DoubleEndedStackAllocater : public MemoryAllocater
//{
//protected:
//	uint64 MemoryUsedFromTop;
//
//public:
//	DoubleEndedStackAllocater(uint8* inMemoryHandle, uint64 inSizeInBytes, uint64 inOffsetToData)
//		: MemoryAllocater(inMemoryHandle, inSizeInBytes, inOffsetToData), MemoryUsedFromTop(0) { }
//
//	virtual ~DoubleEndedStackAllocater() { }
//
//public:
//	/**
//	* Allocates memory from the bottom of the stack
//	*	Uses 4 more bytes so bytes usage = inSizeInBytes  + 4
//	*
//	* @param inSizeInBytes - how many bytes to allocate	*
//	* @return T* pointer pointing to the allocated memory/allocated object
//	*/
//	template<typename T>
//	T* MallocBottom(uint64 inSizeInBytes)
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT((MemoryUsed + MemoryUsedFromTop) + (inSizeInBytes + sizeof(uint64)) < (MemorySize + 1));
//#endif
//		char* RawMemPtr = (MemoryHandle + MemoryUsed);
//		T* MemHandle = (new (RawMemPtr) T()); // placement-new
//
//		RawMemPtr += inSizeInBytes;
//		*((uint64*)(RawMemPtr)) = inSizeInBytes;
//		MemoryUsed += (inSizeInBytes + sizeof(uint64));
//
//		return MemHandle;
//	}
//
//	/**
//	* Allocates memory from the top of the stack
//	*	Uses 4 more bytes so bytes usage = inSizeInBytes  + 4
//	*
//	* @param inSizeInBytes - how many bytes to allocate	*
//	* @return T* pointer pointing to the allocated memory/allocated object
//	*/
//	template<typename T>
//	T* MallocTop(uint64 inSizeInBytes)
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT((MemoryUsed + MemoryUsedFromTop) + (inSizeInBytes + sizeof(uint64)) < (MemorySize + 1));
//#endif
//		char* RawMemPtr = (MemoryHandle + ((MemorySize - MemoryUsedFromTop) - inSizeInBytes));
//		T* MemHandle = (new (RawMemPtr) T()); // placement-new
//
//		RawMemPtr -= sizeof(uint64);
//		*((uint64*)(RawMemPtr)) = inSizeInBytes;
//		MemoryUsedFromTop += (inSizeInBytes + sizeof(uint64));
//
//		return MemHandle;
//	}
//
//	/**
//	* Frees the last allocation added from the top of the stack
//	*/
//	virtual void FreeTop()
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT(MemoryUsedFromTop != 0);
//#endif
//		uint64* PtrToSize = (uint64*)(MemoryHandle + (MemorySize - MemoryUsedFromTop));
//		MemoryUsedFromTop -= (*PtrToSize) + sizeof(uint64);
//	}
//
//	/**
//	* Frees the last allocation added from the bottom of the stack
//	*/
//	virtual void FreeBottom()
//	{
//#if _DEBUG | _DEBUG_EDITOR
//		ASSERT(MemoryUsed != 0);
//#endif
//		MemoryUsed -= 4;
//		uint64* PtrToSize = (uint64*)(MemoryHandle + MemoryUsed);
//		MemoryUsed -= (*PtrToSize);
//	}
//
//	virtual void Flush() override
//	{
//		MemoryAllocater::Flush();
//		MemoryUsedFromTop = 0;
//	}
//
//	inline virtual uint64 GetMemoryUsed() const override
//	{
//		return MemoryUsed + MemoryUsedFromTop;
//	}
//};

