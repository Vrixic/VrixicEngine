/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanTextureView.h"
#include <Misc/Defines/VulkanProfilerDefines.h>

VulkanTextureView::VulkanTextureView(VulkanDevice* device, VkImageCreateInfo& imageCreateInfo)
	: Device(device), ImageHandle(VK_NULL_HANDLE), ViewHandle(VK_NULL_HANDLE)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VK_CHECK_RESULT(vkCreateImage(*device->GetDeviceHandle(), &imageCreateInfo, nullptr, &ImageHandle), "[VulkanTextureView]: Failed to create an image!");

	VkMemoryRequirements MemoryRequirements{};
	vkGetImageMemoryRequirements(*device->GetDeviceHandle(), ImageHandle, &MemoryRequirements);

	uint32 MemoryTypeIndex = device->GetMemoryTypeIndex(MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr);
	VkMemoryAllocateInfo MemoryAlloc = VulkanUtils::Initializers::MemoryAllocateInfo();
	MemoryAlloc.allocationSize = MemoryRequirements.size;
	MemoryAlloc.memoryTypeIndex = MemoryTypeIndex;

	VK_CHECK_RESULT(vkAllocateMemory(*device->GetDeviceHandle(), &MemoryAlloc, nullptr, &ImageMemory), "[VulkanTextureView]: Failed to allocate memory for an image!");
	VK_CHECK_RESULT(vkBindImageMemory(*device->GetDeviceHandle(), ImageHandle, ImageMemory, 0), "[VulkanTextureView]: Failed to bind memory for an image!");
}

VulkanTextureView::~VulkanTextureView()
{
	VE_PROFILE_VULKAN_FUNCTION();

	vkDestroyImage(*Device->GetDeviceHandle(), ImageHandle, nullptr);
	vkDestroyImageView(*Device->GetDeviceHandle(), ViewHandle, nullptr);
	vkFreeMemory(*Device->GetDeviceHandle(), ImageMemory, nullptr);
}


void VulkanTextureView::CreateImageView(VkImageViewType
	viewType, VkFormat& format, uint32 baseMiplevel, uint32 levelCount, uint32 baseArrayLayer,
	uint32 layerCount, VkImageAspectFlags& aspectFlags)
{
	VE_PROFILE_VULKAN_FUNCTION();

	if (ViewHandle != VK_NULL_HANDLE)
	{
		VE_CORE_LOG_WARN("Cannot replace a texture view...");
		return;
	}

	VkImageViewCreateInfo imageViewCI = VulkanUtils::Initializers::ImageViewCreateInfo();
	imageViewCI.viewType = viewType;
	imageViewCI.image = ImageHandle;
	imageViewCI.format = format;
	imageViewCI.subresourceRange.baseMipLevel = baseMiplevel;
	imageViewCI.subresourceRange.levelCount = levelCount;
	imageViewCI.subresourceRange.baseArrayLayer = baseArrayLayer;
	imageViewCI.subresourceRange.layerCount = layerCount;
	imageViewCI.subresourceRange.aspectMask = aspectFlags;

	VK_CHECK_RESULT(vkCreateImageView(*Device->GetDeviceHandle(), &imageViewCI, nullptr, &ViewHandle), "[VulkanTextureView]: Failed to create an image view!");
}
