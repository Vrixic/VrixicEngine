/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanFrameBuffer.h"
#include <Misc/Defines/VulkanProfilerDefines.h>

VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice* device, VulkanRenderPass* renderPass)
	: Device(device), RenderPass(renderPass), FrameBufferHandle(VK_NULL_HANDLE) { }

VulkanFrameBuffer::~VulkanFrameBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	if (FrameBufferHandle != VK_NULL_HANDLE)
	{
		DestroyBuffer();
	}
}

void VulkanFrameBuffer::AllocateBuffer(uint32 numAttachments, const VkImageView* attachments, const VkExtent2D* extent)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VkFramebufferCreateInfo FrameBufferCreateInfo = VulkanUtils::Initializers::FrameBufferCreateInfo();
	FrameBufferCreateInfo.pNext = NULL;
	FrameBufferCreateInfo.renderPass = *RenderPass->GetRenderPassHandle();
	FrameBufferCreateInfo.attachmentCount = numAttachments;
	FrameBufferCreateInfo.pAttachments = attachments;
	FrameBufferCreateInfo.width = extent->width;
	FrameBufferCreateInfo.height = extent->height;
	FrameBufferCreateInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(*Device->GetDeviceHandle(), &FrameBufferCreateInfo, nullptr, &FrameBufferHandle), "[VulkanFrameBuffer]: Failed to create a frame buffer!");
}

void VulkanFrameBuffer::DestroyBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	Device->WaitUntilIdle();
	vkDestroyFramebuffer(*Device->GetDeviceHandle(), FrameBufferHandle, nullptr);
}

