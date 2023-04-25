/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanBuffer.h"

/**
* Representation of a VkDescriptorSetLayout, except it can hold multiple layouts
*/
class VRIXIC_API VulkanDescriptorSetsLayout
{
private:
	friend class VulkanPipelineLayout;

	VulkanDevice* Device;
	std::vector<VkDescriptorSetLayout> DescriptorSetLayoutHandles;

public:
	VulkanDescriptorSetsLayout(VulkanDevice* inDevice)
		: Device(inDevice) { }

	~VulkanDescriptorSetsLayout()
	{
		Device->WaitUntilIdle();

		for (uint32 i = 0; i < DescriptorSetLayoutHandles.size(); ++i)
		{
			vkDestroyDescriptorSetLayout(*Device->GetDeviceHandle(), DescriptorSetLayoutHandles[i], nullptr);
		}
	}

	VulkanDescriptorSetsLayout(const VulkanDescriptorSetsLayout& other) = delete;
	VulkanDescriptorSetsLayout operator=(const VulkanDescriptorSetsLayout& other) = delete;

public:
	/**
	* Creates a descriptor set layout
	*
	* @param layout bindings that will be used for layout creation
	* @param inDescriptorSetLayoutCreateInfo Information for changing how the layout is created
	*
	* @return the id to where the layout is located
	*/
	uint32 CreateDescriptorSetLayout(VulkanUtils::Descriptions::FDescriptorSetLayoutBinding& inLayoutBinding, VulkanUtils::Descriptions::FDescriptorSetLayoutCreateInfo& inDescriptorSetLayoutCreateInfo)
	{
		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
		DescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		inLayoutBinding.WriteTo(DescriptorSetLayoutBinding);

		VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = VulkanUtils::Initializers::DescriptorSetLayoutCreateInfo();
		inDescriptorSetLayoutCreateInfo.WriteTo(DescriptorSetLayoutCreateInfo);
		DescriptorSetLayoutCreateInfo.bindingCount = 1;
		DescriptorSetLayoutCreateInfo.pBindings = &DescriptorSetLayoutBinding;

		VkDescriptorSetLayout NewLayout;
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*Device->GetDeviceHandle(), &DescriptorSetLayoutCreateInfo, nullptr, &NewLayout), "[VulkanDescriptorSetsLayout]: Failed to create a descriptor set layout!");

		DescriptorSetLayoutHandles.push_back(NewLayout);

		return DescriptorSetLayoutHandles.size() - 1;
	}

    /**
    * Creates a descriptor set layouts
    *
    * @param inLayoutBindings bindings that will be used for layout creation
    * @param inBindingCount count of bindings 
    * @param inDescriptorSetLayoutCreateInfo Information for changing how the layout is created
    *
    * @return the id to where the layout is located
    */
    uint32 CreateDescriptorSetLayout(VulkanUtils::Descriptions::FDescriptorSetLayoutBinding* inLayoutBindings, uint32 inBindingCount, VulkanUtils::Descriptions::FDescriptorSetLayoutCreateInfo& inDescriptorSetLayoutCreateInfo)
    {
        VkDescriptorSetLayoutBinding* DescriptorSetLayoutBindings = new VkDescriptorSetLayoutBinding[inBindingCount];
        
        for (uint32 i = 0; i < inBindingCount; i++)
        {
            DescriptorSetLayoutBindings[i].pImmutableSamplers = nullptr;
            inLayoutBindings[i].WriteTo(DescriptorSetLayoutBindings[i]);
        }

        VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = VulkanUtils::Initializers::DescriptorSetLayoutCreateInfo();
        inDescriptorSetLayoutCreateInfo.WriteTo(DescriptorSetLayoutCreateInfo);
        DescriptorSetLayoutCreateInfo.bindingCount = inBindingCount;
        DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings;

        VkDescriptorSetLayout NewLayout;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*Device->GetDeviceHandle(), &DescriptorSetLayoutCreateInfo, nullptr, &NewLayout), "[VulkanDescriptorSetsLayout]: Failed to create a descriptor set layout!");

        DescriptorSetLayoutHandles.push_back(NewLayout);

        delete[] DescriptorSetLayoutBindings;

        return DescriptorSetLayoutHandles.size() - 1;
    }

