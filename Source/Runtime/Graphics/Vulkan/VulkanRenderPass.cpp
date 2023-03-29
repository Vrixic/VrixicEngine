/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanRenderPass.h"
#include <Misc/Defines/VulkanProfilerDefines.h>

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, VulkanRenderLayout& renderLayout, std::vector<VkSubpassDependency>& inSubpassDependencies)
	: Device(device), RenderLayout(renderLayout), RenderPassHandle(VK_NULL_HANDLE)
{
	VE_PROFILE_VULKAN_FUNCTION();

	Create(inSubpassDependencies);
}

VulkanRenderPass::~VulkanRenderPass()
{
	VE_PROFILE_VULKAN_FUNCTION();

	vkDestroyRenderPass(*Device->GetDeviceHandle(), RenderPassHandle, nullptr);
}

//void VulkanRenderPass::CreateDefault()
//{
//	VE_ASSERT(RenderPassHandle == VK_NULL_HANDLE, "[VulkanRenderPass]: Renderpass already allocated, destroy before recreating another...!");
//
//	VkSubpassDescription SubpassDescription = { };
//	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	SubpassDescription.colorAttachmentCount = RenderLayout.GetNumColorAttachments();
//	SubpassDescription.pColorAttachments = RenderLayout.GetColorReference();
//	SubpassDescription.pDepthStencilAttachment = RenderLayout.GetDepthReference();
//	SubpassDescription.inputAttachmentCount = RenderLayout.GetNumInputAttachments();
//	SubpassDescription.pInputAttachments = RenderLayout.GetInputAttachments();
//	SubpassDescription.preserveAttachmentCount = RenderLayout.GetNumPreserveAttachments();
//	SubpassDescription.pPreserveAttachments = RenderLayout.GetPreserveAttachments();
//	SubpassDescription.pResolveAttachments = RenderLayout.GetResolveAttachments();
//
//	// Subpass dependencies for layout transitions
//	VkSubpassDependency Dependencies[2];
//
//	Dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//	Dependencies[0].dstSubpass = 0;
//	Dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//	Dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	Dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	Dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	Dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	Dependencies[1].srcSubpass = 0;
//	Dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//	Dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	Dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//	Dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	Dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	Dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	VkRenderPassCreateInfo RenderPassInfo = VulkanUtils::Initializers::RenderPassCreateInfo();
//	RenderPassInfo.attachmentCount = RenderLayout.GetNumAttachments();
//	RenderPassInfo.pAttachments = RenderLayout.GetAttachments();
//	RenderPassInfo.subpassCount = 1;
//	RenderPassInfo.pSubpasses = &SubpassDescription;
//	RenderPassInfo.dependencyCount = 2;
//	RenderPassInfo.pDependencies = Dependencies;
//
//	VK_CHECK_RESULT(vkCreateRenderPass(*Device->GetDeviceHandle(), &RenderPassInfo, nullptr, &RenderPassHandle), "[VulkanRenderPass]: Failed to create a render pass!");
//}

void VulkanRenderPass::Create(std::vector<VkSubpassDependency>& inSubpassDependencies)
{
	VE_ASSERT(RenderPassHandle == VK_NULL_HANDLE, "[VulkanRenderPass]: Renderpass already allocated, destroy before recreating another...!");

	VkSubpassDescription SubpassDescription = { };
	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.colorAttachmentCount = RenderLayout.GetNumColorAttachments();
	SubpassDescription.pColorAttachments = RenderLayout.GetColorReference();
	SubpassDescription.pDepthStencilAttachment = RenderLayout.GetDepthReference();
	SubpassDescription.inputAttachmentCount = RenderLayout.GetNumInputAttachments();
	SubpassDescription.pInputAttachments = RenderLayout.GetInputAttachments();
	SubpassDescription.preserveAttachmentCount = RenderLayout.GetNumPreserveAttachments();
	SubpassDescription.pPreserveAttachments = RenderLayout.GetPreserveAttachments();
	SubpassDescription.pResolveAttachments = RenderLayout.GetResolveAttachments();

	VkRenderPassCreateInfo RenderPassInfo = VulkanUtils::Initializers::RenderPassCreateInfo();
	RenderPassInfo.attachmentCount = RenderLayout.GetNumAttachments();
	RenderPassInfo.pAttachments = RenderLayout.GetAttachments();
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &SubpassDescription;
	RenderPassInfo.dependencyCount = inSubpassDependencies.size();
	RenderPassInfo.pDependencies = inSubpassDependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(*Device->GetDeviceHandle(), &RenderPassInfo, nullptr, &RenderPassHandle), "[VulkanRenderPass]: Failed to create a render pass!");
}
