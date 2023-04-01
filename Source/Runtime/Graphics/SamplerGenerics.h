/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>
#include "PipelineGenerics.h"

/**
* The address mode defines what to do for texture coordinates outside the [0, 1] range 
*/
enum class ESamplerAddressMode
{
    // repeats texture coords with interal [0, 1], specifies that repeat wrap mode will be used
    Repeat,

    // specifies that the mirrored repeat wrap mode will be used
    MirrorRepeat,

    // Clamps to edge
    ClampToEdge,

   // Clamps to border 
    ClampToBorder
};

/**
* The sampling filter to use 
*/
enum class ESamplerFilter
{
    // takes the closest/nearest texel as a sample
    Nearest,

    // interpolates between multiple texels from samples 
    Linear
};

/**
* The mip map modes 
*/
enum class EMipMapMode
{
    // takes the closest/nearest texel as a sample
    Nearest,

    // interpolates between multiple texels from samples 
    Linear
};

/**
* Enumeration specifying teh border color used for texture lookups 
*/
enum class EBorderColor
{
   FloatTransparentBlack        = 0,
   IntTransparentBlack          = 1,
   FloatOpaqueBlack             = 2,
   IntOpaqueBlack               = 3,
   FloatOpaqueWhite             = 4,
   IntOpaqueWhite               = 5,
};

struct VRIXIC_API SamplerConfig
{
public:
    // The mag filter to use
    ESamplerFilter MagFilter;

    // The min filter to used
    ESamplerFilter MinFilter;

    // the filter used for mipmaps 
    EMipMapMode MipMapMode;

    // the mode in the U direction or the x axis 
    ESamplerAddressMode AddressModeU;

    // the mode in the V direction or the y axis
    ESamplerAddressMode AddressModeV;

    // the mode in the W direction or the z axis 
    ESamplerAddressMode AddressModeW;

    // the LOD bias used for mip-mapping 
    float MipMapLodBias;
    
    // specifies if the sampler should use mip-maps or not
    bool bEnableMinMapping;

    // the max anisotrophy used from the range [1, 16], 0 means disabled 
    uint32 MaxAnisotropy;

    // specifies if it should use a compare operation for depth texture or not
    bool bEnableCompare;

    // the compare operation for depth textures
    ECompareOp CompareOp;

    // minimum level of detail
    float MinLod;

    // maximum level of detail
    float MaxLod;

    // the border color used for texture lookups
    EBorderColor BorderColor;
};