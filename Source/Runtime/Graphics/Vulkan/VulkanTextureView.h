#pragma once
#include "VulkanDevice.h"

/**
* A wrapper for VkImage and VkImage view, mainly used for depth and stenciling
*/
class VRIXIC_API VulkanTextureView
{
private:
	VulkanDevice* Device;

	VkImage ImageHandle;
	VkDeviceMemory ImageMemory;

	VkImageView ViewHandle;

public:
	/**
	* @param inImageCreateInfo - Image creation info
	* 
	* @remarks Creates, Allocates, and Binds Image Memory 
	*/
	VulkanTextureView(VulkanDevice* inDevice, VkImageCreateInfo& inImageCreateInfo);

	~VulkanTextureView();

	VulkanTextureView(const VulkanTextureView& other) = delete;
	VulkanTextureView operator=(const VulkanTextureView& other) = delete;

public:
	/**
	* Creates the image view for image that was created on construct
	* 
	* @param inViewType - The image view type
	* @param inFormat - The image view format
	*/
	void CreateImageView(VkImageViewType inViewType, VkFormat& inFormat, uint32 inBaseMiplevel, uint32 inLevelCount, uint32 inBaseArrayLayer,
		uint32 inLayerCount, VkImageAspectFlags& inAspectFlags);

public:
	inline const VkImage* GetImageHandle() const
	{
		return &ImageHandle;
	}

	inline const VkImageView* GetImageViewHandle() const
	{
		return &ViewHandle;
	}
};
