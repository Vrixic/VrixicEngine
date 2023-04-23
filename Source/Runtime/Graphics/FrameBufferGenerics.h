/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Texture.h"

/**
* Enumeration that specifies the type of attachment it is (FrameBuffer attachments)
*/
enum class EAttachmentType
{
    Color,  // Used for Color Output

    DepthStencl, // Used for Depth and Stencil output 
};

/**
* Helper for a frame buffer attachment
*/
struct VRIXIC_API FrameBufferAttachment
{
public:
    // The ouput texture attachment for the frame buffer 
    Texture* Attachment;

public:
    FrameBufferAttachment()
        : Attachment(nullptr) { }
};

/**
* Contains information for a frame buffer creation 
*/
struct VRIXIC_API FrameBufferConfig
{
public:
    // The render pass this frame buffer is associated with 
    const IRenderPass* RenderPass;

    // The resolution of the frame buffer 
    Extent2D Resolution;

    // all of the attachments for the frame buffer
    std::vector<FrameBufferAttachment> Attachments;

public:
    FrameBufferConfig()
        : RenderPass(nullptr), Resolution() { }
};
