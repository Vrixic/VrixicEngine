/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VulkanDevice.h"
#include <Runtime/Graphics/Sampler.h>
#include <Runtime/Graphics/SamplerGenerics.h>

/**
* Vulkan specific sampler: Samplers allow the shaders to read textures and also filter them based on the settings we provide: Nearest, Bilinear, Anisotrophy....etc...
*/
class VRIXIC_API VulkanSampler final : public Sampler
{
public:
    /**
    * Empty Sampler (NULL)
    */
    VulkanSampler(VulkanDevice* inDevice);

    ~VulkanSampler();

    /**
    * Creates the sampler
    *
    * @param inSamplerConfig the configuration used to create the sampler
    */
    void Create(const FSamplerConfig& inSamplerConfig);

public:
    inline VkSampler GetSamplerHandle() const { return SamplerHandle; }

private:
    VulkanDevice* Device;

    VkSampler SamplerHandle;
};