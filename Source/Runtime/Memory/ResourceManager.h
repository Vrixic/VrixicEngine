#pragma once
#include "ResourceManagerImp.h"

/**
* Base Resource manager class which different types of resource manager can inherit from
*/
class ResourceManager
{
protected: 
	/* Implementation of the resource manager */
	IResourceManager* ResourceManagerImp;

public:
	ResourceManager(IResourceManager* inResourceManagerImp);

	virtual ~ResourceManager();

public:
	/**
	* Creates a shader resource 
	* 
	* @Param inFilePath - file path to the shader location
	* 
	* @Return uint32 - the key to where the shader handle is located
	*/
	uint32 CreateShaderResource(const VString& inFilePath);

	/**
	* Gets the shader module
	* 
	* @Param inShaderKey - the key to a shader handle
	* 
	* @Return void* - the shader module
	*/
	const void* GetShaderModule(uint32 inShaderKey) const;

	/**
	* Frees all memory used by the device 
	*/
	void FreeAllMemory(VulkanDevice* inDevice) const;
};