public:
	/**
	* Returns a handle to a descriptor set layout
	*
	* @param the id to which the layout is located
	*
	* @return a descriptor set layout at the 'inLayoutId' specified
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
class VRIXIC_API VulkanDescriptorPool
{
private:
	VulkanDevice* Device;
	VkDescriptorPool DescriptorPoolHandle;

	uint32 MaxDescriptorSets;

	const VulkanDescriptorSetsLayout* DescriptorSetsLayout;

public:
	VulkanDescriptorPool(VulkanDevice* inDevice, const VulkanDescriptorSetsLayout& inSetsLayout, uint32 inMaxSets, std::vector<VkDescriptorPoolSize>& inPoolSizes)
		: Device(inDevice), MaxDescriptorSets(inMaxSets),
		DescriptorSetsLayout((const VulkanDescriptorSetsLayout*)&inSetsLayout),
		DescriptorPoolHandle(VK_NULL_HANDLE)
	{
		CreateDescriptorPool(inPoolSizes);
	}

	~VulkanDescriptorPool()
	{
		Device->WaitUntilIdle();
		vkDestroyDescriptorPool(*Device->GetDeviceHandle(), DescriptorPoolHandle, nullptr);
	}

	/**
	* Uses this pool to allocate a descriptor set
	*
	* @param 1: count of descriptor sets to create
	* @param 2: descriptor set to be filled
	* @param 3: the layout id used to create the decriptor set
	*
	* @return if allocated of set(s) was successfull
	*/
	bool AllocateDescriptorSets(uint32 inDescriptorSetCount, VkDescriptorSet* outDescriptorSet, uint32 inLayoutId) const
	{
		VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = VulkanUtils::Initializers::DescriptorSetAllocateInfo();
		DescriptorSetAllocateInfo.descriptorPool = DescriptorPoolHandle;
		DescriptorSetAllocateInfo.descriptorSetCount = inDescriptorSetCount;
		DescriptorSetAllocateInfo.pSetLayouts = DescriptorSetsLayout->GetLayoutHandle(inLayoutId);

#if _DEBUG
		VkResult Result = vkAllocateDescriptorSets(*Device->GetDeviceHandle(), &DescriptorSetAllocateInfo, outDescriptorSet);
		VK_CHECK_RESULT(Result, "[VulkanDescriptorPool]: Failed to allocate a descriptor set!");

		return Result == VK_SUCCESS;
#else
		return vkAllocateDescriptorSets(*Device->GetDeviceHandle(), &DescriptorSetAllocateInfo, outDescriptorSet) == VK_SUCCESS;
#endif		
	}

	/**
	* Bind/Link a descriptor set to a vulkan buffer
	* 
	* @param inBuffer The buffer the descriptor set will get bound to
	* @param inWriteDescriptorSet Information about where/what to bind
	*/
	void BindDescriptorSetToBuffer(const VulkanBuffer* inBuffer, const VulkanUtils::Descriptions::FWriteDescriptorSet& inWriteDescriptorSet)
	{
		VkDescriptorBufferInfo DescriptorBufferInfo = { 0 };
		DescriptorBufferInfo.buffer = *inBuffer->GetBufferHandle();
		DescriptorBufferInfo.offset = 0;
		DescriptorBufferInfo.range = inBuffer->GetBufferSize();

		VkWriteDescriptorSet WriteDescriptorSet = VulkanUtils::Initializers::WriteDescriptorSet();
		inWriteDescriptorSet.WriteTo(WriteDescriptorSet);
		WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;

		// Link 
		vkUpdateDescriptorSets(*Device->GetDeviceHandle(), 1, &WriteDescriptorSet, 0, nullptr);
	}

	/**
	* Bind/Link a descriptor set to a vulkan texture view - image 
	*
	* @param inDescriptorImageInfo - Information about the image binding 
	* @param inWriteDescriptorSet Information about where/what to bind
	*/
	void BindDescriptorSetToTexture(const VulkanUtils::Descriptions::FDescriptorImageInfo& inDescriptorImageInfo, const VulkanUtils::Descriptions::FWriteDescriptorSet& inWriteDescriptorSet)
	{
		VkDescriptorImageInfo DescriptorImageInfo = { 0 };
		inDescriptorImageInfo.WriteTo(DescriptorImageInfo);

		VkWriteDescriptorSet WriteDescriptorSet = VulkanUtils::Initializers::WriteDescriptorSet();
		inWriteDescriptorSet.WriteTo(WriteDescriptorSet);
		WriteDescriptorSet.pImageInfo = &DescriptorImageInfo;

		// Link 
		vkUpdateDescriptorSets(*Device->GetDeviceHandle(), 1, &WriteDescriptorSet, 0, nullptr);
	}

private:
	void CreateDescriptorPool(std::vector<VkDescriptorPoolSize>& inPoolSizes)
	{
		VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = VulkanUtils::Initializers::DescriptorPoolCreateInfo();
		DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
		DescriptorPoolCreateInfo.maxSets = MaxDescriptorSets;
		DescriptorPoolCreateInfo.poolSizeCount = inPoolSizes.size();
		DescriptorPoolCreateInfo.pPoolSizes = inPoolSizes.data();

		VK_CHECK_RESULT(vkCreateDescriptorPool(*Device->GetDeviceHandle(), &DescriptorPoolCreateInfo, nullptr, &DescriptorPoolHandle), "[VulkanDescriptorPool]: Failed to create a descriptor pool!");
	}

public:
	inline const VkDescriptorPool* GetDescriptorPoolHandle() const
	{
		return &DescriptorPoolHandle;
	}
};
