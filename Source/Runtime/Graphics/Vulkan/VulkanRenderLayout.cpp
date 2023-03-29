/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#include "VulkanRenderLayout.h"
#include <Misc/Defines/VulkanProfilerDefines.h>

VulkanRenderLayout::VulkanRenderLayout(VulkanDevice* device, uint32 numColorAttachments, VkRect2D& renderArea,
		VkExtent2D* extent2D)
	: Device(device), NumColorAttachments(numColorAttachments), RenderArea(renderArea)
{
	VE_PROFILE_VULKAN_FUNCTION();

	ColorReference = { };
	DepthReference = { };
	
	Extent2D = { };
	if (extent2D != nullptr)
	{
		Extent2D.width = extent2D->width;
		Extent2D.height = extent2D->height;
	}

	//Extent3D = { };
}

VulkanRenderLayout::~VulkanRenderLayout() { }

void VulkanRenderLayout::SetAttachments(std::vector<VkAttachmentDescription>& attachments)
{
	VE_PROFILE_VULKAN_FUNCTION();

	for (uint32 i = 0; i < attachments.size(); ++i)
	{
		Attachments.push_back(attachments[i]);
	}
}

void VulkanRenderLayout::SetInputAttachments(std::vector<VkAttachmentReference>& attachments)
{
	VE_PROFILE_VULKAN_FUNCTION();

	for (uint32 i = 0; i < attachments.size(); ++i)
	{
		InputAttachments.push_back(attachments[i]);
	}
}

void VulkanRenderLayout::SetPreserveAttachments(std::vector<uint32>& attachments)
{
	VE_PROFILE_VULKAN_FUNCTION();

	for (uint32 i = 0; i < attachments.size(); ++i)
	{
		PreserveAttachments.push_back(attachments[i]);
	}
}

void VulkanRenderLayout::SetResolveAttachments(std::vector<VkAttachmentReference>& attachments)
{
	VE_PROFILE_VULKAN_FUNCTION();

	for (uint32 i = 0; i < attachments.size(); ++i)
	{
		ResolveAttachments.push_back(attachments[i]);
	}
}

void VulkanRenderLayout::SetClearValues(std::vector<VkClearValue>& clearValues)
{
	VE_PROFILE_VULKAN_FUNCTION();

	for (uint32 i = 0; i < clearValues.size(); ++i)
	{
		ClearValues.push_back(clearValues[i]);
	}
}

void VulkanRenderLayout::SetColorReference(VkAttachmentReference colorReference)
{
	VE_PROFILE_VULKAN_FUNCTION();

	ColorReference = colorReference;
}

void VulkanRenderLayout::SetDepthReference(VkAttachmentReference depthReference)
{
	VE_PROFILE_VULKAN_FUNCTION();

	DepthReference = depthReference;
}

void VulkanRenderLayout::SetRenderArea(VkRect2D& renderArea)
{
	VE_PROFILE_VULKAN_FUNCTION();

	RenderArea = renderArea;
}

void VulkanRenderLayout::SetExtent2D(VkExtent2D& extent2D)
{
	VE_PROFILE_VULKAN_FUNCTION();

	Extent2D = extent2D;
}

