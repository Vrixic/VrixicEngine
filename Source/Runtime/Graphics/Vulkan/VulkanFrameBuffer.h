/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanRenderPass.h"

/**
* Representation of vulkan frame buffer
*/
class VRIXIC_API VulkanFrameBuffer
{
private:
	VulkanDevice* Device;
	VulkanRenderPass* RenderPass;
	VkFramebuffer FrameBufferHandle;

public:
	/**
	* @param inRenderPass - render pass associated with this frame buffer
	*/
	VulkanFrameBuffer(VulkanDevice* inDevice, VulkanRenderPass* inRenderPass);
	~VulkanFrameBuffer();

	VulkanFrameBuffer(const VulkanFrameBuffer& other) = delete;
	VulkanFrameBuffer operator=(const VulkanFrameBuffer& other) = delete;
public:
	/**
	* Allocates the Frame Buffer
	* 
	* @param inNumAttachments - Attachments count
	* @param inAttachments - all attachments for the frame buffer
	* @param inExtent - extent of the frame buffer
	*/
	void AllocateBuffer(uint32 inNumAttachments, const VkImageView* inAttachments, const VkExtent2D* inExtent);

	/**
	* Destroys this frame buffer 
	*/
	void DestroyBuffer();

public:
	inline VkFramebuffer GetFrameBufferHandle() const
	{
		return FrameBufferHandle;
	}
};

