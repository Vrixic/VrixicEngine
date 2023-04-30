/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanDescriptorSet.h"
#include "VulkanSampler.h"
#include "VulkanTextureView.h"

void VulkanDescriptorSets::LinkToBuffer(uint32 inIndex, const FDescriptorSetsLinkInfo& inDescriptorSetsLinkInfo)
{
    VulkanBuffer* BufferHandle = (VulkanBuffer*)inDescriptorSetsLinkInfo.ResourceHandle.BufferHandle;

    VE_ASSERT(BufferHandle != nullptr, VE_TEXT("[VulkanDescriptorSets]: Cannot update a descriptor set if the buffer is invalid!"));

    VkDescriptorBufferInfo DescriptorBufferInfo = { };
    DescriptorBufferInfo.buffer = *BufferHandle->GetBufferHandle();
    DescriptorBufferInfo.offset = 0;
    DescriptorBufferInfo.range = BufferHandle->GetBufferSize();

    VkWriteDescriptorSet WriteDescriptorSet = VulkanUtils::Initializers::WriteDescriptorSet();
    WriteDescriptorSet.dstBinding = inDescriptorSetsLinkInfo.BindingStart;
    WriteDescriptorSet.dstSet = DescriptorSetHandles[inIndex];
    WriteDescriptorSet.dstArrayElement = inDescriptorSetsLinkInfo.ArrayElementStart;
    WriteDescriptorSet.descriptorCount = inDescriptorSetsLinkInfo.DescriptorCount;

    WriteDescriptorSet.descriptorType = VulkanTypeConverter::ConvertBindFlagsToVkDescriptorType(EResourceType::Buffer, BufferHandle->GetUsageFlags());

    WriteDescriptorSet.pImageInfo = nullptr;
    WriteDescriptorSet.pBufferInfo = &DescriptorBufferInfo;
    WriteDescriptorSet.pTexelBufferView = nullptr;

    // Link 
    vkUpdateDescriptorSets(*Device->GetDeviceHandle(), 1, &WriteDescriptorSet, 0, nullptr);
}

void VulkanDescriptorSets::LinkToTexture(uint32 inIndex, const FDescriptorSetsLinkInfo& inDescriptorSetsLinkInfo)
{
    VulkanTextureView* TextureHandle = (VulkanTextureView*)inDescriptorSetsLinkInfo.ResourceHandle.TextureHandle;
    VE_ASSERT(TextureHandle != nullptr, VE_TEXT("[VulkanDescriptorSets]: Cannot update a descriptor set if the texture is invalid!"));

    VulkanSampler* SamplerVk = (VulkanSampler*)inDescriptorSetsLinkInfo.TextureSampler;
    VE_ASSERT(SamplerVk != nullptr, VE_TEXT("[VulkanDescriptorSets]: Cannot update a descriptor set if the sampler for the texture is invalid..!"));

    VkDescriptorImageInfo  DescriptorImageInfo = { };
    DescriptorImageInfo.sampler = SamplerVk->GetSamplerHandle();
    DescriptorImageInfo.imageView = *TextureHandle->GetImageViewHandle();
    DescriptorImageInfo.imageLayout = TextureHandle->GetImageLayout();

    VkWriteDescriptorSet WriteDescriptorSet = VulkanUtils::Initializers::WriteDescriptorSet();
    WriteDescriptorSet.dstBinding = inDescriptorSetsLinkInfo.BindingStart;
    WriteDescriptorSet.dstSet = DescriptorSetHandles[inIndex];
    WriteDescriptorSet.dstArrayElement = inDescriptorSetsLinkInfo.ArrayElementStart;
    WriteDescriptorSet.descriptorCount = inDescriptorSetsLinkInfo.DescriptorCount;

    WriteDescriptorSet.descriptorType = VulkanTypeConverter::ConvertBindFlagsToVkDescriptorType(EResourceType::Texture, TextureHandle->GetBindFlags());

    WriteDescriptorSet.pImageInfo = &DescriptorImageInfo;
    WriteDescriptorSet.pBufferInfo = nullptr;
    WriteDescriptorSet.pTexelBufferView = nullptr;

    // Link 
    vkUpdateDescriptorSets(*Device->GetDeviceHandle(), 1, &WriteDescriptorSet, 0, nullptr);
}
