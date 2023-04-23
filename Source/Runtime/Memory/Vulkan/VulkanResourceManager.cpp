/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanResourceManager.h"

#include <fstream>

VulkanResourceManager::VulkanResourceManager(VulkanDevice* inDevice) : Device(inDevice) { }

VulkanResourceManager::~VulkanResourceManager()
{
	// Wait for device to be idle before freeing all of the resources
	Device->WaitUntilIdle();

	// Free shader modules
	for (uint32 i = 0; i < ShaderModules.size(); ++i)
	{
		vkDestroyShaderModule(*Device->GetDeviceHandle(), ShaderModules[i], nullptr);
	}
}

uint32 VulkanResourceManager::CreateShaderResourceFromPath(const VString& inFilePath, uint32 inShaderType, bool inInvertY)
{
	// Intialize runtime shader compiler HLSL -> SPIRV
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
	// TODO: Part 3C
	shaderc_compile_options_set_invert_y(options, inInvertY);
#ifndef NDEBUG
	shaderc_compile_options_set_generate_debug_info(options);
#endif

	VK_CHECK_RESULT(LoadShaderModuleFromPath(&inFilePath, inShaderType, compiler, options), "[VulkanResourceManager]: Failed to load shader module from path!");

	return ShaderModules.size() - 1;
}

uint32 VulkanResourceManager::CreateShaderResourceFromString(const VString& inShaderCode, uint32 inShaderType, bool inInvertY)
{
	// Intialize runtime shader compiler HLSL -> SPIRV
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
	// TODO: Part 3C
	shaderc_compile_options_set_invert_y(options, inInvertY);
#ifndef NDEBUG
	shaderc_compile_options_set_generate_debug_info(options);
#endif

	VK_CHECK_RESULT(LoadShaderModuleFromString(&inShaderCode, inShaderType, compiler, options), "[VulkanResourceManager]: Failed to load shader module from string!");

	return ShaderModules.size() - 1;
}

const void* VulkanResourceManager::GetShaderModule(uint32 inShaderKey) const
{
	return &ShaderModules[inShaderKey];
}

void VulkanResourceManager::FreeAllMemory() const
{
	Device->WaitUntilIdle();

	for (uint32 i = 0; i < ShaderModules.size(); ++i)
	{
		vkDestroyShaderModule(*Device->GetDeviceHandle(), ShaderModules[i], nullptr);
	}
}

VkResult VulkanResourceManager::LoadShaderModuleFromPath(const VString* inShaderPath, uint32 inShaderKind,
	const shaderc_compiler_t& inCompiler, const shaderc_compile_options_t& inOptions)
{
	std::ifstream FileHandle(inShaderPath->c_str());
	if (!FileHandle.is_open())
	{
#if _DEBUG
		VE_CORE_LOG_ERROR(FMT_STRING("ERROR: Shader Source File \"{0}\" Not Found!") , inShaderPath->data());
#endif // DEBUG
		return VK_ERROR_UNKNOWN;
	}

	FileHandle.seekg(0, std::ios::end);
	size_t Size = FileHandle.tellg();
	std::string ShaderSource(Size, ' ');
	FileHandle.seekg(0);
	FileHandle.read(&ShaderSource[0], Size);

	shaderc_shader_kind ShaderKind = { };
	VString InputFileName = "";
	VString EntryPointName = "";
	switch (inShaderKind)
	{
	case 0:
		ShaderKind = shaderc_vertex_shader;
		InputFileName = "main.vert";
		EntryPointName = "main";
		break;

	case 1:
		ShaderKind = shaderc_fragment_shader;
		InputFileName = "main.frag";
		EntryPointName = "main";
		break;
	default:
		break;
	}

	std::string ShaderSourceStr = std::string(ShaderSource);
	return LoadShaderModuleFromString(&ShaderSourceStr, inShaderKind, inCompiler, inOptions);
}

VkResult VulkanResourceManager::LoadShaderModuleFromString(const VString* inShaderCode, uint32 inShaderKind,
	const shaderc_compiler_t& inCompiler, const shaderc_compile_options_t& inOptions)
{
	shaderc_shader_kind ShaderKind = { };
	VString InputFileName = "";
	VString EntryPointName = "";
	switch (inShaderKind)
	{
	case 0:
		ShaderKind = shaderc_vertex_shader;
		InputFileName = "main.vert";
		EntryPointName = "main";
		break;

	case 1:
		ShaderKind = shaderc_fragment_shader;
		InputFileName = "main.frag";
		EntryPointName = "main";
		break;
	default:
		break;
	}

	shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
		inCompiler, inShaderCode->c_str(), inShaderCode->length(),
		ShaderKind, InputFileName.c_str(), EntryPointName.c_str(), inOptions);
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
	{
		VE_CORE_LOG_ERROR("Shader Errors: {0}", shaderc_result_get_error_message(result));
	}

	VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
	ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCreateInfo.codeSize = shaderc_result_get_length(result);
	ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>((char*)shaderc_result_get_bytes(result));

	//Create the Shader Module and return the result of it.
	VkShaderModule ShaderModuleHandle = VK_NULL_HANDLE;
	VkResult Result = vkCreateShaderModule(*Device->GetDeviceHandle(), &ShaderModuleCreateInfo, nullptr, &ShaderModuleHandle);
	shaderc_result_release(result); // done

	ShaderModules.push_back(ShaderModuleHandle);

	//VkResult Result = VK_ERROR_COMPRESSION_EXHAUSTED_EXT;
	return Result;
}
