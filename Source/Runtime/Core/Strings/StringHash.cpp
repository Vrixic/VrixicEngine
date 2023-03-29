/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "StringHash.h"

TMap<uint32, const char*> StringHash::StringMap = TMap<uint32, const char*>();

StringHash::StringHash(const char* inString)
{
	StringId = GetStringHash(inString);
}

uint32 StringHash::GetStringHash(const char* inString)
{
	uint32 SID = CRC32::crc32buf(inString, strlen(inString));
	std::unordered_map<uint32, const char*>::iterator It = StringMap.Find(SID);

	if (It == StringMap.End())
	{
		// This string has not yet been added to the
		// table. Add it, being sure to copy it in case
		// the original was dynamically allocated and
		// might later be freed
		StringMap.Add(SID, strdup(inString));
	}

	return SID;
}

const char* StringHash::GetStringFromHash(uint32 inHashedString)
{
	const char* String = nullptr;
	std::unordered_map<uint32, const char*>::iterator It = StringMap.Find(inHashedString);
	if (It != StringMap.End())
	{
		String = It->second;
	}

	return String;
}
