/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Vector3D.h"

class VRIXIC_API Ray
{
    /** Origin point of the Ray*/
    Vector3D Origin;

    /** Direction vector of the Ray */
    Vector3D Direction;

public:
    /**
    * Creates a defaut ray with 0 as origin, and forward as direction
    */
    inline Ray();

    /**
    * Creates a Ray with origin and direction
    *
    * @param inOrigin - Origin point of Ray
    * @param inDirection - Direction vector of Ray
    * @param inIsNormalized - is 'inDirection' normalized, by default it is false
    */
    inline Ray(const Vector3D& inOrigin, const Vector3D& inDirection, bool inIsNormalized = false);

public:
    /**
    * Calculates a point on ray at position specified by paramater
    *
    * @param inScalar - Distance from origin along the Ray
    * @return Vector3D a point on Ray
    */
    Vector3D PointAtPosition(float inScalar) const;
};

Ray::Ray()
{
    Origin = Vector3D::ZeroVector();
    Direction = Vector3D(0, 0, 1);
}

Ray::Ray(const Vector3D& inOrigin, const Vector3D& inDirection, bool inIsNormalized)
{
    Origin = inOrigin;
    Direction = inDirection;

    if (!inIsNormalized)
    {
        Direction.Normalize();
    }
}

Vector3D Ray::PointAtPosition(float inScalar) const
{
    return Origin + (Direction * inScalar);
}
