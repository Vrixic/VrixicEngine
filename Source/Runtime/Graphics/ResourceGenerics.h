/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>

/**
* Enumeration including all the resource types supported and can be used by a pipeline binding decriptor
*/
enum class EBDResourceType
{
    Undefined,      // Resource type is unknown
    Buffer,         // a buffer resource
    Texture,        // a texture resource
    Sampler,        // a sampler resource 
};


/**
* Binding flags for resources - describes the resources 
*/
struct ResourceBindFlags
{
#define BIT(x) (1 << x)

    enum
    {
        // buffer resource that is made for vertices
        VertexBuffer                = BIT(0),

        // buffer resource that is made for indices
        IndexBuffer                 = BIT(1),

        // buffer resource for binding to set of constant (vulkan - > push constants)
        ConstantBuffer              = BIT(2),

        // Allows for textures to be used as a render target for color attachment 
        ColorAttachment             = BIT(3),

        // allows tectures to be used as a render target for depth-stencil attachment 
        DepthStencilAttachment      = BIT(4),
    };
};
