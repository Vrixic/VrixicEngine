/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

#include <string>
#include <vector>

/**
* @TODO: create a better way to integrate and configure instance and device extensions, as well as device features 
*/

/**
* Graphics API that could be support by the engine
*/
enum class ERenderInterface
{
    Direct3D12,
    Vulkan
};

/**
* These are features that are available on a physical device (GPU) -> Ex. vulkan -> VkPhysicalDeviceFeatures
*/
struct VRIXIC_API PhysicalDeviceFeatures
{
    bool TessellationShader             = false;
    bool GeometryShader                 = false;
    bool FillModeNonSolid               = false;
    bool SamplerAnisotropy              = false;
    bool MultiViewports                 = false;
};

/**
* Information about the renderer
*/
struct VRIXIC_API RendererInfo
{
    // Name of the renderer interface, ex: vulkan
    std::string Name;

    // device vendor name, ex: NVIDIA
    std::string DeviceVendorName;

    // device name it self, the GPU, ex: Geforce RTX....
    std::string DeviceName;
};

/**
* Details about the application instance, for instance: VkInstance needs to know about the version, name, etc..
*/
struct VRIXIC_API ApplicationInstanceInfo
{
public:
    // The name of the application, ex: Sandbox Project, for game it could be game name
    std::string ApplicationName;

    // The version of the application
    uint32 ApplicationVersion;

    // Engine name, for this it'll be "Vrixic Engine" -> Do not set
    std::string EngineName;

    // The engine version -> Do not set as it is not used as of right now (HARD SET)
    uint32 EngineVersion;
};

/**
* Consists of things you can set for vulkan renderer creation
*/
struct VRIXIC_API VulkanRendererConfig
{
public:
    // The application instance info used to create instance 
    ApplicationInstanceInfo AppInstanceInfo;

    // The layers to enable when creating a new vulkan instance 
    std::vector<std::string> EnabledInstanceLayers;

    // The extensions to enable when creating a new vulkan instance 
    std::vector<std::string> EnabledInstanceExtensions;

    // All of the enabled extensions on the device in use by the renderer 
    // ex: for vulkan you can have VK_EXT_multiviewport, etc...
    const char** EnabledDeviceExtensions;

    // count of all the enabled device extensions
    uint32 EnabledDeviceExtensionCount;

    // contains all of the enabled device features for example: multiViewporting
    PhysicalDeviceFeatures EnabledDeviceFeatures;
};
