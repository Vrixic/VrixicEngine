/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanSemaphore.h"
#include <Misc/Defines/StringDefines.h>

VulkanSemaphore::VulkanSemaphore(VulkanDevice* inDevice)
    : Device(inDevice), SemaphoreHandles(nullptr), SemaphoreCount(0) { }

VulkanSemaphore::~VulkanSemaphore()
{
    Destroy();
}

void VulkanSemaphore::Create(const SemaphoreConfig& inConfig)
{
    VE_ASSERT(SemaphoreCount == 0, VE_TEXT("[VulkanSemaphore]: Memory Leak!! Cannot create another sempahore while a valid handle exists!!"));

    SemaphoreCount = inConfig.NumSemaphores;
    SemaphoreHandles = new VkSemaphore[SemaphoreCount];

    VkSemaphoreCreateInfo SemaphoreCreateInfo = VulkanUtils::Initializers::SemaphoreCreateInfo(nullptr);
    for (uint32 i = 0; i < SemaphoreCount; i++)
    {
        VK_CHECK_RESULT(vkCreateSemaphore(*Device->GetDeviceHandle(), &SemaphoreCreateInfo, nullptr, &SemaphoreHandles[i]), VE_TEXT("[VulkanSemaphore]: Failed to create a semaphore!!"));
    }
}

void VulkanSemaphore::Destroy()
{
    VE_ASSERT(SemaphoreHandles != 0, VE_TEXT("[VulkanSemaphore]: Cannot destory a semaphore that is already a NULL Handle!!"));

    Device->WaitUntilIdle();
    for (uint32 i = 0; i < SemaphoreCount; i++)
    {
        vkDestroySemaphore(*Device->GetDeviceHandle(), SemaphoreHandles[i], nullptr);
    }

    SemaphoreHandles = nullptr;
    SemaphoreCount = 0;
}
