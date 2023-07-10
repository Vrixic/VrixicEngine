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
        /** Buffer Flags */

        // buffer resource that is made for vertices
        VertexBuffer                = BIT(0),

        // buffer resource that is made for indices
        IndexBuffer                 = BIT(1),

        // buffer resource for binding to set of constant -> read only 
        ConstantBuffer              = BIT(2),

        // storage buffer resource used to bind to a buffer that holds huge amounts of data 
        StorageBuffer               = BIT(3),

        // buffer resource for binding to set of constant -> read only 
        UniformBuffer               = BIT(4),

        TexelBuffer                 = BIT(5),

        // dynamics buffers 
        Dynamic                     = BIT(6),

        StagingBuffer               = BIT(7),

        /** Texture Flags */

        // Allows for textures to be used as a render target for color attachment 
        ColorAttachment             = BIT(8),

        // allows tectures to be used as a render target for depth-stencil attachment 
        DepthStencilAttachment      = BIT(9),

        // The texture will get sampled 
        Sampled                     = BIT(10),

        /** ADDITIVE FLAGS */
        SrcTransfer                 = BIT(15), 
        DstTransfer                 = BIT(16),
    };
};

/**
* Creation flags for resources 
*/
struct VRIXIC_API FResourceCreationFlags
{
public:
#define BIT(x) (1 << x)
    enum {

        /** Texture */

        // The texture allows for imageview to have a different format 
        Mutable = BIT(8),

        /** Allows for cube arrays or cube to be created fro imageview */
        Cube = BIT(9),

        /** States that the resource was created using KTX software */
        KTX = BIT(10),
    };
};

