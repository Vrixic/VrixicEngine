/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanPhysicalDevice.h"

VulkanPhysicalDevice::VulkanPhysicalDevice()
{
    PhysicalDeviceHandle = VK_NULL_HANDLE;
    PhysicalDeviceProperties = { 0 };
}

VulkanPhysicalDevice::~VulkanPhysicalDevice() { }

void VulkanPhysicalDevice::QueryDeviceProperties(RendererInfo& outRendererInfo)
{
    VE_ASSERT(PhysicalDeviceHandle != VK_NULL_HANDLE, "[VulkanPhysicalDevice]: Cannot querying device properties as there is no device selected.. Call PickBestPhysicalDevice() first!");

    outRendererInfo.DeviceName = PhysicalDeviceProperties.deviceName;

    // Select the vendor name from id 
    switch (PhysicalDeviceProperties.vendorID)
    {
    case 0x1002:
        outRendererInfo.DeviceVendorName = "Advanced Micro Devices, Inc.";
        break;
    case 0x10de:
        outRendererInfo.DeviceVendorName = "NVIDIA Corporation";
        break;
    case 0x102b:
        outRendererInfo.DeviceVendorName = "Matrox Electronic Systems Ltd.";
        break;
    case 0x1414:
        outRendererInfo.DeviceVendorName = "Microsoft Corporation";
        break;
    case 0x5333:
        outRendererInfo.DeviceVendorName = "S3 Graphics Co., Ltd.";
        break;
    case 0x8086:
        outRendererInfo.DeviceVendorName = "Intel Corporation";
        break;
    case 0x80ee:
        outRendererInfo.DeviceVendorName = "Oracle Corporation";
        break;
    case 0x15ad: 
        outRendererInfo.DeviceVendorName =  "VMware Inc.";
        break;
    }
    
    outRendererInfo.Name = "Vulkan 1.3";
}

bool VulkanPhysicalDevice::PickBestPhysicalDevice(VkInstance inVulkanInstance)
{
    VE_ASSERT(PhysicalDeviceHandle == VK_NULL_HANDLE, "[VulkanPhysicalDevice]: cannot pick another physical device as one that is valid already exists!");

    VkResult Result;

    // Physical Device
    uint32 PhysicalDevicesCount = 0;

    // Get Number of available physical devices
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(inVulkanInstance, &PhysicalDevicesCount, nullptr), "[EntryPoint]: No physical devices (GPU) found!");
    if (PhysicalDevicesCount == 0)
    {
        VE_CORE_LOG_ERROR("No device with Vulkan support found");
        return false;
    }

    // Enumerate physical devices
    std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDevicesCount);
    Result = (vkEnumeratePhysicalDevices(inVulkanInstance, &PhysicalDevicesCount, PhysicalDevices.data()));
    if (Result != VK_SUCCESS)
    {
        VE_CORE_LOG_ERROR("Could not enumerate physical devices");
        return false;
    }

    // GPU Selection
    VulkanUtils::Helpers::GetBestPhysicalDevice(PhysicalDevices.data(), PhysicalDevicesCount, PhysicalDeviceHandle);

    vkGetPhysicalDeviceProperties(PhysicalDeviceHandle, &PhysicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(PhysicalDeviceHandle, &PhysicalDeviceFeatures);

    return true;
}
