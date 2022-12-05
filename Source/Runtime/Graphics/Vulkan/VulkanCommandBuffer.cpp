#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"

/* ------------------------------------------------------------------------------- */
/* -----------------------         Command Buffer         ------------------------ */
/* ------------------------------------------------------------------------------- */

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VulkanCommandPool* commandPool, uint32 imageIndex)
	: Device(device), CommandPool(commandPool), ImageIndex(imageIndex), CommandBufferHandle(VK_NULL_HANDLE)
{
	// Fence (Used to check draw command buffer completion)
	// Create in signaled state so we don't wait on first render of each command buffer
	VkFenceCreateInfo FenceCreateInfo = VulkanUtils::Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT, nullptr);
	VK_CHECK_RESULT(vkCreateFence(*Device->GetDeviceHandle(), &FenceCreateInfo, nullptr, &WaitFence));
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	//for (uint32 i = 0; i < WaitSemaphores.size(); ++i)
	//{
	//	vkDestroySemaphore(Device->GetDeviceHandle(), WaitSemaphores[i], nullptr);
	//}

	vkDestroyFence(*Device->GetDeviceHandle(), WaitFence, nullptr);
}

void VulkanCommandBuffer::AllocateCommandBuffer()
{
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = VulkanUtils::Initializers::CommandBufferAllocateInfo();
	CommandBufferAllocateInfo.commandPool = CommandPool->GetCommandPoolHandle();
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(*Device->GetDeviceHandle(), &CommandBufferAllocateInfo, &CommandBufferHandle));
}

void VulkanCommandBuffer::FreeCommandBuffer()
{
	vkFreeCommandBuffers(*Device->GetDeviceHandle(), CommandPool->GetCommandPoolHandle(), 1, &CommandBufferHandle);
}

void VulkanCommandBuffer::BeginCommandBuffer()
{
	VkCommandBufferBeginInfo CommandBufferBeginInfo = VulkanUtils::Initializers::CommandBufferBeginInfo(nullptr);
	VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBufferHandle, &CommandBufferBeginInfo));
}

void VulkanCommandBuffer::EndCommandBuffer()
{
	VK_CHECK_RESULT(vkEndCommandBuffer(CommandBufferHandle));
}

void VulkanCommandBuffer::BeginRenderPass(const VulkanRenderPass* renderPass, VulkanFrameBuffer* frameBuffer)
{
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
	vkCmdEndRenderPass(CommandBufferHandle);
}

void VulkanCommandBuffer::AddWaitSemaphore(VkSemaphore* semaphore)
{
	WaitSemaphores.push_back(*semaphore);
}

void VulkanCommandBuffer::SetWaitFence() const
{
	VK_CHECK_RESULT(vkWaitForFences(*Device->GetDeviceHandle(), 1, &WaitFence, VK_TRUE, UINT64_MAX));
}

void VulkanCommandBuffer::ResetWaitFence() const
{
	VK_CHECK_RESULT(vkResetFences(*Device->GetDeviceHandle(), 1, &WaitFence));
}

/* ------------------------------------------------------------------------------- */
/* -----------------------          Command Pool          ------------------------ */
/* ------------------------------------------------------------------------------- */

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device)
	: Device(device), CommandPoolHandle(VK_NULL_HANDLE) { }

VulkanCommandPool::~VulkanCommandPool()
{
	Device->WaitUntilIdle();

	DestroyBuffers();
	vkDestroyCommandPool(*Device->GetDeviceHandle(), CommandPoolHandle, nullptr);
}

VulkanCommandBuffer* VulkanCommandPool::CreateCommandBuffer(uint32 imageIndex)
{
	VulkanCommandBuffer* CommandBuffer = new VulkanCommandBuffer(Device, this, imageIndex);
	CommandBuffers.push_back(CommandBuffer);

	return CommandBuffer;
}

void VulkanCommandPool::CreateCommandPool(uint32 queueFamilyIndex)
{
	VkCommandPoolCreateInfo CommandPoolInfo = VulkanUtils::Initializers::CommandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		queueFamilyIndex, nullptr);

	VK_CHECK_RESULT(vkCreateCommandPool(*Device->GetDeviceHandle(), &CommandPoolInfo, nullptr, &CommandPoolHandle));
}

void VulkanCommandPool::DestroyBuffers()
{
	for (uint32 i = 0; i < CommandBuffers.size(); ++i)
	{
		CommandBuffers[i]->FreeCommandBuffer();
		delete CommandBuffers[i];
	}

	CommandBuffers.clear();
}

