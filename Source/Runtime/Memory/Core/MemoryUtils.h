/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Assert.h>

#define MEBIBYTES_TO_BYTES(inMiB) inMiB * 1048576

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
		const uint64 Mask = inAlignment - 1;
#if _DEBUG || _DEBUG_EDITOR
		ASSERT((inAlignment & Mask) == 0, "All addresses to be aligned has to be of a power of 2"); // Power of 2
#endif
		return (inAddress + Mask) & ~Mask;
	}
};
