/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "Misc/Defines/GenericDefines.h"

/* An interface of a resource manager */
class VRIXIC_API IResourceManager
{
public:
	virtual ~IResourceManager() { };

public:
	virtual uint32 CreateShaderResourceFromPath(const VString& inFilePath, uint32 inShaderType, bool inInvertY) = 0;
	virtual uint32 CreateShaderResourceFromString(const VString& inShaderStr, uint32 inShaderType, bool inInvertY) = 0;

	virtual const void* GetShaderModule(uint32 inShaderKey) const = 0;

	virtual void FreeAllMemory() const = 0;
};

