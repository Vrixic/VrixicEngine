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

#if _DEBUG
	bool HasBeenCreated = false;
#endif

	//VulkanDescriptorSetsLayout* DescriptorSetsLayout;

public:
	VulkanPipelineLayout(VulkanDevice* inDevice)
		: Device(inDevice), PipelineLayoutHandle(VK_NULL_HANDLE) { }

	~VulkanPipelineLayout()
	{
		Device->WaitUntilIdle();

		vkDestroyPipelineLayout(*Device->GetDeviceHandle(), PipelineLayoutHandle, nullptr);
	}

	VulkanPipelineLayout(const VulkanPipelineLayout& other) = delete;
	VulkanPipelineLayout operator=(const VulkanPipelineLayout& other) = delete;

public:
	/*
	* Should Only be called once
	*/
	void Create(VulkanDescriptorSetsLayout* inDescriptorSetsLayout, std::vector<VkPushConstantRange>* inPushConstants)
	{
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VulkanUtils::Initializers::PipelineLayoutCreateInfo();
		PipelineLayoutCreateInfo.pSetLayouts = inDescriptorSetsLayout->DescriptorSetLayoutHandles.data();
		PipelineLayoutCreateInfo.setLayoutCount = inDescriptorSetsLayout->DescriptorSetLayoutHandles.size();

		if (inPushConstants != nullptr)
		{
			PipelineLayoutCreateInfo.pPushConstantRanges = inPushConstants->data();
			PipelineLayoutCreateInfo.pushConstantRangeCount = inPushConstants->size();
		}

#if _DEBUG
		ASSERT(!HasBeenCreated);
		HasBeenCreated = true;

		VK_CHECK_RESULT(vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
			&PipelineLayoutHandle));
#else
		vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
			&PipelineLayoutHandle);
#endif		
	}

	void CreateEmpty()
	{
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VulkanUtils::Initializers::PipelineLayoutCreateInfo();

#if _DEBUG
		ASSERT(!HasBeenCreated);
		HasBeenCreated = true;

		VK_CHECK_RESULT(vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
			&PipelineLayoutHandle));
#else
		vkCreatePipelineLayout(*Device->GetDeviceHandle(), &PipelineLayoutCreateInfo, nullptr,
			&PipelineLayoutHandle);
#endif	
	}

public:
	inline const VkPipelineLayout* GetPipelineLayoutHandle() const
	{
		return &PipelineLayoutHandle;
	}
};

/**
* Wrapper for vulkan pipeline 
*	Do not create this object, use VulkanGraphicsPipeline..
*/
class VulkanPipeline
{
protected:
	VulkanDevice* Device;
	VkPipeline PipelineHandle;

protected:
	VulkanPipeline(VulkanDevice* inDevice)
		: Device(inDevice), PipelineHandle(VK_NULL_HANDLE) {}

public:
	~VulkanPipeline()
	{
		Device->WaitUntilIdle();
		if (PipelineHandle != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(*Device->GetDeviceHandle(), PipelineHandle, nullptr);
		}
	}

	VulkanPipeline(const VulkanPipeline& other) = delete;
	VulkanPipeline operator=(const VulkanPipeline& other) = delete;

public:
	inline const VkPipeline* GetPipelineHandle() const
	{
		return &PipelineHandle;
	}
};

/**
* Represents a graphics vulkan pipeline 
*/
class VulkanGraphicsPipeline : public VulkanPipeline
{	
public:
	VulkanGraphicsPipeline(VulkanDevice* inDevice)
		: VulkanPipeline(inDevice)  {  }

public:
	/**
	* Creates a graphics pipeline 
	* 
	* @param inCreateInfo - graphics pipeline create info which is used for pipeline creation
	*/
	void Create(VkGraphicsPipelineCreateInfo& inCreateInfo)
	{
		vkCreateGraphicsPipelines(*Device->GetDeviceHandle(), VK_NULL_HANDLE, 1, &inCreateInfo, nullptr, &PipelineHandle);
	}
};
