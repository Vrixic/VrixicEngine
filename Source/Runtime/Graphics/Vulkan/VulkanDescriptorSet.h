#pragma once
#include "VulkanDevice.h"

/* ------------------------------------------------------------------------------- */
/**
* @TODO: Create a way to bind and update descriptor sets
*/
/* ------------------------------------------------------------------------------- */

/**
* Representation of a VkDescriptorSetLayout, expect it can hold multiple layouts
*/
class VulkanDescriptorSetsLayout
{
private:
	VulkanDevice* Device;
	std::vector<VkDescriptorSetLayout> DescriptorSetLayoutHandles;

public:
	VulkanDescriptorSetsLayout(VulkanDevice* inDevice);

	~VulkanDescriptorSetsLayout();

	VulkanDescriptorSetsLayout(const VulkanDescriptorSetsLayout& other) = delete;
	VulkanDescriptorSetsLayout operator=(const VulkanDescriptorSetsLayout& other) = delete;

public:
	/**
	* Creates a descriptor set layout
	*
	* @Param: layout bindings that will be used for layout creation
	*
	* @Return: the id to where the layout is located
	*/
	uint32 CreateDescriptorSetLayout(VulkanUtils::Descriptions::DescriptorSetLayoutBinding& inLayoutBinding)
	{
		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
		DescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		inLayoutBinding.WriteTo(DescriptorSetLayoutBinding);

		VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = VulkanUtils::Initializers::DescriptorSetLayoutCreateInfo();
		DescriptorSetLayoutCreateInfo.bindingCount = 1;
		DescriptorSetLayoutCreateInfo.pBindings = &DescriptorSetLayoutBinding;

		VkDescriptorSetLayout NewLayout;

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*Device->GetDeviceHandle(), &DescriptorSetLayoutCreateInfo, nullptr, &NewLayout));

		DescriptorSetLayoutHandles.push_back(NewLayout);

		return DescriptorSetLayoutHandles.size() - 1;
	}

public:
	/**
	* Returns a handle to a descriptor set layout
	*
	* @Param: the id to which the layout is located
	*
	* @Return: a descriptor set layout at the 'inLayoutId' specified
	*/
	const VkDescriptorSetLayout* GetLayoutHandle(uint32 inLayoutId) const
	{
		return &DescriptorSetLayoutHandles[inLayoutId];
	}
};

/**
* Representation of a VkDescriptorPool
* Can be used to allocate descriptor sets
* Sets can be shared out to any user as they do not need to be kept track of...
*/
class VulkanDescriptorPool
{
private:
	VulkanDevice* Device;
	VkDescriptorPool DescriptorPoolHandle;

	uint32 MaxDescriptorSets;

	const VulkanDescriptorSetsLayout* DescriptorSetsLayout;

public:
	VulkanDescriptorPool(VulkanDevice* inDevice, const VulkanDescriptorSetsLayout& inSetsLayout, uint32 inMaxSets);

	~VulkanDescriptorPool();

	/**
	* Uses this pool to allocate a descriptor set
	*
	* @Param 1: count of descriptor sets to create
	* @Param 2: descriptor set to be filled
	* @Param 3: the layout id used to create the decriptor set
	*
	* @Return: if allocated of set(s) was successfull
	*/
	bool AllocateDescriptorSets(uint32 inDescriptorSetCount, VkDescriptorSet* outDescriptorSet, uint32 inLayoutId) const
	{
		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = VulkanUtils::Initializers::DescriptorSetAllocateInfo();
		DescriptorSetAllocateInfo.descriptorPool = DescriptorPoolHandle;
		DescriptorSetAllocateInfo.descriptorSetCount = inDescriptorSetCount;
		DescriptorSetAllocateInfo.pSetLayouts = DescriptorSetsLayout->GetLayoutHandle(inLayoutId);

		VkResult Result = vkAllocateDescriptorSets(*Device->GetDeviceHandle(), &DescriptorSetAllocateInfo, outDescriptorSet);
		VK_CHECK_RESULT(Result);

		return Result == VK_SUCCESS;
	}

public:
	inline const VkDescriptorPool* GetDescriptorPoolHandle() const
	{
		return &DescriptorPoolHandle;
	}
};
