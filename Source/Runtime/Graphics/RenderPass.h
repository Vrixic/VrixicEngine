/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include <Runtime/Core/Math/Vector4D.h>

class IFrameBuffer;

/**
* Base class for api to define a render pass object, a shell, as of now empty 
* Renderpasses tell API(also GPUs) many things about how to render a frame and what attachments to use and also keeps tracks of relation ships (Subpasses), 
*   meaning if an attachment is dependent in another that the render pass will know
*/
class VRIXIC_API IRenderPass : public Interface
{

};

/**
* Clear values used for rendering
*/
struct VRIXIC_API FRenderClearValues
{
public:
    typedef Vector4D LinearColor;

    LinearColor Color;
    float Depth;
    uint32 Stencil;

public:
    FRenderClearValues()
        : Color(0.0f, 0.0f, 0.0f, 1.0f), Depth(0.0f), Stencil(0u) { }
};

/**
* Helper struct that contains information of begininning a render pass
*/
struct VRIXIC_API FRenderPassBeginInfo
{
public:
    /** Render pass pointer */
    IRenderPass* RenderPassPtr;

    // 
    /** The frame buffer to attach */
    IFrameBuffer* FrameBuffer;

    /** Clear values array (Could also be one) */
    FRenderClearValues* ClearValues;

    /** Number of clear values */
    uint32 NumClearValues;

public:
    FRenderPassBeginInfo()
        : RenderPassPtr(nullptr), FrameBuffer(nullptr), ClearValues(nullptr), NumClearValues(0) { }
};
