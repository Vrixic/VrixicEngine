/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Matrix4D.h"

/* Represents a translation matrix */
struct VRIXIC_API TranslationMatrix4D : public Matrix4D
{
    inline TranslationMatrix4D(const Vector3D& translationVector);

    /* A Factory function to easily make a translation matrix */
    inline static TranslationMatrix4D Make(const Vector3D& translationVector);
};

inline TranslationMatrix4D::TranslationMatrix4D(const Vector3D& translationVector)
    : Matrix4D(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        translationVector.X, translationVector.Y, translationVector.Z, 1.0f) { }

inline TranslationMatrix4D TranslationMatrix4D::Make(const Vector3D& translationVector)
{
    return TranslationMatrix4D(translationVector);
}
