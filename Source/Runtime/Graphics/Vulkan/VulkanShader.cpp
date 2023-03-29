/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanDevice.h"
#include "VulkanShader.h"
#include <Misc/Defines/VulkanProfilerDefines.h>

/* ------------------------------------------------------------------------------- */
/* -----------------------          VulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShader::VulkanShader(VulkanDevice* inDevice, VkShaderStageFlagBits inShaderStageBits)
{
	VE_PROFILE_VULKAN_FUNCTION();

	ShaderKey = UINT32_MAX;
	Device = inDevice;
	ShaderStageBits = inShaderStageBits;
}

VulkanShader::~VulkanShader() { }

/* ------------------------------------------------------------------------------- */
/* -----------------------         TVulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

template<typename VkShaderStageFlagBits ShaderStageFlagBits>
TVulkanShader<ShaderStageFlagBits>::TVulkanShader(VulkanDevice* inDevice)
	: VulkanShader(inDevice, ShaderStageBits) { }

/* ------------------------------------------------------------------------------- */
/* -----------------------      VulkanShaderFactory      ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShaderFactory::VulkanShaderFactory(ResourceManager* inResourceManagerHandle)
{
	ResourceManagerHandle = inResourceManagerHandle;
}

VulkanShaderFactory::~VulkanShaderFactory() { }

VulkanVertexShader* VulkanShaderFactory::CreateVertexShaderFromPath(VulkanDevice* inDevice, const char* inShaderPath, bool inInvertY)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VulkanVertexShader* VertexShader = new VulkanVertexShader(inDevice);
	VertexShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromPath(inShaderPath, (uint32)EShaderType::Vertex, inInvertY);

	return VertexShader;
}

VulkanFragmentShader* VulkanShaderFactory::CreateFragmentShaderFromPath(VulkanDevice* inDevice, const char* inShaderPath, bool inInvertY)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VulkanFragmentShader* FragmentShader = new VulkanFragmentShader(inDevice);
	FragmentShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromPath(inShaderPath, (uint32)EShaderType::Fragment, inInvertY);

	return FragmentShader;
}

VulkanVertexShader* VulkanShaderFactory::CreateVertexShaderFromString(VulkanDevice* inDevice, const char* inShaderStr, bool inInvertY)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VulkanVertexShader* VertexShader = new VulkanVertexShader(inDevice);
	VertexShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromString(inShaderStr, (uint32)EShaderType::Vertex, inInvertY);

	return VertexShader;
}

VulkanFragmentShader* VulkanShaderFactory::CreateFragmentShaderFromString(VulkanDevice* inDevice, const char* inShaderStr, bool inInvertY)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VulkanFragmentShader* FragmentShader = new VulkanFragmentShader(inDevice);
	FragmentShader->ShaderKey = ResourceManagerHandle->CreateShaderResourceFromString(inShaderStr, (uint32)EShaderType::Fragment, inInvertY);

	return FragmentShader;
}
