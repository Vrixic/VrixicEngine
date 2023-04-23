/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanCommandBuffer.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Runtime/Graphics/Vulkan/VulkanTypeConverter.h>
#include "VulkanBuffer.h"
#include "VulkanUtils.h"
#include "VulkanPipeline.h"

/* ------------------------------------------------------------------------------- */
/* -----------------------         Command Buffer         ------------------------ */
/* ------------------------------------------------------------------------------- */
VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VulkanCommandPool* commandPool, uint32 imageIndex)
	: Device(device), CommandPool(commandPool), ImageIndex(imageIndex), CommandBufferHandle(VK_NULL_HANDLE), WaitFence(VK_NULL_HANDLE)
{
	VE_PROFILE_VULKAN_FUNCTION();

    CreateWaitFence();
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

void VulkanCommandBuffer::Begin()
{
    VE_PROFILE_VULKAN_FUNCTION();

    BeginCommandBuffer();
}

void VulkanCommandBuffer::End()
{
    VE_PROFILE_VULKAN_FUNCTION();

    EndCommandBuffer();
}

void VulkanCommandBuffer::SetRenderViewports(const RenderViewport* inRenderViewports, uint32 inNumRenderViewports)
{
    VE_PROFILE_VULKAN_FUNCTION();

    vkCmdSetViewport(CommandBufferHandle, 0, inNumRenderViewports, (VkViewport*)(inRenderViewports));
}

void VulkanCommandBuffer::SetRenderScissors(const RenderScissor* inRenderScissors, uint32 inNumRenderScissors)
{
    VE_PROFILE_VULKAN_FUNCTION();

    vkCmdSetScissor(CommandBufferHandle, 0, inNumRenderScissors, (VkRect2D*)inRenderScissors);
}

void VulkanCommandBuffer::SetVertexBuffer(Buffer& inVertexBuffer)
{
    VE_PROFILE_VULKAN_FUNCTION();

    VulkanBuffer& Buf = (VulkanBuffer&)inVertexBuffer;
    VkDeviceSize Offsets[] = { 0 };

    vkCmdBindVertexBuffers(CommandBufferHandle, 0, 1, Buf.GetBufferHandle(), Offsets);
}

void VulkanCommandBuffer::SetIndexBuffer(Buffer& inIndexBuffer)
{
    VE_PROFILE_VULKAN_FUNCTION();

    VulkanBuffer& Buf = (VulkanBuffer&)inIndexBuffer;

    vkCmdBindIndexBuffer(CommandBufferHandle, *Buf.GetBufferHandle(), 0, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::BeginRenderPass(const RenderPassBeginInfo& inRenderPassBeginInfo)
{
    VE_PROFILE_VULKAN_FUNCTION();

    VulkanRenderPass* RenderPass = (VulkanRenderPass*)inRenderPassBeginInfo.RenderPassPtr;
    VulkanFrameBuffer* FrameBuffer = (VulkanFrameBuffer*)inRenderPassBeginInfo.FrameBuffer;
    VkRenderPassBeginInfo RenderPassBeginInfo = VulkanUtils::Initializers::RenderPassBeginInfo(RenderPass->GetRenderPassHandle(), nullptr);
    
    const VkRect2D* RenderArea = RenderPass->GetRenderLayout()->GetRenderArea();
    RenderPassBeginInfo.renderArea.offset.x = RenderArea->offset.x;
    RenderPassBeginInfo.renderArea.offset.y = RenderArea->offset.y;
    RenderPassBeginInfo.renderArea.extent.width = RenderArea->extent.width;
    RenderPassBeginInfo.renderArea.extent.height = RenderArea->extent.height;

    RenderPassBeginInfo.clearValueCount = inRenderPassBeginInfo.NumClearValues;
    RenderPassBeginInfo.pClearValues = (VkClearValue*)inRenderPassBeginInfo.ClearValues;

    RenderPassBeginInfo.framebuffer = FrameBuffer->GetFrameBufferHandle();

    vkCmdBeginRenderPass(CommandBufferHandle, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::EndRenderPass()
{
    VE_PROFILE_VULKAN_FUNCTION();
    
    vkCmdEndRenderPass(CommandBufferHandle);
}

void VulkanCommandBuffer::BindPipeline(const IPipeline* inPipeline)
{
    ASSERT(inPipeline->GetBindPoint() != EPipelineBindPoint::Undefined, "[VulkanCommandBuffer]: Trying to bind a pipeline that is undefined, it has to be Graphics as thats whats only supported right now!")

    VulkanPipeline* Pipeline = (VulkanPipeline*)inPipeline;
    vkCmdBindPipeline(CommandBufferHandle, (VkPipelineBindPoint)inPipeline->GetBindPoint(), *Pipeline->GetPipelineHandle());
}

void VulkanCommandBuffer::Draw(uint32 inNumVertices, uint32 inFirstVertexIndex)
{
    vkCmdDraw(CommandBufferHandle, inNumVertices, 1, inFirstVertexIndex, 0);
}

void VulkanCommandBuffer::DrawIndexed(uint32 inNumIndices, uint32 inFirstIndex, int32 inVertexOffset)
{
    vkCmdDrawIndexed(CommandBufferHandle, inNumIndices, 1, inFirstIndex, inVertexOffset, 0);
}

void VulkanCommandBuffer::DrawInstanced(uint32 inNumVertices, uint32 inNumInstances, uint32 inFirstVertexIndex, uint32 inFirstInstanceIndex)
{
    vkCmdDraw(CommandBufferHandle, inNumVertices, inNumInstances, inFirstVertexIndex, inFirstInstanceIndex);
}

void VulkanCommandBuffer::DrawIndexedInstanced(uint32 inNumIndices, uint32 inNumInstances, uint32 inFirstIndex, uint32 inVertexOffset, uint32 inFirstInstanceIndex)
{
    vkCmdDrawIndexed(CommandBufferHandle, inNumIndices, inNumInstances, inFirstIndex, inVertexOffset, inFirstInstanceIndex);
}

void VulkanCommandBuffer::CreateWaitFence()
{
    VE_ASSERT(WaitFence == VK_NULL_HANDLE, VE_TEXT("[VulkanCommandBuffer]: Potential GPU Memory Leak!! Cannot create a wait fence twice!!"));

    // Fence (Used to check draw command buffer completion)
    // Create in signaled state so we don't wait on first render of each command buffer
    VkFenceCreateInfo FenceCreateInfo = VulkanUtils::Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT, nullptr);
    VK_CHECK_RESULT(vkCreateFence(*Device->GetDeviceHandle(), &FenceCreateInfo, nullptr, &WaitFence), "[VulkanCommandBuffer]: Failed to create a fence that is used to check draw command buffer completion!");
}

void VulkanCommandBuffer::AllocateCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = VulkanUtils::Initializers::CommandBufferAllocateInfo();
	CommandBufferAllocateInfo.commandPool = CommandPool->GetCommandPoolHandle();
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;

    AllocatedBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(*Device->GetDeviceHandle(), &CommandBufferAllocateInfo, &CommandBufferHandle), "[VulkanCommandBuffer]: Failed to create a command buffer!");
}

void VulkanCommandBuffer::AllocateCommandBuffer(const CommandBufferConfig& inConfig)
{
    VE_PROFILE_VULKAN_FUNCTION();

    VE_ASSERT(CommandBufferHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanCommandBuffer]: Potential GPU Memory Leak!! Cannot create command buffer twice!!"));
    
    VkCommandBufferAllocateInfo CommandBufferAllocateInfo = VulkanUtils::Initializers::CommandBufferAllocateInfo();
    CommandBufferAllocateInfo.commandPool = CommandPool->GetCommandPoolHandle();
    CommandBufferAllocateInfo.level = VulkanTypeConverter::ConvertCmdBuffFlagsToVk(inConfig.Flags);
    CommandBufferAllocateInfo.commandBufferCount = inConfig.NumBuffersToAllocate;

    AllocatedBufferCount = inConfig.NumBuffersToAllocate;

    VK_CHECK_RESULT(vkAllocateCommandBuffers(*Device->GetDeviceHandle(), &CommandBufferAllocateInfo, &CommandBufferHandle), "[VulkanCommandBuffer]: Failed to create a command buffer!");
}

void VulkanCommandBuffer::FreeCommandBuffer()
{
	VE_PROFILE_VULKAN_FUNCTION();

    VE_ASSERT(CommandBufferHandle != VK_NULL_HANDLE, VE_TEXT("[VulkanCommandBuffer]: Cannot free an already invalid command buffer!!"));

	vkFreeCommandBuffers(*Device->GetDeviceHandle(), CommandPool->GetCommandPoolHandle(), AllocatedBufferCount, &CommandBufferHandle);
    CommandBufferHandle = VK_NULL_HANDLE;

    vkDestroyFence(*Device->GetDeviceHandle(), WaitFence, nullptr);
    WaitFence = VK_NULL_HANDLE;

    CommandPool->EraseCommandBuffer(this);
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
//
//void VulkanCommandBuffer::BeginRenderPass(const VulkanRenderPass* renderPass, VulkanFrameBuffer* frameBuffer)
//{
//	VE_PROFILE_VULKAN_FUNCTION();
//
//	VkRenderPassBeginInfo RenderPassBeginInfo = VulkanUtils::Initializers::RenderPassBeginInfo(renderPass->GetRenderPassHandle(), nullptr);
//	const VkRect2D* RenderArea = renderPass->GetRenderLayout()->GetRenderArea();
//	RenderPassBeginInfo.renderArea.offset.x = RenderArea->offset.x;
//	RenderPassBeginInfo.renderArea.offset.y = RenderArea->offset.y;
//	RenderPassBeginInfo.renderArea.extent.width = RenderArea->extent.width;
//	RenderPassBeginInfo.renderArea.extent.height = RenderArea->extent.height;
//
//	RenderPassBeginInfo.clearValueCount = renderPass->GetRenderLayout()->GetNumClearValues();
//	RenderPassBeginInfo.pClearValues = renderPass->GetRenderLayout()->GetClearValues();
//
//	RenderPassBeginInfo.framebuffer = frameBuffer->GetFrameBufferHandle();
//
//	vkCmdBeginRenderPass(CommandBufferHandle, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//}

//void VulkanCommandBuffer::EndRenderPass()
//{
//	VE_PROFILE_VULKAN_FUNCTION();
//
//	vkCmdEndRenderPass(CommandBufferHandle);
//}

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

    VE_ASSERT(CommandPoolHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanCommandPool]: Potential GPU Memory Leak! A vulkan command pool can only be created once!!"));

	VkCommandPoolCreateInfo CommandPoolInfo = VulkanUtils::Initializers::CommandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		queueFamilyIndex, nullptr);

	VK_CHECK_RESULT(vkCreateCommandPool(*Device->GetDeviceHandle(), &CommandPoolInfo, nullptr, &CommandPoolHandle), "[VulkanCommandPool]: Failed to create a command pool!");
}

void VulkanCommandPool::EraseCommandBuffer(VulkanCommandBuffer* inCmdBuffer)
{
    VE_PROFILE_VULKAN_FUNCTION();
    
    VE_ASSERT(*inCmdBuffer->GetCommandBufferHandle() == VK_NULL_HANDLE, VE_TEXT("[VulkanCommandPool]: Potential GPU Memory Leak! A vulkan command buffer shoudn't be getting erased before being freed!!"));
    for (auto It = CommandBuffers.begin(); It != CommandBuffers.end(); ++It)
    {
        if (*It == inCmdBuffer)
        {
            CommandBuffers.erase(It);
            break;
        }
    }
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

