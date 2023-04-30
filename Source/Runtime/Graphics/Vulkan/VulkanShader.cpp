/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanShader.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Runtime/Graphics/VertexInputDescription.h>
#include "VulkanDevice.h"
#include "VulkanTypeConverter.h"

#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MD platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") // add "shaderc_combined.lib" to the list of lib 
#endif


/* ------------------------------------------------------------------------------- */
/* -----------------------          VulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShader::VulkanShader(VulkanDevice* inDevice, EShaderType inShaderType)
{
    ShaderPool = nullptr;
    ShaderKey = UINT32_MAX;
    Device = inDevice;
    ShaderType = inShaderType;
}

VulkanShader::~VulkanShader() { }

void VulkanShader::BuildInputLayout(uint32 inNumVertexDescriptions, const FVertexInputDescription* inVertexDescriptions)
{
    if (inNumVertexDescriptions == 0 || inVertexDescriptions == nullptr) return;

    std::vector<VkVertexInputBindingDescription> BindingDescriptions;
    for (uint32 i = 0; i < inNumVertexDescriptions; ++i)
    {
        const FVertexInputDescription& Description = inVertexDescriptions[i];

        VkVertexInputBindingDescription InputBindingDescription = { };
        InputBindingDescription.binding = Description.BindingNum;
        InputBindingDescription.stride = Description.Stride;
        InputBindingDescription.inputRate = Description.InputRate == EInputRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;

        InputBindings.push_back(InputBindingDescription);

        for (uint32 j = 0; j < Description.GetVertexAttributes().size(); j++)
        {
            const FVertexInputAttribute& Attribute = Description.GetVertexAttributes()[j];

            VkVertexInputAttributeDescription InputAttributeDescription = { };
            InputAttributeDescription.binding = Attribute.BindingNum;
            InputAttributeDescription.offset = Attribute.Offset;
            InputAttributeDescription.location = Attribute.Location;
            InputAttributeDescription.format = VulkanTypeConverter::Convert(Attribute.Format);

            InputAttributes.push_back(InputAttributeDescription);
        }
    }
}

void VulkanShader::CreateVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& inInputStateCreateInfo) const
{
    inInputStateCreateInfo = VulkanUtils::Initializers::PipelineVertexInputStateCreateInfo();

    inInputStateCreateInfo.vertexBindingDescriptionCount = InputBindings.size();
    inInputStateCreateInfo.pVertexBindingDescriptions = InputBindings.data();

    inInputStateCreateInfo.vertexAttributeDescriptionCount = InputAttributes.size();
    inInputStateCreateInfo.pVertexAttributeDescriptions = InputAttributes.data();
}

/* ------------------------------------------------------------------------------- */
/* -----------------------         TVulkanShader         ------------------------- */
/* ------------------------------------------------------------------------------- */

template<typename EShaderType TShaderType>
TVulkanShader<TShaderType>::TVulkanShader(VulkanDevice* inDevice)
    : VulkanShader(inDevice, TShaderType) { }

/* ------------------------------------------------------------------------------- */
/* -----------------------      VulkanShaderFactory      ------------------------- */
/* ------------------------------------------------------------------------------- */

VulkanShaderFactory::VulkanShaderFactory(VulkanDevice* inDevice/*, ResourceManager* inResourceManagerHandle*/)
{
    Device = inDevice;
    //ResourceManagerHandle = inResourceManagerHandle;
}

VulkanShaderFactory::~VulkanShaderFactory() { }

VulkanShader* VulkanShaderFactory::CreateShader(VulkanShaderPool* inShaderPool, const FShaderConfig& inConfig) const
{
    switch (inConfig.Type)
    {
    case EShaderType::Undefined:
        break;
    case EShaderType::Vertex:           return CreateVertexShader(inShaderPool, inConfig);
    case EShaderType::TessControl:
        break;
    case EShaderType::TessEvaluation:
        break;
    case EShaderType::Geometry:
        break;
    case EShaderType::Fragment:         return CreateFragmentShader(inShaderPool, inConfig);
    case EShaderType::Compute:
        break;
    default:
        break;
    }
    return nullptr;
}

VulkanVertexShader* VulkanShaderFactory::CreateVertexShader(VulkanShaderPool* inShaderPool, const FShaderConfig& inConfig) const
{
    VulkanVertexShader* VertexShader = nullptr;

    // Only type supported is string for now 
    if (inConfig.SourceType == EShaderSourceType::String)
    {
        // Then we have to compile the shader only
        const uint32* CompiledSourceCode = nullptr;
        uint32 CompiledSourceCodeSize = 0;
        CompileSourceCode(inConfig, CompiledSourceCode, &CompiledSourceCodeSize);

        VertexShader = new VulkanVertexShader(inShaderPool->Device);
        VertexShader->ShaderKey = inShaderPool->ShaderModuleHandles.size();
        VertexShader->ShaderPool = inShaderPool;

        // Create the shader module 
        inShaderPool->CreateShaderModule(CompiledSourceCode, CompiledSourceCodeSize);

        // Build the input layout information 
        VertexShader->BuildInputLayout(1, &inConfig.VertexBindings);

        // Free Resource -> 
        delete[] CompiledSourceCode;
    }

    return VertexShader;
}

