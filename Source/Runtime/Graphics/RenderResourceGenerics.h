/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>

/**
* Enumeration including all the resource types supported and can be used by a pipeline binding decriptor
*/
enum class EResourceType
{
    Undefined,                  // Resource type is unknown
    Buffer,                     // a buffer resource
    Texture,                    // a texture resource
    Sampler,                    // a sampler resource 
    CombinedTextureSampler,     // combination of a texture and a sampler
    StorageTexture,              // 
    InputAttachment             //
};

/**
* Binding flags for resources - describes the resources 
*/
struct VRIXIC_API FResourceBindFlags
{
public:
#define BIT(x) (1 << x)

    enum
    {
        // buffer resource that is made for vertices
        VertexBuffer                = BIT(0),

        // buffer resource that is made for indices
        IndexBuffer                 = BIT(1),

        // buffer resource for binding to set of constant 
        ConstantBuffer              = BIT(2),

        // storage buffer resource used to bind to a buffer
        StorageBuffer               = BIT(3),

        TexelBuffer                 = BIT(4),

        // Allows for textures to be used as a render target for color attachment 
        ColorAttachment             = BIT(5),

        // allows tectures to be used as a render target for depth-stencil attachment 
        DepthStencilAttachment      = BIT(6),

        // dynamics buffers 
        Dynamic                     = BIT(7)
    };
};

