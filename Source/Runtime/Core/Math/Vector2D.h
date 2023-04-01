/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>

struct VRIXIC_API Vector2D
{
    float X;

    float Y;

public:
    inline Vector2D() : X(0.0f), Y(0.0f) { }

    inline Vector2D(float inX, float inY);

    inline Vector2D(float inValue);
};

inline Vector2D::Vector2D(float inX, float inY) : X(inX), Y(inY) { }

inline Vector2D::Vector2D(float inValue) : X(inValue), Y(inValue) { }
