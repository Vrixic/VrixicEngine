/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "VulkanSampler.h"
#include <Misc/Defines/StringDefines.h>
#include "VulkanTypeConverter.h"

VulkanSampler::VulkanSampler(VulkanDevice* inDevice) : Device(inDevice), SamplerHandle(VK_NULL_HANDLE) { }

VulkanSampler::~VulkanSampler() 
{ 
    Device->WaitUntilIdle();
    
    if (SamplerHandle != VK_NULL_HANDLE)
    {
        vkDestroySampler(*Device->GetDeviceHandle(), SamplerHandle, nullptr);
    }
}

void VulkanSampler::Create(const FSamplerConfig& inSamplerConfig)
{
    VE_ASSERT(SamplerHandle == VK_NULL_HANDLE, VE_TEXT("[VulkanSampler]: Cannot create another sampler when a valid one already exists...!"));

    VkSamplerCreateInfo SamplerCreateInfo = VulkanUtils::Initializers::SamplerCreateInfo();

    SamplerCreateInfo.anisotropyEnable = inSamplerConfig.MaxAnisotropy == 0 ? VK_FALSE : VK_TRUE;
    SamplerCreateInfo.maxAnisotropy = inSamplerConfig.MaxAnisotropy;

    SamplerCreateInfo.compareEnable = inSamplerConfig.bEnableCompare;
    SamplerCreateInfo.compareOp = VulkanTypeConverter::ConvertCompareOpToVk(inSamplerConfig.CompareOp);

    SamplerCreateInfo.minLod = inSamplerConfig.MinLod;
    SamplerCreateInfo.maxLod = inSamplerConfig.MaxLod;

    SamplerCreateInfo.mipLodBias = inSamplerConfig.MipMapLodBias;

    SamplerCreateInfo.addressModeU = VulkanTypeConverter::ConvertSamplerAddressModeToVk(inSamplerConfig.AddressModeU);
    SamplerCreateInfo.addressModeV = VulkanTypeConverter::ConvertSamplerAddressModeToVk(inSamplerConfig.AddressModeV);
    SamplerCreateInfo.addressModeW = VulkanTypeConverter::ConvertSamplerAddressModeToVk(inSamplerConfig.AddressModeW);

    SamplerCreateInfo.minFilter = VulkanTypeConverter::ConvertSamplerFilterToVk(inSamplerConfig.MinFilter);
    SamplerCreateInfo.magFilter = VulkanTypeConverter::ConvertSamplerFilterToVk(inSamplerConfig.MagFilter);

    SamplerCreateInfo.borderColor = VulkanTypeConverter::ConvertBorderColorToVk(inSamplerConfig.BorderColor);

    SamplerCreateInfo.mipmapMode = VulkanTypeConverter::ConvertMipMapModeToVk(inSamplerConfig.MipMapMode);

    VK_CHECK_RESULT(vkCreateSampler(*Device->GetDeviceHandle(), &SamplerCreateInfo, nullptr, &SamplerHandle), VE_TEXT("[VulkanSampler]: Failed to create a sampler..."));
}
