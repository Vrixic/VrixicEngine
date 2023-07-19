/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Graphics/Shader.h>
#include "Runtime/Memory/ResourceManager.h"
#include "VulkanDevice.h"

#include <External/glslang/Include/glslang/Public/ShaderLang.h>
#include <External/glslang/Include/glslang/SPIRV/spirv.hpp>

struct FVertexInputDescription;

/**
* A shader pool
* Contains shader modules and is the only way to allocate/create shader modules specific to vulkan (VkShaderModule)
* Used by VulkanShaderFactory for creating shaders
*/
class VRIXIC_API VulkanShaderPool
{
public:
    VulkanShaderPool(VulkanDevice* inDevice);

    ~VulkanShaderPool();

private:
    VkShaderModule CreateShaderModule(const uint8* inCompiledShaderCode, uint64 inCodeSizeInBytes);

public:
    VkShaderModule GetShaderModule(uint32 inShaderKey) const
    {
        return ShaderModuleHandles[inShaderKey];
    }

private:
    friend class VulkanShaderFactory;

    VulkanDevice* Device;

    /** All shader module handles */
    std::vector<VkShaderModule> ShaderModuleHandles;
};

/**
* Representation of a Shader in Vulkan
* Do not manually create these.. Use VulkanShaderFactory!
*/
class VRIXIC_API VulkanShader : public Shader
{
public:
    /**
    * @param EShaderType - The type of shader ex. Vertex, Fragment, etc...
    */
    VulkanShader(VulkanDevice* inDevice, EShaderType inShaderType);

    virtual ~VulkanShader();

    VulkanShader(const VulkanShader& other) = delete;
    VulkanShader operator=(const VulkanShader& other) = delete;

    void BuildInputLayout(uint32 inNumVertexDescriptions, const FVertexInputDescription* inVertexDescriptions);

    void CreateVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& inInputStateCreateInfo) const;

    /* ------------------------------------------------------------------------------- */
    /* -------------        Shader Parsing Helper Functions        ------------------- */
    /* ------------------------------------------------------------------------------- */

    void ParseSpirvCodeIntoPipelineLayoutConfig(FPipelineLayoutConfig& outLayoutConfig) const;

    uint32 ParseSpvExecutionModel(spv::ExecutionModel inModel) const;

public:
    inline std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributes()
    {
        return InputAttributes;
    }

    inline std::vector<VkVertexInputBindingDescription>& GetVertexInputBindings()
    {
        return InputBindings;
    }

    inline uint32 GetShaderKey() const
    {
        return ShaderKey;
    }

    inline VkShaderModule GetShaderModuleHandle() const
    {
        return ShaderPool->GetShaderModule(ShaderKey);
    }

    inline const uint8* GetCompiledShaderBinary() const
    {
        return CompiledShaderBinary;
    }

    inline const uint32 GetCompiledShaderBinarySize() const
    {
        return CompiledShaderBinarySize;
    }

protected:
    VulkanDevice* Device;

    /** A Key to the shader module location into the array of shader modules inside of the pool that was used to create this shader  */
    uint32 ShaderKey;

    /** The shader pool that is used to create this shader */
    VulkanShaderPool* ShaderPool;

    uint8* CompiledShaderBinary;
    uint64 CompiledShaderBinarySize;

    std::vector<VkVertexInputAttributeDescription> InputAttributes;
    std::vector<VkVertexInputBindingDescription> InputBindings;

private:
    struct VRIXIC_API FMemberField
    {
        uint32 IdIndex;
        uint32 Offset;

        std::string Name;
    };

    struct VRIXIC_API FSpirvIdData
    {
        spv::Op SpvOp;
        uint32 Set;
        uint32 Binding;

        // Integers / Floats
        uint8 Width;
        uint8 Sign;

        // Arrays, Vectors, and Matrices
        uint32 TypeIndex;
        uint32 Count;

        // Variables
        spv::StorageClass SpvStorageClass;

        // Constants
        uint32 Value;

        // Structures
        std::string Name;
        std::vector<FMemberField> Members;
    };
};

/**
* Easier to just make a template rather than hand make all the shader types fragment, vertex, hull.....
* Only shader factory can create this class, Do not try creating a VulkanShader directly
* This will make it easier to keep track of all shaders and VkShaderModules later for deletion
* */
template<typename EShaderType TShaderType>
class TVulkanShader : public VulkanShader
{
private:
    TVulkanShader(VulkanDevice* inDevice);

    TVulkanShader(const TVulkanShader& other) = delete;
    TVulkanShader operator=(const TVulkanShader& other) = delete;

private:
    friend class VulkanShaderFactory;
};

/* Alias for shader variables, makes it easier to create these types of shaders, less verbose */
typedef TVulkanShader<EShaderType::Vertex> VulkanVertexShader;
typedef TVulkanShader<EShaderType::Fragment> VulkanFragmentShader;

/**
* Factory for creating all types of shaders, use this to create shaders
* (DEPRECATED) A ResourceManager is associated with a VulkanShaderFactory, 1:1 correlation -> new approach uses shader pools
*/
class VRIXIC_API VulkanShaderFactory
{
public:
    /**
    * @param inResourceManagerHandle - The resource manager that will be in use by this factory to create shader modules/handles
    */
    VulkanShaderFactory(VulkanDevice* inDevice);
    virtual ~VulkanShaderFactory();

    VulkanShaderFactory(const VulkanShaderFactory& other) = delete;
    VulkanShaderFactory operator=(const VulkanShaderFactory& other) = delete;

public:
    VulkanShader* CreateShader(VulkanShaderPool* inShaderPool, const FShaderConfig& inConfig) const;

    VulkanVertexShader* CreateVertexShader(VulkanShaderPool* inShaderPool, FShaderConfig& inConfig) const;
    VulkanFragmentShader* CreateFragmentShader(VulkanShaderPool* inShaderPool, FShaderConfig& inConfig) const;

private:

    /* ------------------------------------------------------------------------------- */
    /* -------------     Shader File Loading Helper Functions      ------------------- */
    /* ------------------------------------------------------------------------------- */

    void LoadShaderSourceFromFilePath(const FShaderConfig& inConfig, std::string& outSource) const;

    template<EShaderType T>
    void CreateShaderCompiledBinaryFile(const TVulkanShader<T>* inVulkanShader) const;

    /* ------------------------------------------------------------------------------- */
    /* -------------      Shader Compilation Helper Functions      ------------------- */
    /* ------------------------------------------------------------------------------- */

    void CompileSourceCode(const FShaderConfig& inConfig, uint8*& outCode, uint64* outCodeSize) const;

    EShLanguage ConvertShaderType(EShaderType inShaderType) const;

private:
    VulkanDevice* Device;

    static TBuiltInResource BuiltInResources;
};