VulkanFragmentShader* VulkanShaderFactory::CreateFragmentShader(VulkanShaderPool* inShaderPool, const FShaderConfig& inConfig) const
{
    VulkanFragmentShader* FragmentShader = nullptr;

    // Only type supported is string for now 
    if (inConfig.SourceType == EShaderSourceType::String)
    {
        // Then we have to compile the shader only
        const uint32* CompiledSourceCode = nullptr;
        uint32 CompiledSourceCodeSize = 0;
        CompileSourceCode(inConfig, CompiledSourceCode, &CompiledSourceCodeSize);

        FragmentShader = new VulkanFragmentShader(inShaderPool->Device);
        FragmentShader->ShaderKey = inShaderPool->ShaderModuleHandles.size();
        FragmentShader->ShaderPool = inShaderPool;

        // Create the shader module 
        inShaderPool->CreateShaderModule(CompiledSourceCode, CompiledSourceCodeSize);

        // Free Resource -> 
        delete[] CompiledSourceCode;
    }

    return FragmentShader;
}


void VulkanShaderFactory::CompileSourceCode(const FShaderConfig& inConfig, const uint32*& outCode, uint32* outCodeSize) const
{
    // Intialize runtime shader compiler HLSL -> SPIRV
    shaderc_compiler_t Compiler = shaderc_compiler_initialize();
    shaderc_compile_options_t CompilerOptions = shaderc_compile_options_initialize();

    if (inConfig.CompileFlags & FShaderCompileFlags::GLSL)
    {
        shaderc_compile_options_set_source_language(CompilerOptions, shaderc_source_language_glsl);
    }
    else
    {
        shaderc_compile_options_set_source_language(CompilerOptions, shaderc_source_language_hlsl);
    }

    // TODO: Part 3C
    shaderc_compile_options_set_invert_y(CompilerOptions, (inConfig.CompileFlags & FShaderCompileFlags::InvertY));
#ifdef _DEBUG
    shaderc_compile_options_set_generate_debug_info(CompilerOptions);
#endif

    shaderc_shader_kind ShaderKind = { };
    std::string InputFileName = "";
    std::string EntryPointName = inConfig.EntryPoint;
    switch (inConfig.Type)
    {
    case EShaderType::Vertex:
        ShaderKind = shaderc_vertex_shader;
        InputFileName = "main.vert";
        EntryPointName = "main";
        break;

    case EShaderType::Fragment:
        ShaderKind = shaderc_fragment_shader;
        InputFileName = "main.frag";
        //EntryPointName = "main";
        break;
    default:
        break;
    }

    shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
        Compiler, inConfig.SourceCode.c_str(), inConfig.SourceCode.length(),
        ShaderKind, InputFileName.c_str(), EntryPointName.c_str(), CompilerOptions);

    if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
    {
        VE_CORE_LOG_ERROR("Shader Errors: {0}", shaderc_result_get_error_message(result));
    }

    *outCodeSize = shaderc_result_get_length(result);

    outCode = new uint32[*outCodeSize];
    memcpy((void*)outCode, shaderc_result_get_bytes(result), *outCodeSize);

    shaderc_result_release(result); // done
}

VulkanShaderPool::VulkanShaderPool(VulkanDevice* inDevice)
    : Device(inDevice) { }

VulkanShaderPool::~VulkanShaderPool()
{
    Device->WaitUntilIdle();

    for (uint32 i = 0; i < ShaderModuleHandles.size(); i++)
    {
        vkDestroyShaderModule(*Device->GetDeviceHandle(), ShaderModuleHandles[i], nullptr);
    }
}

VkShaderModule VulkanShaderPool::CreateShaderModule(const uint32* inCompiledShaderCode, uint32 inCodeSize)
{
    VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
    ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ShaderModuleCreateInfo.codeSize = inCodeSize;
    ShaderModuleCreateInfo.pCode = inCompiledShaderCode;

    //Create the Shader Module and return the result of it.
    VkShaderModule ShaderModuleHandle = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateShaderModule(*Device->GetDeviceHandle(), &ShaderModuleCreateInfo, nullptr, &ShaderModuleHandle), "[VulkanShaderPool]: Failed to create a shader module from compiled shader source code!!");

    ShaderModuleHandles.push_back(ShaderModuleHandle);
    return ShaderModuleHandle;
}
