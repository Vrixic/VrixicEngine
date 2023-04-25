/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "Extents.h"
#include <Misc/Defines/GenericDefines.h>
#include "Format.h"

#include <vector>

/**
* Render passes Load operations
*/
enum class EAttachmentLoadOp
{
    // Could cause unknown errors, should always specify
    Undefined,

    // Loads prev content 
    Load,

    // Clears the prev content 
    Clear
};

/**
* Render passes Store operations
*/
enum class EAttachmentStoreOp
{
    // Could cause unknown errors, should always specify
    Undefined,

    // Stores the output into its respective attachment 
    Store
};

/**
* Layout of a texture 
*/
enum class ETextureLayout
{
   Undefined                = 0,
   ColorAttachment          = 2,
   DepthStencilAttachment   = 3,
   DepthStencilReadOnly     = 4,
   PresentSrc               = 1000001002, // for swapchains 
};

/**
* All flags in use by the subpass dependencies
*/
struct FSubpassAssessFlags
{
public:
    enum
    {
#define BIT(x) (1 << x)

        ColorAttachmentRead          = BIT(0),
        ColorAttachmentWrite         = BIT(1)
    };
};

/**
* Defines description for an attachment, the format, load and store operations
*/
struct VRIXIC_API FAttachmentDescription
{
public:
    /** Attachment format */
    EPixelFormat Format;

    /** The load operation of the pervious attachment */
    EAttachmentLoadOp LoadOp;

    /** The store operation of the ouput for the attachment */
    EAttachmentStoreOp StoreOp;

    /** layout the attachment image subresource will be (For when the render pass begins) */
    ETextureLayout InitialLayout;

    /** layout the attachment image subresource will be transitioned to when render pass ends */
    ETextureLayout FinalLayout;

public:
    FAttachmentDescription()
        : Format(EPixelFormat::Undefined), LoadOp(EAttachmentLoadOp::Undefined), StoreOp(EAttachmentStoreOp::Undefined), InitialLayout(ETextureLayout::Undefined), FinalLayout(ETextureLayout::Undefined) { }
};

/**
* It is a subpass dependency which states the flow of the renderpass and how the src and dst mask are in use 
*/
struct VRIXIC_API FSubpassDependencyDescription
{
public:
    /** Source Access Mask */
    uint32 SrcAccessMaskFlags;

    /** Destination Access Mask */
    uint32 DstAccessMaskFlags;

public:
    FSubpassDependencyDescription()
        : SrcAccessMaskFlags(0), DstAccessMaskFlags(0) { }
};

/**
* Contain information for configuring a render pass creation 
*/
struct VRIXIC_API FRenderPassConfig
{
public:
    /** Color attachments for the render pass */
    std::vector<FAttachmentDescription> ColorAttachments;

    /** List of subpass dependencies */
    std::vector<FSubpassDependencyDescription> SubpassDependencies;

    /** Depth and stencil attachment used by the render pass */
    FAttachmentDescription DepthStencilAttachment;

    /** Number of samples for the attachment, CANNOT BE ZERO (Default = 1, which indicated multisampling is disabled) */
    uint32 NumSamples;

    /** The render area that will be in use for the render pass */
    FExtent2D RenderArea;

public:
    FRenderPassConfig()
        : NumSamples(1), RenderArea(1280u, 720u) { }

public:
    uint32 GetNumColorAttachments() const
    {
        return (uint32)ColorAttachments.size();
    }
};
