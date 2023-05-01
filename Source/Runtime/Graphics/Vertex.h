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
struct VRIXIC_API PBRVertex {
public:
    /** Position of the vertex: Tighly Packed With UVs, meaning The XYZ components is the positions, while W is the first UV */
    Vector4D Position;

    /** Tangent vector */
    Vector4D Tangent;

    /** Normal vector: Tighlty packed with UVs, meaning the XYZ components is the normal vector while W is the second UV*/
    Vector4D Normal;
};
