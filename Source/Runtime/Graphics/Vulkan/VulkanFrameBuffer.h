/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanRenderPass.h"
#include <Runtime/Graphics/FrameBuffer.h>

/**
* Representation of vulkan frame buffer
*/
class VRIXIC_API VulkanFrameBuffer : public IFrameBuffer
{
private:
	VulkanDevice* Device;
	VulkanRenderPass* RenderPass;
	VkFramebuffer FrameBufferHandle;

    uint32 NumAttachments; // number of attachments 
    Extent2D Extent; // the extent of the buffer

public:
	/**
	* @param inRenderPass - render pass associated with this frame buffer
	*/
	VulkanFrameBuffer(VulkanDevice* inDevice);
	~VulkanFrameBuffer();

	VulkanFrameBuffer(const VulkanFrameBuffer& other) = delete;
	VulkanFrameBuffer operator=(const VulkanFrameBuffer& other) = delete;
public:
	/**
	* Creates the Frame Buffer
	* 
	* @param inNumAttachments - Attachments count
	* @param inAttachments - all attachments for the frame buffer
	* @param inExtent - extent of the frame buffer
	*/
    void Create(uint32 inNumAttachments, const VkImageView* inAttachments, const VkExtent2D* inExtent, VulkanRenderPass* inRenderPass);

    /**
    * Creates the Frame Buffer
    *
    * @param inFrameBufferConfig the configuration used to allocate the frame buffer 
    */
    void Create(const FrameBufferConfig& inFrameBufferConfig);

	/**
	* Destroys this frame buffer 
	*/
	void DestroyBuffer();

public:
    /**
    * @returns Extend2D the extent of the framebuffer in screen space
    */
    virtual Extent2D GetResolution() const override;

    /**
    * @return uint32 the number of attachments that are attached to this frame buffer
    */
    virtual uint32 GetNumAttachments() const override;

    /**
    * @returns IRenderPass* a pointer to the render pass that was used to create the frame buffer (render pass that is associated with this frame buffer)
    */
    virtual IRenderPass* GetRenderPassHandle() const override;

    /**
    * @returns VkFrameBuffer the handle to the frame buffer 
    */
	inline VkFramebuffer GetFrameBufferHandle() const
	{
		return FrameBufferHandle;
	}
};

