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
struct VRIXIC_API FFrameBufferAttachment
{
public:
    /** The ouput texture attachment for the frame buffer */
    Texture* Attachment;

public:
    FFrameBufferAttachment()
        : Attachment(nullptr) { }
};

/**
* Contains information for a frame buffer creation 
*/
struct VRIXIC_API FFrameBufferConfig
{
public:
    /** The render pass this frame buffer is associated with */
    const IRenderPass* RenderPass;

    /** The resolution of the frame buffer */
    FExtent2D Resolution;

    /** all of the attachments for the frame buffer */
    std::vector<FFrameBufferAttachment> Attachments;

public:
    FFrameBufferConfig()
        : RenderPass(nullptr), Resolution() { }
};
