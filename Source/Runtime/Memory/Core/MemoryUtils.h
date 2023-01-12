#pragma once
#include <Misc/Defines/GenericDefines.h>

/**
* Static class that contains functions for manipulating memory/pointers
*/
struct MemoryUtils
{
	template<typename T>
	inline static T* AlignPointer(T* inPtr, uint64 inAlignment)
	{
		const uintptr Address = reinterpret_cast<uintptr>(inPtr);
		const uintptr AddressAligned = AlignAddress(Address, inAlignment);
		return reinterpret_cast<T*>(AddressAligned);
	}

	/**
	* Aligns the address: Shifts the given address upwards if/as necessary to ensure it's aligned to the given
	* number of byes
	*/
	inline static uintptr AlignAddress(uintptr inAddress, uint64 inAlignment)
	{
		const uint128 Mask = inAlignment - 1;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT((inAlignment & Mask) == 0); // Power of 2
#endif
		return (inAddress + Mask) & ~Mask;
	}
};
