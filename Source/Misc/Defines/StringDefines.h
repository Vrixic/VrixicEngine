#pragma once
#include <Runtime/Core/Strings/StringHash.h>

// User Defining a literal -- _SHID = StringHashID
uint32 operator"" _SHID(const char* inString, size_t inLength)
{
	return StringHash::GetStringHash(inString);
}


/* VE_CONST_CHAR - work around to get the user defined literal working */
#define VE_CONST_CHAR(...) __VA_ARGS__
#define VE_TEXT(...) StringHash::GetStringFromHash(VE_CONST_CHAR(__VA_ARGS__)_SHID)
