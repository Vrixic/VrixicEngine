/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanTextureView.h"
#include <Misc/Defines/StringDefines.h>
#include <Misc/Defines/VulkanProfilerDefines.h>
#include "VulkanTypeConverter.h"

//VulkanTextureView::VulkanTextureView(VulkanDevice* device, VkImageCreateInfo& imageCreateInfo)
//	: Device(device), ImageHandle(VK_NULL_HANDLE), ViewHandle(VK_NULL_HANDLE)
//{
//	VE_PROFILE_VULKAN_FUNCTION();
//
//	VK_CHECK_RESULT(vkCreateImage(*device->GetDeviceHandle(), &imageCreateInfo, nullptr, &ImageHandle), "[VulkanTextureView]: Failed to create an image!");
//
//	VkMemoryRequirements MemoryRequirements{};
//	vkGetImageMemoryRequirements(*device->GetDeviceHandle(), ImageHandle, &MemoryRequirements);
//
//	uint32 MemoryTypeIndex = device->GetMemoryTypeIndex(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr);
//	VkMemoryAllocateInfo MemoryAlloc = VulkanUtils::Initializers::MemoryAllocateInfo();
//	MemoryAlloc.allocationSize = MemoryRequirements.size;
//	MemoryAlloc.memoryTypeIndex = MemoryTypeIndex;
//
//	VK_CHECK_RESULT(vkAllocateMemory(*device->GetDeviceHandle(), &MemoryAlloc, nullptr, &ImageMemory), "[VulkanTextureView]: Failed to allocate memory for an image!");
//	VK_CHECK_RESULT(vkBindImageMemory(*device->GetDeviceHandle(), ImageHandle, ImageMemory, 0), "[VulkanTextureView]: Failed to bind memory for an image!");
//}

VulkanTextureView::VulkanTextureView(VulkanDevice* inDevice, const FTextureConfig& inTextureConfig)
    : Texture(inTextureConfig.Type, inTextureConfig.BindFlags), Device(inDevice), ImageHandle(VK_NULL_HANDLE),
    ImageMemory(VK_NULL_HANDLE), ViewHandle(VK_NULL_HANDLE)
{
    ImageFormat = VulkanTypeConverter::Convert(inTextureConfig.Format);
    NumMipLevels = inTextureConfig.MipLevels;
    NumArrayLayers = inTextureConfig.NumArrayLayers;

    ImageLayout = VulkanTypeConverter::ConvertTextureLayoutToVk(inTextureConfig.Layout);

    // Create Image
    CreateImage(inTextureConfig);
}

VulkanTextureView::~VulkanTextureView()
{
    Device->WaitUntilIdle();

    if (ImageHandle != VK_NULL_HANDLE)
    {
        vkDestroyImage(*Device->GetDeviceHandle(), ImageHandle, nullptr);
    }

    if (ViewHandle != VK_NULL_HANDLE)
    {
        vkDestroyImageView(*Device->GetDeviceHandle(), ViewHandle, nullptr);
    }

    if (ImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(*Device->GetDeviceHandle(), ImageMemory, nullptr);
    }
}

void VulkanTextureView::CreateImageView(VkImageViewType
    viewType, VkFormat& format, uint32 baseMiplevel, uint32 levelCount, uint32 baseArrayLayer,
    uint32 layerCount)
{
    VE_ASSERT(ViewHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanTextureView]: Cannot replace a texture view..."));

    VkImageViewCreateInfo imageViewCI = VulkanUtils::Initializers::ImageViewCreateInfo();
    imageViewCI.viewType = viewType;
    imageViewCI.image = ImageHandle;
    imageViewCI.format = format;
    imageViewCI.subresourceRange.baseMipLevel = baseMiplevel;
    imageViewCI.subresourceRange.levelCount = levelCount;
    imageViewCI.subresourceRange.baseArrayLayer = baseArrayLayer;
    imageViewCI.subresourceRange.layerCount = layerCount;
    imageViewCI.subresourceRange.aspectMask = GetAspectFlags();

    VK_CHECK_RESULT(vkCreateImageView(*Device->GetDeviceHandle(), &imageViewCI, nullptr, &ViewHandle), "[VulkanTextureView]: Failed to create an image view!");
}

