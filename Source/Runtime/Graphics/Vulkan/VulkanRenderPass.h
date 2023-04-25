/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanRenderLayout.h"
#include <Runtime/Graphics/RenderPass.h>
#include <Runtime/Graphics/RenderPassGenerics.h>

/**
* Representation of vulkan render pass 
*/
class VRIXIC_API VulkanRenderPass : public IRenderPass
{
public:	
	/**
	* @param inRenderLayout - the render layout used to create the render pass 
	* 
	* @remarks Creates the RenderPass if bCreateDefault is true (Default = true) 
	*/
	VulkanRenderPass(VulkanDevice* inDevice, VulkanRenderLayout& inRenderLayout, std::vector<VkSubpassDependency>& inSubpassDependencies);

    /**
    * @param inRenderLayout - the render layout used to create the render pass
    * @param inRenderConfig - configuration used for the creation of the renderpass
    *
    * @remarks Creates the RenderPass if bCreateDefault is true (Default = true)
    */
	VulkanRenderPass(VulkanDevice* inDevice, VulkanRenderLayout& inRenderLayout, const FRenderPassConfig& inRenderPassConfig);

	~VulkanRenderPass();

	VulkanRenderPass(const VulkanRenderPass& other) = delete;
	VulkanRenderPass operator=(const VulkanRenderPass& other) = delete;

public:
	/**
	* Creates a render pass with the render layout passed in and subpass dependencies passed in
	* 
	* @param inSubpassDependencies the subpass dependencies of the render pass 
	* @remarks asserts if renderpass is valid, only create one if current is invalid, otherwise destory and then create 
	*/
	void Create(std::vector<VkSubpassDependency>& inSubpassDependencies);

	/**
	* Updates the render area 
	*/
	void UpdateRenderArea(VkRect2D& inRenderArea)
	{
		RenderLayout.SetRenderArea(inRenderArea);
	}

	/**
	* Updates the extent
	*/
	void UpdateExtent2D(VkExtent2D& inExtent2D)
	{
		RenderLayout.SetExtent2D(inExtent2D);
	}

public:
	inline const VkRenderPass* GetRenderPassHandle() const
	{
		return &RenderPassHandle;
	}

	inline const VulkanRenderLayout* GetRenderLayout() const
	{
		return &RenderLayout;
	}

    inline VkSampleCountFlagBits GetSampleCountFlagBits() const
    {
        return SampleCountFlagBits;
    }

    inline uint32 GetNumColorAttachments() const
    {
        return NumColorAttachments;
    }

private:
    VulkanDevice* Device;
    VkRenderPass RenderPassHandle;

    VulkanRenderLayout RenderLayout;

    VkSampleCountFlagBits SampleCountFlagBits;
    uint32 DepthStencilAttachmentIndex;

    uint32 NumColorAttachments;
};

