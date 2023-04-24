/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanRenderPass.h"
#include <Misc/Defines/VulkanProfilerDefines.h>
#include <Runtime/Graphics/Vulkan/VulkanTypeConverter.h>

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, VulkanRenderLayout& renderLayout, std::vector<VkSubpassDependency>& inSubpassDependencies)
	: Device(device), RenderLayout(renderLayout), RenderPassHandle(VK_NULL_HANDLE)
{
	Create(inSubpassDependencies);
}

VulkanRenderPass::VulkanRenderPass(VulkanDevice* inDevice, VulkanRenderLayout& inRenderLayout, const RenderPassConfig& inRenderPassConfig)
    : Device(inDevice), RenderLayout(inRenderLayout), RenderPassHandle(VK_NULL_HANDLE)
{
    /*uint32*/ NumColorAttachments = inRenderPassConfig.GetNumColorAttachments();
    uint32 NumAttachments = NumColorAttachments;

    // depth stencil attachemnt check
    bool bHasDepthStencil = false;
    if (inRenderPassConfig.DepthStencilAttachment.Format != EPixelFormat::Undefined)
    {
        NumAttachments++;
        bHasDepthStencil = true;
    }

    // create our attachment descriptions
    VkSampleCountFlagBits SampleCountBits = VulkanTypeConverter::ConvertSampleCountToVk(inRenderPassConfig.NumSamples);
    uint32 NumDescriptions = SampleCountBits > VK_SAMPLE_COUNT_1_BIT ? (NumAttachments * 2) : NumAttachments;
    VkAttachmentDescription* AttachmentDescs = new VkAttachmentDescription[NumDescriptions];

    for (uint32 i = 0; i < NumColorAttachments; i++)
    {
        AttachmentDescs[i] = VulkanTypeConverter::ConvertAttachmentDescToVk(inRenderPassConfig.ColorAttachments[i], VK_SAMPLE_COUNT_1_BIT);
    }

    if (bHasDepthStencil)
    {
        AttachmentDescs[NumColorAttachments] = VulkanTypeConverter::ConvertAttachmentDescToVk(inRenderPassConfig.DepthStencilAttachment, SampleCountBits);
    }

    // If multi-sampling is enabled 
    if (SampleCountBits > VK_SAMPLE_COUNT_1_BIT)
    {
        // add multi-sample attachments after the base attachments for the color and depth.stencil
        for (uint32 i = 0; i < NumColorAttachments; i++)
        {
            AttachmentDescs[NumAttachments + i] = VulkanTypeConverter::ConvertAttachmentDescToVk(inRenderPassConfig.ColorAttachments[i], SampleCountBits);

            // don't rely on any other attachment to load for the base ones 
            AttachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }
    
    SampleCountFlagBits = SampleCountBits;
    const bool bMultiSamplingEnabled = (SampleCountBits > VK_SAMPLE_COUNT_1_BIT);

    // store depth stencil index 
    if (bHasDepthStencil)
    {
        DepthStencilAttachmentIndex = NumColorAttachments;
    }

    // Attachment references
    std::vector<VkAttachmentReference> ColorReferences;
    std::vector<VkAttachmentReference> MSAAReferences;
    VkAttachmentReference DepthStencilReference;

    ColorReferences.resize(NumColorAttachments);
    for (uint32 i = 0; i < NumColorAttachments; i++)
    {
        ColorReferences[i] = { };
        ColorReferences[i].attachment = i;
        ColorReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if (bHasDepthStencil)
    {
        DepthStencilReference = { };
        DepthStencilReference.attachment = DepthStencilAttachmentIndex;
        DepthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if (bMultiSamplingEnabled)
    {
        MSAAReferences.resize(NumColorAttachments);
        for (uint32 i = 0; i < NumColorAttachments; i++)
        {
            MSAAReferences[i] = { };
            MSAAReferences[i].attachment = NumAttachments + i;
            MSAAReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    // Subpass Descriptions
    VkSubpassDescription SubpassDesc = { };
    SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDesc.colorAttachmentCount = NumColorAttachments;
    if (bMultiSamplingEnabled && NumColorAttachments > 0)
    {
        // Swap as the multi-sampled ones are now the actual color attachments and the base color references 
        // are not
        SubpassDesc.pColorAttachments = MSAAReferences.data();
        SubpassDesc.pResolveAttachments = ColorReferences.data();
    }
    else
    {
        SubpassDesc.pColorAttachments = ColorReferences.data();
        SubpassDesc.pResolveAttachments = nullptr;
    }

    SubpassDesc.pDepthStencilAttachment = (bHasDepthStencil ? &DepthStencilReference : nullptr);

    // Sub pass Dependency
    VkSubpassDependency SubpassDependency = { };
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.srcAccessMask = 0;
    SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
    SubpassDependency.dependencyFlags = 0;

    // Create the actual renderpass
    VkRenderPassCreateInfo RenderPassInfo = VulkanUtils::Initializers::RenderPassCreateInfo();
    RenderPassInfo.attachmentCount = (bMultiSamplingEnabled ? NumAttachments + NumColorAttachments : NumAttachments);
    RenderPassInfo.pAttachments = AttachmentDescs;
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &SubpassDesc;
    RenderPassInfo.dependencyCount = 1;
    RenderPassInfo.pDependencies = &SubpassDependency;

    VK_CHECK_RESULT(vkCreateRenderPass(*Device->GetDeviceHandle(), &RenderPassInfo, nullptr, &RenderPassHandle), "[VulkanRenderPass]: Failed to create a render pass!");

    delete[] AttachmentDescs;
}

VulkanRenderPass::~VulkanRenderPass()
{
    Device->WaitUntilIdle();

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
