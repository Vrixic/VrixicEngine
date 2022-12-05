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

uint32 VulkanResourceManager::CreateShaderResource(const VString& inFilePath)
{
	// Intialize runtime shader compiler HLSL -> SPIRV
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
	// TODO: Part 3C
	shaderc_compile_options_set_invert_y(options, false);
#ifndef NDEBUG
	shaderc_compile_options_set_generate_debug_info(options);
#endif

	VK_CHECK_RESULT(LoadShaderModule(inFilePath.c_str(), "main.vert", "main", shaderc_vertex_shader, compiler, options));

	return ShaderModules.size() - 1;
}

const void* VulkanResourceManager::GetShaderModule(uint32 inShaderKey) const
{
	return &ShaderModules[inShaderKey];
}

void VulkanResourceManager::FreeAllMemory(VulkanDevice* inDevice) const
{
	inDevice->WaitUntilIdle();

	for (uint32 i = 0; i < ShaderModules.size(); ++i)
	{
		vkDestroyShaderModule(*inDevice->GetDeviceHandle(), ShaderModules[i], nullptr);
	}
}

VkResult VulkanResourceManager::LoadShaderModule(const char* inShaderPath, const char* inInputFileName, const char* inEntryPointName,
	shaderc_shader_kind inShaderKind, const shaderc_compiler_t& inCompiler, const shaderc_compile_options_t& inOptions)
{
	std::ifstream FileHandle(inShaderPath);
	if (!FileHandle.is_open())
	{
#if _DEBUG
		std::cout << "ERROR: Shader Source File \"" << inShaderPath << "\" Not Found!" << std::endl;
#endif // DEBUG
		return VK_ERROR_UNKNOWN;
	}

	FileHandle.seekg(0, std::ios::end);
	size_t Size = FileHandle.tellg();
	std::string ShaderSource(Size, ' ');
	FileHandle.seekg(0);
	FileHandle.read(&ShaderSource[0], Size);

	shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
		inCompiler, ShaderSource.c_str(), ShaderSource.length(),
		inShaderKind, inInputFileName, inEntryPointName, inOptions);
	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;

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
