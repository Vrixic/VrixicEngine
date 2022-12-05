#pragma once
#include "VulkanRenderLayout.h"

/**
* Representation of vulkan render pass 
*/
class VulkanRenderPass
{
private:
	VulkanDevice* Device;
	VkRenderPass RenderPassHandle;

	VulkanRenderLayout RenderLayout;

public:
	/*VulkanRenderPass(VulkanDevice* device, uint32 numAttachments, VkAttachmentDescription* attachments,
		uint32 numColorAttachment, VkAttachmentReference* colorReference, VkAttachmentReference* depthReference,
		uint32 numInputAttachments, VkAttachmentReference* inputAttachments,
		uint32 numPreserveAttachments, VkAttachmentReference* preserveAttachments, uint32 numClearValues,
		VkClearValue* clearValues);*/
	
	/**
	* @Param inRenderLayout - the render layout used to create the render pass 
	* 
	* @Note: Creates the RenderPass
	*/
	VulkanRenderPass(VulkanDevice* inDevice, VulkanRenderLayout& inRenderLayout);

	~VulkanRenderPass();

	VulkanRenderPass(const VulkanRenderPass& other) = delete;
	VulkanRenderPass operator=(const VulkanRenderPass& other) = delete;

public:
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
};

