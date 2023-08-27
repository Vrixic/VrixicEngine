/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "Texture.h"

TextureResource::TextureResource(const FTextureConfig& inTextureConfig)
    : Type(inTextureConfig.Type), BindFlags(inTextureConfig.BindFlags), Extent(inTextureConfig.Extent) { }

void TextureResource::SetPath(const std::string& inTexturePath)
{
    Path = inTexturePath;
}

inline EResourceType TextureResource::GetResourceType() const
{
    return EResourceType::Texture;
}

inline ETextureType TextureResource::GetType() const
{
    return Type;
}

inline uint32 TextureResource::GetBindFlags() const
{
    return BindFlags;
}

inline const FExtent3D& TextureResource::GetExtent() const
{
    return Extent;
}

inline const std::string& TextureResource::GetPath() const
{
    return Path;
}
