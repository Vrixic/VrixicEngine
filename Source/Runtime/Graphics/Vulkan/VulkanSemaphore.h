/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Runtime/Graphics/Semaphore.h>
#include <Runtime/Graphics/Vulkan/VulkanDevice.h>


/**
* A vulkan specific semaphore used for GPU synchronization (waiting for rendering/presentation, etc...)
* 
* Can hold more than one semaphore
*/
class VulkanSemaphore : public ISemaphore
{
public:
    /**
    * @remarks does not create a sempahore by default (manual creation required)
    */
    VulkanSemaphore(VulkanDevice* inDevice);
    ~VulkanSemaphore();

    /**
    * Creates the semaphore 
    */
    void Create(const FSemaphoreConfig& inConfig);

    /**
    * Destroys the semaphore
    */
    void Destroy();

public:
    VkSemaphore* GetSemaphoresHandle()
    {
        return SemaphoreHandles;
    }

    uint32 GetSemaphoresCount() const
    {
        return SemaphoreCount;
    }

private:
    VkSemaphore* SemaphoreHandles;
    VulkanDevice* Device;

    uint32 SemaphoreCount;
};

