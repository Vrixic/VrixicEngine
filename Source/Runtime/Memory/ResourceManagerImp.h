#pragma once
#include "Misc/Defines/GenericDefines.h"
#include "Runtime/Graphics/Vulkan/VulkanDevice.h"

/* An interface of a resource manager */
class IResourceManager
{
public:
	virtual ~IResourceManager() { };

public:
	virtual uint32 CreateShaderResource(const VString& inFilePath) = 0;

	virtual const void* GetShaderModule(uint32 inShaderKey) const = 0;

	virtual void FreeAllMemory(VulkanDevice* inDevice) const = 0;
};

