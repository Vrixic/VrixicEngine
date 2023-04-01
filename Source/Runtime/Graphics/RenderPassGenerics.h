/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
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
* Defines description for an attachment, the format, load and store operations
*/
struct VRIXIC_API AttachmentDescription
{
public:
    // Attachment format 
    EPixelFormat Format;

    // The load operation of the pervious attachment 
    EAttachmentLoadOp LoadOp;

    // The store operation of the ouput for the attachment 
    EAttachmentStoreOp StoreOp;

};

struct VRIXIC_API RenderPassConfig
{
public:
    // Color attachments for the render pass
    std::vector<AttachmentDescription> ColorAttachments;

    // Depth and stencil attachment used by the render pass
    AttachmentDescription DepthStencilAttachment;

    // Number of samples for the attachment, CANNOT BE ZERO (Default = 1, which indicated multisampling is disabled)
    uint32 NumSamples;

public:
    uint32 GetNumColorAttachments() const
    {
        return ColorAttachments.size();
    }
};
