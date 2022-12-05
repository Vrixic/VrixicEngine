#pragma once
#include "VulkanRenderPass.h"

/**
* Representation of vulkan frame buffer
*/
class VulkanFrameBuffer
{
private:
	VulkanDevice* Device;
	VulkanRenderPass* RenderPass;
	VkFramebuffer FrameBufferHandle;

public:
	/**
	* @Param inRenderPass - render pass associated with this frame buffer
	*/
	VulkanFrameBuffer(VulkanDevice* inDevice, VulkanRenderPass* inRenderPass);
	~VulkanFrameBuffer();

	VulkanFrameBuffer(const VulkanFrameBuffer& other) = delete;
	VulkanFrameBuffer operator=(const VulkanFrameBuffer& other) = delete;
public:
	/**
	* Allocates the Frame Buffer
	* 
	* @Param inNumAttachments - Attachments count
	* @Param inAttachments - all attachments for the frame buffer
	* @Param inExtent - extent of the frame buffer
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

