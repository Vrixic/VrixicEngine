#include "ResourceManager.h"

ResourceManager::ResourceManager(IResourceManager* inResourceManagerImp)
	: ResourceManagerImp(inResourceManagerImp) { }

ResourceManager::~ResourceManager() { }

uint32 ResourceManager::CreateShaderResource(const VString& inFilePath)
{
	return ResourceManagerImp->CreateShaderResource(inFilePath);
}

const void* ResourceManager::GetShaderModule(uint32 inShaderKey) const
{
	return ResourceManagerImp->GetShaderModule(inShaderKey);
}

void ResourceManager::FreeAllMemory(VulkanDevice* inDevice) const
{
	ResourceManagerImp->FreeAllMemory(inDevice);
}
