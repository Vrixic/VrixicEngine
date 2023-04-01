/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Matrix4D.h"

/* Makes a look at matrix that is already inversed */
struct VRIXIC_API LookAtMatrix4D : public Matrix4D
{
    /* By Default Make a Left Handed LootAtMatrix -> View matrix is returned */
    inline LookAtMatrix4D(const Vector3D& eye, const Vector3D& target, const Vector3D& up = Vector3D(0, 1, 0));

    /* Factory function for making a left handed look at matrix */
    inline static LookAtMatrix4D MakeLookAtLH(const Vector3D& eye, const Vector3D& target, const Vector3D& up = Vector3D(0, 1, 0));
};

inline LookAtMatrix4D::LookAtMatrix4D(const Vector3D& eye, const Vector3D& target, const Vector3D& up)
{
    Vector3D ZAxis = (target - eye).Normalize();					// normal(target - eye)
    Vector3D XAxis = Vector3D::CrossProduct(up, ZAxis).Normalize(); // normal(cross(up, ZAxis))
    Vector3D YAxis = Vector3D::CrossProduct(ZAxis, XAxis);;			// cross(ZAxis, XAxis)

    // [ XAxis.X			YAxis.X				ZAxis.X			0]
    // [ XAxis.Y			YAxis.Y				ZAxis.Y			0]
    // [ XAxis.Z			YAxis.Z				ZAxis.Z			0]
    // [-dot(XAxis, eye)   -dot(YAxis, eye)	   -dot(ZAxis, eye)	1]

    M[0][0] = XAxis.X;
    M[1][0] = XAxis.Y;
    M[2][0] = XAxis.Z;
    M[3][0] = -Vector3D::DotProduct(XAxis, eye);

    M[0][1] = YAxis.X;
    M[1][1] = YAxis.Y;
    M[2][1] = YAxis.Z;
    M[3][1] = -Vector3D::DotProduct(YAxis, eye);

    M[0][2] = ZAxis.X;
    M[1][2] = ZAxis.Y;
    M[2][2] = ZAxis.Z;
    M[3][2] = -Vector3D::DotProduct(ZAxis, eye);

    M[0][3] = 0.0f;
    M[1][3] = 0.0f;
    M[2][3] = 0.0f;
    M[3][3] = 1.0f;

    //Matrix4D Result(
    //	XAxis.X, YAxis.X, ZAxis.X, 0.0f,
    //	XAxis.Y, YAxis.Y, ZAxis.Y, 0.0f,
    //	XAxis.Z, YAxis.Z, ZAxis.Z, 0.0f,
    //	-Vector3D::DotProduct(XAxis, eye),
    //	-Vector3D::DotProduct(YAxis, eye),
    //	-Vector3D::DotProduct(ZAxis, eye),
    //	1.0f
    //);
}

inline LookAtMatrix4D LookAtMatrix4D::MakeLookAtLH(const Vector3D& eye, const Vector3D& target, const Vector3D& up)
{
    return LookAtMatrix4D(eye, target, up);
}
