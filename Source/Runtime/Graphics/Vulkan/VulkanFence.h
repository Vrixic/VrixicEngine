/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Graphics/Fence.h>
#include <Runtime/Graphics/Vulkan/VulkanDevice.h>

/**
* Vulkan definition for the fence interface 
*/
class VulkanFence : public IFence
{
private:
    VulkanDevice* Device;
    VkFence FenceHandle;

public:
    /**
    * Creates the fence 
    */
    VulkanFence(VulkanDevice* inDevice);
    ~VulkanFence();

    /**
    * Waits on the fence until a timeout on the device the fence was used to create with 
    * 
    * @param uint64 the time in nanoseconds to wait
    */
    void Wait(uint64 inTimeout);

    /**
    * Resets the fence
    */
    void Reset();

public:
    /**
    * @returns VkFence the handle to the fence 
    */
    inline VkFence GetFenceHandle() const
    {
        return FenceHandle;
    }
};