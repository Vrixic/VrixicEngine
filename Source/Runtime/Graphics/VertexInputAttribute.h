/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "Format.h"
#include <Misc/Defines/GenericDefines.h>

/**
 * Defines a vertex input attribute used to define vertex attribute for vertex shaders
*/
struct VRIXIC_API FVertexInputAttribute
{
public:
    /**
    * The shader input location number
    *
    * @remakes glsl example: layout(location = 0) in vec4 vertexColor; // meaning location = 0
    */
    uint32          Location;

    /**
    * The binding number which this attribute takes its data from; 
    * its the binding slot, this correlates with the binding num from vertex input description
    */
    uint32          BindingNum;

    /** Vertex Attribute format, Ex: RGBA32Float which basically means a Vector4D */
    EPixelFormat    Format;

    /** The byte offset of this attribute relative from the start of an element in the vertex binding */
    uint32          Offset;

public:
    inline FVertexInputAttribute() = default;
    inline FVertexInputAttribute(const FVertexInputAttribute&) = default;
    inline FVertexInputAttribute& operator=(const FVertexInputAttribute&) = default;

    inline bool operator==(const FVertexInputAttribute& inRhs) const;

    inline bool operator!=(const FVertexInputAttribute& inRhs) const;
};

bool FVertexInputAttribute::operator==(const FVertexInputAttribute& inRhs) const
{
    return
        (
            Location == inRhs.Location &&
            BindingNum == inRhs.BindingNum &&
            Format == inRhs.Format &&
            Offset == inRhs.Offset
            );
}

bool FVertexInputAttribute::operator!=(const FVertexInputAttribute& inRhs) const
{
    return !(*this == inRhs);
}