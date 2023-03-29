/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once

#include "vulkan/vulkan.h"

#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MD platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") // add "shaderc_combined.lib" to the list of lib 
#endif

#include "../ResourceManagerImp.h"
#include "Runtime/Graphics/Vulkan/VulkanDevice.h"

class VRIXIC_API VulkanResourceManager : public IResourceManager
{
private:
	VulkanDevice* Device;
	std::vector<VkShaderModule> ShaderModules;

public:
	VulkanResourceManager(VulkanDevice* inDevice); 
	virtual ~VulkanResourceManager();

public:
	/**
	* @TODO: Create Better Functions to not restrict how shaders should be set up
	*/
	virtual uint32 CreateShaderResourceFromPath(const VString& inFilePath, uint32 inShaderType, bool inInvertY) override;

	virtual uint32 CreateShaderResourceFromString(const VString& inShaderCode, uint32 inShaderType, bool inInvertY) override;

	virtual const void* GetShaderModule(uint32 inShaderKey) const override;

	virtual void FreeAllMemory(VulkanDevice* inDevice) const override;

private:
	/**
	* Loads a shader module from file path
	*/
	VkResult LoadShaderModuleFromPath(const VString* inShaderPath, uint32 inShaderKind,
		const shaderc_compiler_t& inCompiler, const shaderc_compile_options_t& inOptions);

	VkResult LoadShaderModuleFromString(const VString* inShaderCode, uint32 inShaderKind,
		const shaderc_compiler_t& inCompiler, const shaderc_compile_options_t& inOptions);
};
