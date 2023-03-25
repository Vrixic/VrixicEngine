#pragma once
#include "ResourceManagerImp.h"

/**
* Base Resource manager class which different types of resource manager can inherit from
*/
class VRIXIC_API ResourceManager
{
protected: 
	/* Implementation of the resource manager */
	IResourceManager* ResourceManagerImp;

public:
	ResourceManager(IResourceManager* inResourceManagerImp);

	virtual ~ResourceManager();

public:
	/**
	* Creates a shader resource from specified path
	* 
	* @param inFilePath - file path to the shader location
	* @param inShaderType - type of shader to create: vert, frag, etc...
	* 
	* @return uint32 - the key to where the shader handle is located
	*/
	uint32 CreateShaderResourceFromPath(const VString& inFilePath, uint32 inShaderType);

	/**
	* Creates a shader resource from specified shader code
	*
	* @param inShaderStr - shader code to be compiled and used
	* @param inShaderType - type of shader to create: vert, frag, etc...
	*
	* @return uint32 - the key to where the shader handle is located
	*/
	uint32 CreateShaderResourceFromString(const VString& inShaderStr, uint32 inShaderType);

	/**
	* Gets the shader module
	* 
	* @param inShaderKey - the key to a shader handle
	* 
	* @return void* - the shader module
	*/
	const void* GetShaderModule(uint32 inShaderKey) const;

	/**
	* Frees all memory used by the device 
	*/
	void FreeAllMemory(VulkanDevice* inDevice) const;
};
