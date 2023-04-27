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
enum class ERenderInterfaceType
{
    Direct3D12,
    Vulkan
};

/**
* These are features that are available on a physical device (GPU) -> Ex. vulkan -> VkPhysicalDeviceFeatures
*/
struct VRIXIC_API FPhysicalDeviceFeatures
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
struct VRIXIC_API FRendererInfo
{
public:
    /** Name of the renderer interface, ex: vulkan */
    std::string Name;

    /** device vendor name, ex: NVIDIA */
    std::string DeviceVendorName;

    /** device name it self, the GPU, ex: Geforce RTX.... */
    std::string DeviceName;

public:
    FRendererInfo() { }
};

/**
* Details about the application instance, for instance: VkInstance needs to know about the version, name, etc..
*/
struct VRIXIC_API FApplicationInstanceInfo
{
public:
    /** The name of the application, ex: Sandbox Project, for game it could be game name */
    std::string ApplicationName;

    /** The version of the application */
    uint32 ApplicationVersion;

    /** Engine name, for this it'll be "Vrixic Engine" -> Do not set */
    std::string EngineName;
     
    /** The engine version -> Do not set as it is not used as of right now (HARD SET) */
    uint32 EngineVersion;

public:
    FApplicationInstanceInfo()
        : ApplicationVersion(UINT32_MAX), EngineVersion(UINT32_MAX) { }
};

/**
* Consists of things you can set for vulkan renderer creation
*/
struct VRIXIC_API FVulkanRendererConfig
{
public:
    /** The application instance info used to create instance */
    FApplicationInstanceInfo AppInstanceInfo;

    /** The layers to enable when creating a new vulkan instance */
    std::vector<std::string> EnabledInstanceLayers;

    /** The extensions to enable when creating a new vulkan instance */
    std::vector<std::string> EnabledInstanceExtensions;

    /** 
    * All of the enabled extensions on the device in use by the renderer 
    * ex: for vulkan you can have VK_EXT_multiviewport, etc...
    */
    const char** EnabledDeviceExtensions;

    /** count of all the enabled device extensions */
    uint32 EnabledDeviceExtensionCount;

    /** contains all of the enabled device features for example: multiViewporting */
    FPhysicalDeviceFeatures EnabledDeviceFeatures;

public:
    FVulkanRendererConfig()
        : EnabledDeviceExtensions(nullptr), EnabledDeviceExtensionCount(0) { }
};
