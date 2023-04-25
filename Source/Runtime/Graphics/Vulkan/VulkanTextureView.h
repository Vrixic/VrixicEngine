/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanDevice.h"
#include <Runtime/Graphics/Texture.h>

/**
* A wrapper for VkImage and VkImage view, mainly used for depth and stenciling
*/
class VRIXIC_API VulkanTextureView final : public Texture
{
public:
	/**
	* @param inImageCreateInfo - Image creation info
	* 
	* @remarks Creates, Allocates, and Binds Image Memory 
	*/
	//VulkanTextureView(VulkanDevice* inDevice, VkImageCreateInfo& inImageCreateInfo);

    /**
    * @param inTextureConfig - texture configuration used to create the texture 
    *
    * @remarks Creates, Allocates, and Binds Image Memory
    */
    VulkanTextureView(VulkanDevice* inDevice, const FTextureConfig& inTextureConfig);

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
		uint32 inLayerCount);

    /**
    * Creates the image view for image that was created on construct
    *
    * @param inTextureViewConfig configuration used for the creation of the image view 
    */
    void CreateImageView(const FTextureViewConfig& inTextureViewConfig);

    /**
    * Creates a default image view from the image 
    */
    void CreateDefaultImageView();

private:
    /**
    * Only swapchains should use this version
    */
    VulkanTextureView()
        : Texture(ETextureType::Texture2D, FResourceBindFlags::ColorAttachment), Device(nullptr), 
        ImageHandle(VK_NULL_HANDLE), ImageMemory(VK_NULL_HANDLE), ViewHandle(VK_NULL_HANDLE), ImageFormat(VK_FORMAT_UNDEFINED), NumArrayLayers(0), NumMipLevels(0) { }

    /**
    * Creates a VkImage from the configuration settings passed in
    * 
    * @param inTextureConfig the configuration used to create the image 
    */
    void CreateImage(const FTextureConfig& inTextureConfig);

    /**
    * @returns VkImageAspectFlags aspect flags for this texture (from its Image Format)
    */
    VkImageAspectFlags GetAspectFlags() const;

public:
	inline const VkImage* GetImageHandle() const
	{
		return &ImageHandle;
	}

	inline const VkImageView* GetImageViewHandle() const
	{
		return &ViewHandle;
	}

private:
    friend class VulkanSwapChain;

    VulkanDevice* Device;

    VkImage ImageHandle;
    VkDeviceMemory ImageMemory;

    VkImageView ViewHandle;

    VkFormat ImageFormat;

    uint32 NumMipLevels;

    uint32 NumArrayLayers;
};
