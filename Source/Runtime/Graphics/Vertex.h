/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once

#include <Core/Core.h>
#include <Runtime/Core/Math/Vector4D.h>

/**
* Tighly packed PBRVertex Structure 
*/
struct VRIXIC_API FPBRVertex {
public:
    /** Position of the vertex */
    Vector3D Position;

    /** Tangent vector */
    Vector4D Tangent;

    /** Normal vector */
    Vector3D Normal;

    float TexCoord[2];

    float Padding[4];
};
