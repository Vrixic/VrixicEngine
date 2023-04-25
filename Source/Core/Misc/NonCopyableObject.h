#pragma once
#include <Core/Core.h>

/*
* Should be a base class for all classes that cannot be copied
* @remarks Subclasses have their copy constructor and copy operator deleted
*/
class VRIXIC_API NonCopyableObject
{
public:
	NonCopyableObject(const NonCopyableObject&) = delete;
	NonCopyableObject& operator = (const NonCopyableObject&) = delete;

	virtual ~NonCopyableObject() = default;

protected:
	NonCopyableObject() = default;
};
