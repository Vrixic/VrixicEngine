/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "RenderResource.h"
#include "TextureGenerics.h"

/**
* A texture resource : vulkan -> VkImage 
*/
class VRIXIC_API Texture : RenderResource
{
private:
    /** The texture type */
    ETextureType Type;

    /** The bind flags that were used to create this texture */
    uint32 BindFlags;

protected:
    Texture(const ETextureType inTextureType, uint32 inBindFlags);

public:
    /**
    * @returns EResourceType the type of resource this object is which is texture
    * @remarks this cannot be inherited anymore as there is no need
    */
    virtual inline EResourceType GetResourceType() const override final;

    /**
    * @returns ETextureType the type of texture
    */
    inline ETextureType GetType() const;

    /**
    * @returns uitn32 the bind flags that were used to create this texture with 
    */
    inline uint32 GetBindFlags() const;
};
