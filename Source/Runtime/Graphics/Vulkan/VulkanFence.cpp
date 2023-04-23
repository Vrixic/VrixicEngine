#include "VulkanFence.h"

VulkanFence::VulkanFence(VulkanDevice* inDevice)
    : Device(inDevice)
{
    VkFenceCreateInfo FenceCreateInfo = VulkanUtils::Initializers::FenceCreateInfo(0, nullptr);
    VK_CHECK_RESULT(vkCreateFence(*Device->GetDeviceHandle(), &FenceCreateInfo, nullptr, &FenceHandle), "[VulkanFence]: failed to create a fence object");
}

VulkanFence::~VulkanFence()
{
    if (FenceHandle != VK_NULL_HANDLE)
    {
        vkDestroyFence(*Device->GetDeviceHandle(), FenceHandle, nullptr);
    }
}

void VulkanFence::Wait(uint64 inTimeout)
{
    vkWaitForFences(*Device->GetDeviceHandle(), 1, &FenceHandle, VK_TRUE, inTimeout);
}

void VulkanFence::Reset()
{
    vkResetFences(*Device->GetDeviceHandle(), 1, &FenceHandle);
}
