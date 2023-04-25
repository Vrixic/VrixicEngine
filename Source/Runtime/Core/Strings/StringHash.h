/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once

#include <Containers/Map.h>
#include <Misc/Defines/GenericDefines.h>
#include <Runtime/Core/Algorithms/Hashing/CRC32.h>

/**
* Interns a string into a hash, which increases performance in memory usage 
* 
* Only 4 bytes consumed by the string 
*/
class VRIXIC_API StringHash
{
public:
	StringHash(const char* inString);

	/**
	* Creates a hashed string from a const char*, if string is new, then it is added to the map
	* 
	* @returns uint32 - a hashed string 
	*/
	static uint32 GetStringHash(const char* inString);

	/**
	* @returns const char* - the string from a hashed value 
	*/
	static const char* GetStringFromHash(uint32 inHashedString);

public:
	inline const char* GetConstCharPtr() const
	{
		return GetStringFromHash(StringId);
	}

private:
    static TMap<uint32, const char*> StringMap;

    /** Id to the current string */
    uint32 StringId; 
};

