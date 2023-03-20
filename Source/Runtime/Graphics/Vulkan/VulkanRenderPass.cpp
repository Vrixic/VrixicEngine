#include "VulkanRenderPass.h"

//VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, uint32 numAttachments, VkAttachmentDescription* attachments,
//	uint32 numColorAttachment, VkAttachmentReference* colorReference, VkAttachmentDescription* depthReference,
//	uint32 numInputAttachments, VkAttachmentReference* inputAttachments,
//	uint32 numPreserveAttachments, VkAttachmentReference* preserveAttachments, uint32 numClearValues,
//	VkClearValue* clearValues )
//{
//
//}

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, VulkanRenderLayout& renderLayout)
	: Device(device), RenderLayout(renderLayout)
{
	VkSubpassDescription SubpassDescription = { };
	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.colorAttachmentCount = renderLayout.GetNumColorAttachments();
	SubpassDescription.pColorAttachments = renderLayout.GetColorReference();
	SubpassDescription.pDepthStencilAttachment = renderLayout.GetDepthReference();
	SubpassDescription.inputAttachmentCount = renderLayout.GetNumInputAttachments();
	SubpassDescription.pInputAttachments = renderLayout.GetInputAttachments();
	SubpassDescription.preserveAttachmentCount = renderLayout.GetNumPreserveAttachments();
	SubpassDescription.pPreserveAttachments = renderLayout.GetPreserveAttachments();
	SubpassDescription.pResolveAttachments = renderLayout.GetResolveAttachments();

	// Subpass dependencies for layout transitions
	VkSubpassDependency Dependencies[2];

	Dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependencies[0].dstSubpass = 0;
	Dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	Dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	Dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	Dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	Dependencies[1].srcSubpass = 0;
	Dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	Dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	Dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	Dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	Dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo RenderPassInfo = VulkanUtils::Initializers::RenderPassCreateInfo();
	RenderPassInfo.attachmentCount = renderLayout.GetNumAttachments();
	RenderPassInfo.pAttachments = renderLayout.GetAttachments();
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &SubpassDescription;
	RenderPassInfo.dependencyCount = 2;
	RenderPassInfo.pDependencies = Dependencies;

	VK_CHECK_RESULT(vkCreateRenderPass(*Device->GetDeviceHandle(), &RenderPassInfo, nullptr, &RenderPassHandle), "[VulkanRenderPass]: Failed to create a render pass!");
}

VulkanRenderPass::~VulkanRenderPass()
{
	vkDestroyRenderPass(*Device->GetDeviceHandle(), RenderPassHandle, nullptr);
}

