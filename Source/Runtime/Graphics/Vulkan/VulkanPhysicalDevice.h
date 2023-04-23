/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Graphics/RenderInterfaceGenerics.h>
#include "VulkanDevice.h"

/**
* Representation of VkPhysicalDevice 
*/
class VulkanPhysicalDevice
{
private:
    VkPhysicalDevice PhysicalDeviceHandle;

    // The physical device properties used for querying 
    VkPhysicalDeviceProperties PhysicalDeviceProperties;

    // all of the supported features on the physical device 
    VkPhysicalDeviceFeatures PhysicalDeviceFeatures;

public:
    VulkanPhysicalDevice(); 
    ~VulkanPhysicalDevice();

    /**
    * Queries information from the physical device using its properties and features 
    * 
    * @param outRendererInfo gets filled up with information about the physical device 
    */
    void QueryDeviceProperties(RendererInfo& outRendererInfo);

public:
    /**
    * Picks the best physical device (GPU) available on the computer
    * 
    * @param inVulkanInstance the instance that will be used to get the physical device
    * @returns bool true if it successfully found a physical device, false otherwise
    * @remarks should only be called once 
    */
    bool PickBestPhysicalDevice(VkInstance inVulkanInstance);

    /**
    * @returns VkPhysicalDevice the physical device handle 
    */
    inline VkPhysicalDevice GetPhysicalDeviceHandle() const
    {
        return PhysicalDeviceHandle;
    }

    /**
    * @returns const VkPhysicalDeviceProperties& the properties of the physical device
    */
    inline const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const
    {
        return PhysicalDeviceProperties;
    }

    /**
    * @returns const VkPhysicalDeviceFeatures& all features supported by the physical device 
    */
    inline const VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures() const
    {
        return PhysicalDeviceFeatures;
    }
};
