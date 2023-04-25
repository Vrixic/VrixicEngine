/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Extents.h"
#include "FrameBufferGenerics.h"
#include "RenderPass.h"

/**
* An interface for a generic frame buffer 
*/
class VRIXIC_API IFrameBuffer : public Interface
{
public:
    /**
    * @returns Extend2D the extent of the framebuffer in screen space 
    */
    virtual FExtent2D GetResolution() const = 0;

    /**
    * @return uint32 the number of attachments that are attached to this frame buffer
    */
    virtual uint32 GetNumAttachments() const = 0;

    /**
    * @returns IRenderPass* a pointer to the render pass that was used to create the frame buffer (render pass that is associated with this frame buffer)
    */
    virtual IRenderPass* GetRenderPassHandle() const = 0;
};
