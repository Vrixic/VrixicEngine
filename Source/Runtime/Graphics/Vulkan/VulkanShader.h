#pragma once
#include "VulkanDevice.h"
#include "Runtime/Memory/ResourceManager.h"

/*
* Representation of a Shader in Vulkan
* Do not manually create these.. Use VulkanShaderFactory!
*/
class VulkanShader
{
protected:
	/* A Key to the shader module location into the array of shader modules */
	uint32 ShaderKey;

	VulkanDevice* Device;
	VkShaderStageFlagBits ShaderStageBits;

public:
	/**
	* @param inShaderStageBits - The type of shader ex. Vertex, Fragment, etc...
	*/
	VulkanShader(VulkanDevice* inDevice, VkShaderStageFlagBits inShaderStageBits);

	virtual ~VulkanShader();

	VulkanShader(const VulkanShader& other) = delete;
	VulkanShader operator=(const VulkanShader& other) = delete;
};

/* *
* Easier to just make a template rather than hand make all the shader types fragment, vertex, hull.....
* Only shader factory can create this class, Do not try creating a VulkanShader directly
* This will make it easier to keep track of all shaders and VkShaderModules later for deletion
* */
template<typename VkShaderStageFlagBits ShaderStageFlagBits>
class TVulkanShader : public VulkanShader
{
private:
	friend class VulkanShaderFactory;

private:
	TVulkanShader(VulkanDevice* inDevice);

	TVulkanShader(const TVulkanShader& other) = delete;
	TVulkanShader operator=(const TVulkanShader& other) = delete;
};

/* Alias for shader variables, makes it easier to create these types of shaders, less verbose */
typedef TVulkanShader<VK_SHADER_STAGE_VERTEX_BIT> VulkanVertexShader;
typedef TVulkanShader<VK_SHADER_STAGE_FRAGMENT_BIT> VulkanFragmentShader;

/* *
* Factory for creating all types of shaders, use this to create shaders
* A ResourceManager is associated with a VulkanShaderFactory, 1:1 correlation
*/
class VulkanShaderFactory
{
private:
	/* Handle to the resource manager that will be used to create all shaders within this factory */
	ResourceManager* ResourceManagerHandle;

public:
	/**
	* @param inResourceManagerHandle - The resource manager that will be in use by this factory to create shader modules/handles
	*/
	VulkanShaderFactory(ResourceManager* inResourceManagerHandle);
	virtual ~VulkanShaderFactory();

	VulkanShaderFactory(const VulkanShaderFactory& other) = delete;
	VulkanShaderFactory operator=(const VulkanShaderFactory& other) = delete;

public:
	/**
	* @param inShaderPath - the path to where the shader is located 
	* 
	* @return VulkanVertexShader* - handle to the vertex shader created
	*/
	VulkanVertexShader* CreateVertexShader(VulkanDevice* inDevice, const char* inShaderPath);
};
