#include "VulkanFrameBuffer.h"

VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice* device, VulkanRenderPass* renderPass)
	: Device(device), RenderPass(renderPass), FrameBufferHandle(VK_NULL_HANDLE) { }

VulkanFrameBuffer::~VulkanFrameBuffer()
{
	if (FrameBufferHandle != VK_NULL_HANDLE)
	{
		DestroyBuffer();
	}
}

void VulkanFrameBuffer::AllocateBuffer(uint32 numAttachments, const VkImageView* attachments, const VkExtent2D* extent)
{
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
	Device->WaitUntilIdle();
	vkDestroyFramebuffer(*Device->GetDeviceHandle(), FrameBufferHandle, nullptr);
}