void VulkanTextureView::CreateImageView(const FTextureViewConfig& inTextureViewConfig)
{
    VE_ASSERT(ViewHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanTextureView]: Cannot replace a texture view..."));

    VkImageViewCreateInfo ImageViewCreateInfo = VulkanUtils::Initializers::ImageViewCreateInfo();
    ImageViewCreateInfo.viewType = VulkanTypeConverter::ConvertTextureViewTypeToVk(inTextureViewConfig.Type);
    ImageViewCreateInfo.image = ImageHandle;
    ImageViewCreateInfo.format = VulkanTypeConverter::Convert(inTextureViewConfig.Format);
    ImageViewCreateInfo.subresourceRange.baseMipLevel = inTextureViewConfig.Subresource.BaseMipLevel;
    ImageViewCreateInfo.subresourceRange.levelCount = inTextureViewConfig.Subresource.NumMipLevels;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = inTextureViewConfig.Subresource.BaseArrayLayer;
    ImageViewCreateInfo.subresourceRange.layerCount = inTextureViewConfig.Subresource.NumArrayLayers;
    ImageViewCreateInfo.subresourceRange.aspectMask = GetAspectFlags();

    VK_CHECK_RESULT(vkCreateImageView(*Device->GetDeviceHandle(), &ImageViewCreateInfo, nullptr, &ViewHandle), "[VulkanTextureView]: Failed to create an image view!");
}

void VulkanTextureView::CreateDefaultImageView()
{
    CreateImageView(
        VulkanTypeConverter::ConvertTextureViewTypeToVk(GetType()), ImageFormat, 0u, NumMipLevels, 0u, NumArrayLayers
    );
}

void VulkanTextureView::CreateImage(const FTextureConfig& inTextureConfig)
{
    VkImageCreateInfo ImageCreateInfo = VulkanUtils::Initializers::ImageCreateInfo();
    ImageCreateInfo.imageType = VulkanTypeConverter::ConvertTextureTypeToVk(inTextureConfig.Type);
    ImageCreateInfo.format = VulkanTypeConverter::Convert(inTextureConfig.Format);
    ImageCreateInfo.extent = { inTextureConfig.Extent.Width, inTextureConfig.Extent.Height, inTextureConfig.Extent.Depth };
    ImageCreateInfo.mipLevels = NumMipLevels;
    ImageCreateInfo.arrayLayers = NumArrayLayers;
    ImageCreateInfo.samples = VulkanTypeConverter::ConvertSampleCountToVk(inTextureConfig.NumSamples);
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.usage = VulkanTypeConverter::ConvertTextureUsageFlagsToVk(inTextureConfig.BindFlags);

    VK_CHECK_RESULT(vkCreateImage(*Device->GetDeviceHandle(), &ImageCreateInfo, nullptr, &ImageHandle), "[VulkanTextureView]: Failed to create an image!");

    VkMemoryRequirements MemoryRequirements = { };
    vkGetImageMemoryRequirements(*Device->GetDeviceHandle(), ImageHandle, &MemoryRequirements);

    uint32 MemoryTypeIndex = Device->GetMemoryTypeIndex(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr);
    VkMemoryAllocateInfo MemoryAlloc = VulkanUtils::Initializers::MemoryAllocateInfo();
    MemoryAlloc.allocationSize = MemoryRequirements.size;
    MemoryAlloc.memoryTypeIndex = MemoryTypeIndex;

    VK_CHECK_RESULT(vkAllocateMemory(*Device->GetDeviceHandle(), &MemoryAlloc, nullptr, &ImageMemory), "[VulkanTextureView]: Failed to allocate memory for an image!");
    VK_CHECK_RESULT(vkBindImageMemory(*Device->GetDeviceHandle(), ImageHandle, ImageMemory, 0), "[VulkanTextureView]: Failed to bind memory for an image!");
}

VkImageAspectFlags VulkanTextureView::GetAspectFlags() const
{
    switch (ImageFormat)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;

    case VK_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;

    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}
