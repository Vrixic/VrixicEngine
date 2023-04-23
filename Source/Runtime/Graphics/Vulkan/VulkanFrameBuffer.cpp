/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanFrameBuffer.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Misc/Defines/StringDefines.h>
#include "VulkanTextureView.h"

VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice* device)
	: Device(device), RenderPass(VK_NULL_HANDLE), FrameBufferHandle(VK_NULL_HANDLE),
        Extent(), NumAttachments(0) { }

VulkanFrameBuffer::~VulkanFrameBuffer()
{
	if (FrameBufferHandle != VK_NULL_HANDLE)
	{
		DestroyBuffer();
	}
}

void VulkanFrameBuffer::Create(uint32 numAttachments, const VkImageView* attachments, const VkExtent2D* extent, VulkanRenderPass* inRenderPass)
{
    VE_ASSERT(FrameBufferHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanFrameBuffer]: Cannot create another framebuffer when this one already exists!!"));

    NumAttachments = numAttachments;
    Extent.Width = extent->width;
    Extent.Height = extent->height;
    RenderPass = inRenderPass;

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

void VulkanFrameBuffer::Create(const FrameBufferConfig& inFrameBufferConfig)
{
    VE_ASSERT(FrameBufferHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanFrameBuffer]: Cannot create another framebuffer when this one already exists!!"));

    NumAttachments = inFrameBufferConfig.Attachments.size();
    Extent.Width = inFrameBufferConfig.Resolution.Width;
    Extent.Height = inFrameBufferConfig.Resolution.Height;
    RenderPass = (VulkanRenderPass*)inFrameBufferConfig.RenderPass;

    // Get the attachments 
    std::vector<VkImageView> Attachments(NumAttachments);
    for (uint32 i = 0; i < NumAttachments; i++)
    {
        VulkanTextureView* TextureView = (VulkanTextureView*)inFrameBufferConfig.Attachments[i].Attachment;
        Attachments[i] = *TextureView->GetImageViewHandle();
    }

    VkFramebufferCreateInfo FrameBufferCreateInfo = VulkanUtils::Initializers::FrameBufferCreateInfo();
    FrameBufferCreateInfo.pNext = NULL;
    FrameBufferCreateInfo.renderPass = *RenderPass->GetRenderPassHandle();
    FrameBufferCreateInfo.attachmentCount = NumAttachments;
    FrameBufferCreateInfo.pAttachments = Attachments.data();
    FrameBufferCreateInfo.width = Extent.Width;
    FrameBufferCreateInfo.height = Extent.Height;
    FrameBufferCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(*Device->GetDeviceHandle(), &FrameBufferCreateInfo, nullptr, &FrameBufferHandle), "[VulkanFrameBuffer]: Failed to create a frame buffer!");
}

void VulkanFrameBuffer::DestroyBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	Device->WaitUntilIdle();
	vkDestroyFramebuffer(*Device->GetDeviceHandle(), FrameBufferHandle, nullptr);
}

Extent2D VulkanFrameBuffer::GetResolution() const
{
	return Extent;
}

uint32 VulkanFrameBuffer::GetNumAttachments() const
{
	return NumAttachments;
}

IRenderPass* VulkanFrameBuffer::GetRenderPassHandle() const
{
    return (IRenderPass*)RenderPass;
}

