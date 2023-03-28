#include "ResourceManager.h"

ResourceManager::ResourceManager(IResourceManager* inResourceManagerImp)
	: ResourceManagerImp(inResourceManagerImp) { }

ResourceManager::~ResourceManager() { }

uint32 ResourceManager::CreateShaderResourceFromPath(const VString& inFilePath, uint32 inShaderType, bool inInvertY)
{
	return ResourceManagerImp->CreateShaderResourceFromPath(inFilePath, inShaderType, inInvertY);
}

uint32 ResourceManager::CreateShaderResourceFromString(const VString& inShaderStr, uint32 inShaderType, bool inInvertY)
{
	return ResourceManagerImp->CreateShaderResourceFromString(inShaderStr, inShaderType, inInvertY);
}

const void* ResourceManager::GetShaderModule(uint32 inShaderKey) const
{
	return ResourceManagerImp->GetShaderModule(inShaderKey);
}

void ResourceManager::FreeAllMemory(VulkanDevice* inDevice) const
{
	ResourceManagerImp->FreeAllMemory(inDevice);
}
