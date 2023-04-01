/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Misc/Interface.h>
#include <Runtime/Core/Math/Vector4D.h>

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
struct VRIXIC_API RenderClearValues
{
public:
    typedef Vector4D LinearColor;

    LinearColor Color;
    float Depth;
    uint32 Stencil;
};

/**
* Helper struct that contains information of begininning a render pass
*/
struct VRIXIC_API RenderPassBeginInfo
{
public:
    // Render pass pointer 
    IRenderPass* RenderPassPtr;

    // Clear values array (Could also be one)
    RenderClearValues* ClearValues;

    // Number of clear values
    uint32 NumClearValues;
};
