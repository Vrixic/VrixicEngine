#pragma once
#include "VulkanDescriptorSet.h"

/* ------------------------------------------------------------------------------- */
/**
* @TODO: Complete vulkan pipeline creation 
*/
/* ------------------------------------------------------------------------------- */

/**
* Defines pipeline creation information, also includes a descriptor set layouts and push constant ranges
*/
class VulkanPipelineLayout
{
private:
	VulkanDevice* Device;
	VkPipelineLayout PipelineLayoutHandle;

	VulkanDescriptorSetsLayout DescriptorSetsLayout;
	std::vector<VkPushConstantRange> PushConstantRanges;

public:
	VulkanPipelineLayout(VulkanDevice* inDevice);

	~VulkanPipelineLayout();

	VulkanPipelineLayout(const VulkanPipelineLayout& other) = delete;
	VulkanPipelineLayout operator=(const VulkanPipelineLayout& other) = delete;
public:
	inline const VkPipelineLayout* GetPipelineLayoutHandle() const
	{
		return &PipelineLayoutHandle;
	}
};

/**
* Wrapper for vulkan pipeline 
*/
class VulkanPipeline
{
private:
	VulkanDevice* Device;
	VkPipeline PipelineHandle;

public:
	VulkanPipeline(VulkanDevice* inDevice, VulkanPipelineLayout& inPipelineLayout);

	~VulkanPipeline();

	VulkanPipeline(const VulkanPipeline& other) = delete;
	VulkanPipeline operator=(const VulkanPipeline& other) = delete;
public:
	inline const VkPipeline* GetPipelineHandle() const
	{
		return &PipelineHandle;
	}
};
