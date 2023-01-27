#pragma once
#include <Misc/Defines/GenericDefines.h>

#define MEBIBYTES_TO_BYTES(inMiB) inMiB * 1048576

/**
* Static class that contains functions for manipulating memory/pointers
*/
struct MemoryUtils
{
	template<typename T>
	inline static T* AlignPointer(T* inPtr, uint32 inAlignment)
	{
		const uintptr Address = reinterpret_cast<uintptr>(inPtr);
		const uintptr AddressAligned = AlignAddress(Address, inAlignment);
		return reinterpret_cast<T*>(AddressAligned);
	}

	/**
	* Aligns the address: Shifts the given address upwards if/as necessary to ensure it's aligned to the given
	* number of byes
	*/
	inline static uintptr AlignAddress(uintptr inAddress, uint32 inAlignment)
	{
		const uint64 Mask = inAlignment - 1;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT((inAlignment & Mask) == 0); // Power of 2
#endif
		return (inAddress + Mask) & ~Mask;
	}
};
