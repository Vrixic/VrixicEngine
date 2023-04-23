/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Graphics/Shader.h>
#include "Runtime/Memory/ResourceManager.h"
#include "VulkanDevice.h"

struct VertexInputDescription;

/**
* A shader pool
* Contains shader modules and is the only way to allocate/create shader modules specific to vulkan (VkShaderModule)
* Used by VulkanShaderFactory for creating shaders
*/
class VRIXIC_API VulkanShaderPool
{
    friend class VulkanShaderFactory;
private:
    VulkanDevice* Device;

    // All shader module handles
    std::vector<VkShaderModule> ShaderModuleHandles;

public:
    VulkanShaderPool(VulkanDevice* inDevice);

    ~VulkanShaderPool();

private:
    VkShaderModule CreateShaderModule(const uint32* inCompiledShaderCode, uint32 inCodeSize);

public:
    VkShaderModule GetShaderModule(uint32 inShaderKey) const
    {
        return ShaderModuleHandles[inShaderKey];
    }
};

/**
* Representation of a Shader in Vulkan
* Do not manually create these.. Use VulkanShaderFactory!
*/
class VRIXIC_API VulkanShader : public Shader
{
protected:
    VulkanDevice* Device;

    /* A Key to the shader module location into the array of shader modules inside of the pool that was used to create this shader  */
    uint32 ShaderKey;

    /** The shader pool that is used to create this shader */
    VulkanShaderPool* ShaderPool;

    std::vector<VkVertexInputAttributeDescription> InputAttributes;
    std::vector<VkVertexInputBindingDescription> InputBindings;

public:
    /**
    * @param EShaderType - The type of shader ex. Vertex, Fragment, etc...
    */
    VulkanShader(VulkanDevice* inDevice, EShaderType inShaderType);

    virtual ~VulkanShader();

    VulkanShader(const VulkanShader& other) = delete;
    VulkanShader operator=(const VulkanShader& other) = delete;

    void BuildInputLayout(uint32 inNumVertexDescriptions, const VertexInputDescription* inVertexDescriptions);

    void CreateVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& inInputStateCreateInfo) const;

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
    friend class VulkanShaderFactory;

private:
    TVulkanShader(VulkanDevice* inDevice);

    TVulkanShader(const TVulkanShader& other) = delete;
    TVulkanShader operator=(const TVulkanShader& other) = delete;
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
private:
    VulkanDevice* Device;

    /* Handle to the resource manager that will be used to create all shaders within this factory */
    //ResourceManager* ResourceManagerHandle;

public:
    /**
    * @param inResourceManagerHandle - The resource manager that will be in use by this factory to create shader modules/handles
    */
    VulkanShaderFactory(VulkanDevice* inDevice/*, ResourceManager* inResourceManagerHandle*/);
    virtual ~VulkanShaderFactory();

    VulkanShaderFactory(const VulkanShaderFactory& other) = delete;
    VulkanShaderFactory operator=(const VulkanShaderFactory& other) = delete;

public:
    VulkanShader* CreateShader(VulkanShaderPool* inShaderPool, const ShaderConfig& inConfig) const;

    VulkanVertexShader* CreateVertexShader(VulkanShaderPool* inShaderPool, const ShaderConfig& inConfig) const;
    VulkanFragmentShader* CreateFragmentShader(VulkanShaderPool* inShaderPool, const ShaderConfig& inConfig) const;

private:
    void CompileSourceCode(const ShaderConfig& inConfig, const uint32*& outCode, uint32* outCodeSize) const;
};
