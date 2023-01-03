#pragma once
#include "Misc/Defines/GenericDefines.h"
#include "Runtime/Graphics/Vulkan/VulkanDevice.h"

/* An interface of a resource manager */
class IResourceManager
{
public:
	virtual ~IResourceManager() { };

public:
	virtual uint32 CreateShaderResourceFromPath(const VString& inFilePath, uint32 inShaderType) = 0;
	virtual uint32 CreateShaderResourceFromString(const VString& inShaderStr, uint32 inShaderType) = 0;

	virtual const void* GetShaderModule(uint32 inShaderKey) const = 0;

	virtual void FreeAllMemory(VulkanDevice* inDevice) const = 0;
};

