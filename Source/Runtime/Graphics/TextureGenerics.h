/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "Extents.h"
#include "Format.h"
#include <Misc/Defines/GenericDefines.h>
#include "RenderPassGenerics.h"

class Buffer;

/**
* Texture type : in vulkan -> ImageViewType
*/
enum class ETextureType
{
    Texture1D = 0,
    Texture2D = 1,
    Texture3D = 2,
    TextureCube = 3,
    Texture1DArray = 4,
    Texture2DArray = 5,
    TextureCubeArray = 6,
};

/**
* Specifies the component values places in each component of the output vector
*/
enum class ETextureSwizzle
{
    Identity = 0, // set to identity swizzle
    Zero = 1, // set to zero
    One = 2, // replaced to a constant one
    R = 3, // replaced by Red component
    G = 4, // replaced by Green component
    B = 5, // replaced by Blue component
    A = 6, // replaced by Alpha component
};

/**
* Helper struct that specifies the mip map level ranges and array layers of the texture
*/
struct VRIXIC_API FTextureSubresourceRange
{
public:
    /** Mipmap level for the sub texture. 0 indicates the base texture and n > 0 are the n-th mip level texture */
    uint32 BaseMipLevel;

    /** Number of mipmap levels */
    uint32 NumMipLevels;

    /** specifies a texture array layer: ETextureType::Texture1DArray */
    uint32 BaseArrayLayer;

    /** the number of array layers */
    uint32 NumArrayLayers;

public:
    FTextureSubresourceRange()
        : BaseMipLevel(0), NumMipLevels(1), BaseArrayLayer(0), NumArrayLayers(1) { }
};

/**
* Configuration used to configure the creation of textures
*/
struct VRIXIC_API FTextureConfig
{
public:
    /** Indicates the texture type, is it a 1D or 2D, etc.. texture */
    ETextureType Type;

    /** Texture layout indiciates what type if texture it is and what can be done with it (read/write) */
    //ETextureLayout Layout;

    /** These flags specify which resource slot and attachment this texture will be bound to, ResourceBindFlags::ColorAttachment */
    uint32 BindFlags;

    /** The format of the texture  */
    EPixelFormat Format;

    /** Size of the texture */
    FExtent3D Extent;

    /** Number of mipmap levels */
    uint32 MipLevels;

    /** Number of array layers */
    uint32 NumArrayLayers;

    /** Number of samplers to take per texel */
    uint32 NumSamples;

public:
    FTextureConfig()
        : Type(ETextureType::Texture2D), /*Layout(ETextureLayout::Undefined),*/ BindFlags(0), Format(EPixelFormat::Undefined), Extent(0u, 0u, 0u), MipLevels(1), NumArrayLayers(1), NumSamples(1) { }
};

/**
* Configuration used to configure the creation of texture views
*/
struct VRIXIC_API FTextureViewConfig
{
public:
    /** Indicates the texture type, is it a 1D or 2D, etc.. texture */
    ETextureType Type;

    /** The format of the texture */
    EPixelFormat Format;

    /** texture subresource range which just specifies mipmap levels and array layers */
    FTextureSubresourceRange Subresource;

public:
    FTextureViewConfig()
        : Type(ETextureType::Texture2D), Format(EPixelFormat::Undefined) { }
};

/**
* Contains information for copying buffer to a texture
*/
struct VRIXIC_API FTextureWriteInfo
{
public:
    /** The handle to the buffer to copy data from */
    Buffer* BufferHandle;

    /** The extent of the texture, for 1d and 2d the depth has to be 1*/
    FExtent3D Extent;
    
    /** Specifies mipmap levels, and array layer ranges...etc...*/
    FTextureSubresourceRange Subresource;

public:
    FTextureWriteInfo() : BufferHandle(nullptr), Extent(0u, 0u, 1u) { }
};

