/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "Texture.h"

Texture::Texture(const FTextureConfig& inTextureConfig)
    : Type(inTextureConfig.Type), BindFlags(inTextureConfig.BindFlags), Extent(inTextureConfig.Extent) { }

void Texture::SetPath(const std::string& inTexturePath)
{
    Path = inTexturePath;
}

inline EResourceType Texture::GetResourceType() const
{
    return EResourceType::Texture;
}

inline ETextureType Texture::GetType() const
{
    return Type;
}

inline uint32 Texture::GetBindFlags() const
{
    return BindFlags;
}

inline const FExtent3D& Texture::GetExtent() const
{
    return Extent;
}

inline const std::string& Texture::GetPath() const
{
    return Path;
}
