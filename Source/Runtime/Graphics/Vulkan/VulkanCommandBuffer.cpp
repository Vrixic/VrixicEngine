#include "VulkanCommandBuffer.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include "VulkanUtils.h"

/* ------------------------------------------------------------------------------- */
/* -----------------------         Command Buffer         ------------------------ */
/* ------------------------------------------------------------------------------- */

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VulkanCommandPool* commandPool, uint32 imageIndex)
	: Device(device), CommandPool(commandPool), ImageIndex(imageIndex), CommandBufferHandle(VK_NULL_HANDLE)
{
	VE_PROFILE_VULKAN_FUNCTION();

	// Fence (Used to check draw command buffer completion)
	// Create in signaled state so we don't wait on first render of each command buffer
	VkFenceCreateInfo FenceCreateInfo = VulkanUtils::Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT, nullptr);
	VK_CHECK_RESULT(vkCreateFence(*Device->GetDeviceHandle(), &FenceCreateInfo, nullptr, &WaitFence), "[VulkanCommandBuffer]: Failed to create a fence that is used to check draw command buffer completion!");
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	//for (uint32 i = 0; i < WaitSemaphores.size(); ++i)
	//{
	//	vkDestroySemaphore(Device->GetDeviceHandle(), WaitSemaphores[i], nullptr);
	//}

	vkDestroyFence(*Device->GetDeviceHandle(), WaitFence, nullptr);
}

void VulkanCommandBuffer::AllocateCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = VulkanUtils::Initializers::CommandBufferAllocateInfo();
	CommandBufferAllocateInfo.commandPool = CommandPool->GetCommandPoolHandle();
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(*Device->GetDeviceHandle(), &CommandBufferAllocateInfo, &CommandBufferHandle), "[VulkanCommandBuffer]: Failed to create a command buffer!");
}

void VulkanCommandBuffer::FreeCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	vkFreeCommandBuffers(*Device->GetDeviceHandle(), CommandPool->GetCommandPoolHandle(), 1, &CommandBufferHandle);
}

void VulkanCommandBuffer::BeginCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	VkCommandBufferBeginInfo CommandBufferBeginInfo = VulkanUtils::Initializers::CommandBufferBeginInfo(nullptr);
	VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBufferHandle, &CommandBufferBeginInfo), "[VulkanCommandBuffer]: Failed to begin a command buffer!");
}

void VulkanCommandBuffer::EndCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	VK_CHECK_RESULT(vkEndCommandBuffer(CommandBufferHandle), "[VulkanCommandBuffer]: Failed to end a command buffer!");
}

void VulkanCommandBuffer::BeginRenderPass(const VulkanRenderPass* renderPass, VulkanFrameBuffer* frameBuffer)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VkRenderPassBeginInfo RenderPassBeginInfo = VulkanUtils::Initializers::RenderPassBeginInfo(renderPass->GetRenderPassHandle(), nullptr);
	const VkRect2D* RenderArea = renderPass->GetRenderLayout()->GetRenderArea();
	RenderPassBeginInfo.renderArea.offset.x = RenderArea->offset.x;
	RenderPassBeginInfo.renderArea.offset.y = RenderArea->offset.y;
	RenderPassBeginInfo.renderArea.extent.width = RenderArea->extent.width;
	RenderPassBeginInfo.renderArea.extent.height = RenderArea->extent.height;

	RenderPassBeginInfo.clearValueCount = renderPass->GetRenderLayout()->GetNumClearValues();
	RenderPassBeginInfo.pClearValues = renderPass->GetRenderLayout()->GetClearValues();

	RenderPassBeginInfo.framebuffer = frameBuffer->GetFrameBufferHandle();

	vkCmdBeginRenderPass(CommandBufferHandle, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::EndRenderPass()
{
	VE_PROFILE_VULKAN_FUNCTION();

	vkCmdEndRenderPass(CommandBufferHandle);
}

void VulkanCommandBuffer::AddWaitSemaphore(VkSemaphore* semaphore)
{
	VE_PROFILE_VULKAN_FUNCTION();

	WaitSemaphores.push_back(*semaphore);
}

void VulkanCommandBuffer::SetWaitFence() const
{
	VE_PROFILE_VULKAN_FUNCTION();

	VK_CHECK_RESULT(vkWaitForFences(*Device->GetDeviceHandle(), 1, &WaitFence, VK_TRUE, UINT64_MAX), "[VulkanCommandBuffer]: Failed to set a fence to wait!");
}

void VulkanCommandBuffer::ResetWaitFence() const
{
	VE_PROFILE_VULKAN_FUNCTION();

	VK_CHECK_RESULT(vkResetFences(*Device->GetDeviceHandle(), 1, &WaitFence), "[VulkanCommandBuffer]: Failed to reset a fence!");
}

/* ------------------------------------------------------------------------------- */
/* -----------------------          Command Pool          ------------------------ */
/* ------------------------------------------------------------------------------- */

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device)
	: Device(device), CommandPoolHandle(VK_NULL_HANDLE) { }

VulkanCommandPool::~VulkanCommandPool()
{
	VE_PROFILE_VULKAN_FUNCTION();

	Device->WaitUntilIdle();

	DestroyBuffers();
	vkDestroyCommandPool(*Device->GetDeviceHandle(), CommandPoolHandle, nullptr);
}

VulkanCommandBuffer* VulkanCommandPool::CreateCommandBuffer(uint32 imageIndex)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VulkanCommandBuffer* CommandBuffer = new VulkanCommandBuffer(Device, this, imageIndex);
	CommandBuffers.push_back(CommandBuffer);

	return CommandBuffer;
}

void VulkanCommandPool::CreateCommandPool(uint32 queueFamilyIndex)
{
	VE_PROFILE_VULKAN_FUNCTION();

	VkCommandPoolCreateInfo CommandPoolInfo = VulkanUtils::Initializers::CommandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		queueFamilyIndex, nullptr);

	VK_CHECK_RESULT(vkCreateCommandPool(*Device->GetDeviceHandle(), &CommandPoolInfo, nullptr, &CommandPoolHandle), "[VulkanCommandPool]: Failed to create a command pool!");
}

void VulkanCommandPool::DestroyBuffers()
{
	VE_PROFILE_VULKAN_FUNCTION();

	for (uint32 i = 0; i < CommandBuffers.size(); ++i)
	{
		CommandBuffers[i]->FreeCommandBuffer();
		delete CommandBuffers[i];
	}

	CommandBuffers.clear();
}

